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

#include "posix_spawn_file_actions.h"


/*
 * =============================================================================
 * 
 * 			The following are NON PUBLIC QNX posix_spawn_file_actions_t
 * 			extensions used by proc to parse a _PROC_POSIX_SPAWN message
 * 					(DO NOT DOCUMENT in library reference guide)
 * 
 * =============================================================================
*/

/*******************************************************************************
 * posix_spawn_file_actions_endswap
 * 
 * Do an endian swap on the '_posix_spawn_file_actions_t' pointed to by <p>.
 * This is an internal routine used by procmgr_pspawn()
 * Perhaps it should only compile when procnto is being built ?
*/
void posix_spawn_file_actions_endswap(void *p)
{
	unsigned i;
	_posix_spawn_file_actions_t *_factp = p;

	if (_factp == NULL) return;

	ENDIAN_SWAP32(&_factp->num_entries);
	for (i=0; i<_factp->num_entries; i++)
	{
		ENDIAN_SWAP32(&_factp->action[i].type);
		switch (_factp->action[i].type)
		{
			case posix_file_action_type_CLOSE:
				ENDIAN_SWAP32(&_factp->action[i]._type.close.fd);
				break;
			case posix_file_action_type_DUP:
				ENDIAN_SWAP32(&_factp->action[i]._type.dup.fd);
				ENDIAN_SWAP32(&_factp->action[i]._type.dup.new_fd);
				break;
			case posix_file_action_type_OPEN:
				ENDIAN_SWAP32(&_factp->action[i]._type.open->size);
				ENDIAN_SWAP32(&_factp->action[i]._type.open->new_fd);
				ENDIAN_SWAP32(&_factp->action[i]._type.open->flags);
				ENDIAN_SWAP32(&_factp->action[i]._type.open->mode);
				break;
			default:
				break;
		}
	}
}


/*******************************************************************************
 * file_open_actions_fixup
 * 
 * This is a procmgr support routine only. Because of the way that the
 * '_posix_spawn_file_actions_open_t' structures are stored (pointers within
 * the '_posix_spawn_file_actions_t' structure) when the structures are
 * messaged to procnto, the open actions will be appended to the end of the
 * message (if there are any). This routine will fix those pointers up in
 * the procmgr local message buffer
 * 
 * <p1> is expected to be a pointer to a '_posix_spawn_file_actions_t' structure
 * and <p2> to a stream of bytes which are a series of '_posix_spawn_file_actions_open_t'
 * structures
*/
int file_open_actions_fixup(void *p1, void *p2)
{
	if ((p1 == NULL) || (p2 == NULL)) {
		return EINVAL;
	} else {
		unsigned i;
		_posix_spawn_file_actions_t *_factp = (_posix_spawn_file_actions_t *)p1;
		_posix_spawn_file_actions_open_t *open_action = (_posix_spawn_file_actions_open_t *)p2;

		for (i=0; i<_factp->num_entries; i++) {
			if (_factp->action[i].type == posix_file_action_type_OPEN) {
				_factp->action[i]._type.open = open_action;
				open_action = (_posix_spawn_file_actions_open_t *)((unsigned)open_action + open_action->size);
			}
		}
		return EOK;
	}
}

/*******************************************************************************
 * get_factp
 * set_factp
 * 
 * These 2 routine translate between a real allocated
 * '_posix_spawn_file_actions_t *' and the token stored in the
 * 'posix_spawn_file_actions_t *'.
 * 
 * A valid, initialized '_posix_spawn_file_actions_t' object has 2 possible
 * values.
 * 		- NULL
 * 		- a valid, allocated address
 * All other values are considered invalid. These 2 routines attempt to catch
 * uninitialized 'posix_spawn_file_actions_t' objects from being used. The
 * example is an automatic 'posix_spawn_file_actions_t' which contains stack
 * garbage. It is non NULL but does not represent valid, allocated memory.
 * 
 * Because we know that allocated memory is always aligned on given boundary
 * we use a key that will always turn a properly allocated memory address into
 * a token which always has the lower 3 bits set (8 byte alignment). This works
 * for all bogus values that do not have 7 as the LSB 
*/
void set_factp(posix_spawn_file_actions_t *pp, void *_p, unsigned key_mask)
{
	_posix_spawn_file_actions_t *p = (_posix_spawn_file_actions_t *)_p;
	assert(pp != NULL);
	assert(((unsigned)p == FKEY(key_mask)) || (((unsigned)p & key_mask) == 0)); // must be aligned or keying will fail
	*pp = ((unsigned)p ^ FKEY(key_mask));
}


#endif	/* _POSIX_SPAWN */

