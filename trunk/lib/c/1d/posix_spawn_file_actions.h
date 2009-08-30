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
 * FKEY
 * 
 * see set_factp()/get_factp for description of use
*/
#define FKEY(_km_)	(0x07426fc0 | (_km_))

/*
 * GET_FACTP
 * SET_FACTP
 * 
 * These macros wrap the get_factp()/set_factp() routines. For set we always use
 * KEY_MASK to enforce 8 byte alignment within this library however the get
 * operation will inspect the lower MIN_KEY_MASK bits of the incoming
 * 'posix_spawn_file_actions_t' object pointer and if non zero, will extract
 * those bits for use as the key mask established during a set operation for
 * the object. This allows procmgr to use an alternate alignment constraint when
 * processing the _PROC_POSIX_SPAWN message. All user programs will however be
 * forced to the alignment established by KEY_MASK. 
*/
#define SET_FACTP(a, b)		set_factp((a), (b), KEY_MASK)
#define GET_FACTP(a)		get_factp(*(posix_spawn_file_actions_t *)((unsigned int)(a) & ~MIN_KEY_MASK), \
									  ((unsigned int)(a) & MIN_KEY_MASK) ? (unsigned int)(a) & MIN_KEY_MASK : KEY_MASK)

/*
 * _posix_spawn_file_actions_t (internal version of 'posix_spawn_file_actions_t')
*/
typedef struct
{
	uint32_t  num_entries;
	posix_spawn_file_actions_list_t	action[1];	// variable sized data structure
} _posix_spawn_file_actions_t;
/*
 * FILE_ACTIONS_T_SIZE
 *
 * convenience macro to calculate the size (in bytes) of the resulting
 * '_posix_spawn_file_actions_t' structure given 'n' where 'n' (typically)
 * represents the value of the '_posix_spawn_file_actions_t.num_entries' field.
 * 
 * Note:
 * 		'n' must be >= 0
 * 		implementation is coded to ensure the proper result even for 'n' == 0
*/
#define FILE_ACTIONS_T_SIZE(n) \
		((sizeof(_posix_spawn_file_actions_t) - sizeof(posix_spawn_file_actions_list_t)) + \
		((n) * sizeof(posix_spawn_file_actions_list_t)))


/*
 * internal prototypes
*/
extern _posix_spawn_file_actions_t *get_factp(unsigned int x, unsigned int key_mask);
extern unsigned valid_factp(const posix_spawn_file_actions_t *_Restrict factp);


#endif	/* _POSIX_SPAWN */

