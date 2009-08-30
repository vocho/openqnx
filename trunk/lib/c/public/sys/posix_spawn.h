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

/*
 * This header file is necessarily public as it defines an interface between
 * libc and the kernel for the purpose of the kernel utilizing some POSIX
 * related functions. It is however not intended to be documented as a user
 * interface
*/
#ifndef _POSIX_SPAWN_H_INCLUDED
#define _POSIX_SPAWN_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

/*
 * KEY_MASK
 * 
 * see set_attrp()/get_attrp() and set_factp()/get_factp for description of use
*/
#define KEY_MASK		0x7					// bits 0, 1 and 2 are always set
#define MIN_KEY_MASK	0x3					// bits 0 and 1 are always set


/*******************************************************************************
 * posix_spawn_file_actions related types for use with the QNX extension
 * posix_spawn_file_getactions() interface
*/
typedef enum
{
	posix_file_action_type_CLOSE	= 1,
	posix_file_action_type_DUP		= 2,
	posix_file_action_type_OPEN		= 3,
} posix_file_action_type;

typedef struct
{
	int32_t  fd;
} _posix_spawn_file_actions_close_t;

typedef struct
{
	int32_t  fd;
	int32_t  new_fd;
} _posix_spawn_file_actions_dup_t;

typedef struct
{
	uint32_t	size;		// size in bytes of this structure including path
	uint32_t	mode;
	uint32_t	flags;
	int32_t		new_fd;
	char		path[1];	// space will be allocated when the addopen() is called
} _posix_spawn_file_actions_open_t;

/*
 * because file actions must be processed in the order in which they are
 * added, we grow an array which is a union of the types of actions. A pointer
 * to each action type could be used however this leads to small allocations and
 * 'open' actiosn are the only "special" type. This type is a pointer to a
 * separate area of memory where enough space for all open() args including the
 * variable length 'path' will have been allocated
*/
typedef struct
{
	uint32_t  type;		// of type posix_file_action_type
	union {
		_posix_spawn_file_actions_close_t	close;
		_posix_spawn_file_actions_dup_t		dup;
		_posix_spawn_file_actions_open_t	*open;	// treat as READONLY
	} _type;
} posix_spawn_file_actions_list_t;


/*
 * prototypes
*/
int posix_spawn_file_getactions(const posix_spawn_file_actions_t *_Restrict fact_p,
								int *num, posix_spawn_file_actions_list_t fact_list[]);
void posix_spawnattr_endswap(void *p);
void posix_spawn_file_actions_endswap(void *p);
int file_open_actions_fixup(void *p1, void *p2);

void set_attrp(posix_spawnattr_t *pp, void *p, unsigned key_mask);
void set_factp(posix_spawn_file_actions_t *pp, void *p, unsigned key_mask);


__END_DECLS

#endif	/* _POSIX_SPAWN_H_INCLUDED */

/* __SRCVERSION("posix_spawn.h $Rev: 169304 $"); */
