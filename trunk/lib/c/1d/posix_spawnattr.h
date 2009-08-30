/*
 * $QNXLicenseC:  
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
*/

#include <unistd.h>		// determines whether _POSIX_SPAWN is defined or not
#ifdef _POSIX_SPAWN

#include <spawn.h>
#include <sys/posix_spawn.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/procmsg.h>
#include <sys/netmgr.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/part.h>
#include <pthread.h>
#include <gulliver.h>

#include <assert.h>



#ifndef ROUNDUP
#define ROUNDUP(x, a)		(((x) + ((a)-1)) & ~((a)-1))
#endif


/*
 * AKEY
 * 
 * see set_attrp()/get_attrp for description of use
*/
#define AKEY(_km_)	(0x031759f0 | (_km_))

/*
 * GET_ATTRP
 * SET_ATTRP
 * 
 * These macros wrap the get_attrp()/set_attrp() routines. For set we always use
 * KEY_MASK to enforce 8 byte alignment within this library however the get
 * operation will inspect the lower MIN_KEY_MASK bits of the incoming
 * 'posix_spawnattr_t' object pointer and if non zero, will extract those bits
 * for use as the key mask established during a set operation for the object.
 * This allows procmgr to use an alternate alignment constraint when processing
 * the _PROC_POSIX_SPAWN message. All user programs will however be forced to
 * the alignment established by KEY_MASK. 
*/
#define SET_ATTRP(a, b)		set_attrp((a), (b), KEY_MASK)
#define GET_ATTRP(a)		get_attrp(*(posix_spawnattr_t *)((unsigned int)(a) & ~MIN_KEY_MASK), \
									  ((unsigned int)(a) & MIN_KEY_MASK) ? (unsigned int)(a) & MIN_KEY_MASK : KEY_MASK)

/*
 * internal posix_spawnattr related types
*/

/*
 * _posix_spawnattr_partition_t
 * 
 * This structure is used internally to record the resource partitions a
 * posix_spawnattr_t attributes object has. These partition id's represent the
 * partitions that a spawned process which uses this object will be associated
 * with
*/
typedef struct
{
	uint32_t	 size;				// size of the _posix_spawnattr_partition_t
	part_list_t	 part;				// list of partitions (sched and memory) to associate with
} _posix_spawnattr_partition_t;

/*
 * _posix_spawnattr_t
 * 
 * Internal representation of the opaque posix_spawnattr_t.
 * A posix_spawnattr_t is simply a 'void *'. The static initializer
 * POSIX_SPAWNATTR_INIT as well as the posix_spawnattr_init() function will
 * set the contents of the user allocated posix_spawnattr_t object to NULL.
 * An attributes object initialized this way will have the same effect as
 * NULL for the posix_spawnattr_t parameter to posix_spawn() ... that is to
 * use default attributes.
 * 
 * Setting non default values for the posix_spawnattr_t object will cause
 * memory to be allocated on the very first _setxxx() call. The address of this
 * allocated memory will be stuffed into the caller provided posix_spawnattr_t
 * object for use in other set calls. This has the effect of making the
 * 'posix_spawnattr_t' object completely opaque to user application as well as
 * requiring only 4 bytes for the 'posix_spawnattr_t' object if default values
 * will be used. 
*/
typedef struct
{
	/* the following fields are POSIX mandated */
	uint32_t	flags;		// lower 16 bits are POSIX. Upper are QNX
	pid_t		pgrp;		// POSIX
	uint32_t	node;		// QNX (why isn't there a node descriptor type ?)

	struct {
		uint32_t	max;	// QNX
	} stack;

	struct {
		sigset_t	dflt;	// POSIX
		sigset_t	mask;	// POSIX
		sigset_t	ignore;	// QNX
	} sig;

	struct {
		struct sched_param	param;	// POSIX
		int32_t  policy;			// POSIX
		uint32_t runmask;			// QNX
	} sched;

	_posix_spawnattr_partition_t *partition;	// QNX
	
} _posix_spawnattr_t;


/*
 * internal prototypes
*/
extern pthread_once_t once;
extern posix_spawnattr_t default_posix_spawnattr_t;
extern const _posix_spawnattr_t _default_posix_spawnattr_t;

extern void posix_spawnattr_t_once_init(void);
extern _posix_spawnattr_t *_posix_spawnattr_t_realloc(_posix_spawnattr_t *attrp, size_t size);
extern _posix_spawnattr_t *get_attrp(unsigned int x, unsigned int key_mask);
extern unsigned valid_attrp(const posix_spawnattr_t *_Restrict attrp);


#endif	/* _POSIX_SPAWN */

