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

#include "posix_spawnattr.h"


/*******************************************************************************
 * get/setflags
 * 
 * behaviour as per POSIX
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL for any invalid parameter
*/
int posix_spawnattr_setflags(posix_spawnattr_t *attrp, short flags)
{
	if (!valid_attrp(attrp)) {
		return EINVAL;
	} else {
		/* we set the lower 16 bit without disturbing the upper 16 bits */
#define _posix_setflags_mask	(0xFFFFFFFFU >> ((sizeof(xflags) - sizeof(flags)) * 8))
		static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;	// for rmw operation
		uint32_t  xflags;
		int r;
		pthread_mutex_lock(&lock);
		if ((r = posix_spawnattr_getxflags(attrp, &xflags)) == EOK) {
			xflags &= ~_posix_setflags_mask;
			xflags |= (uint32_t)flags;
			r = posix_spawnattr_setxflags(attrp, xflags);
		}
		pthread_mutex_unlock(&lock);
		return r;
	}
}

/*
 * =============================================================================
 * 
 * 				The following are QNX posix_spawnattr_t extensions
 * 
 * =============================================================================
*/

/*
 * any flag not defined in this set will not be able to be set and hence not
 * cause any actions.
*/
#define POSIX_FLAGS_IMPLEMENTED \
		( 0 \
			| POSIX_SPAWN_SETPGROUP \
			| POSIX_SPAWN_SETSIGMASK \
			| POSIX_SPAWN_SETSIGDEF \
			| POSIX_SPAWN_SETSCHEDULER \
			| POSIX_SPAWN_SETSCHEDPARAM \
			| POSIX_SPAWN_RESETIDS \
			| POSIX_SPAWN_SETSIGIGN \
			| POSIX_SPAWN_SETMPART \
			| POSIX_SPAWN_SETSPART \
	/* not yet		| POSIX_SPAWN_SETND */ \
			| POSIX_SPAWN_EXPLICIT_CPU \
			| POSIX_SPAWN_SETSTACKMAX \
			| POSIX_SPAWN_NOZOMBIE \
			| POSIX_SPAWN_ALIGN_DEFAULT \
			| POSIX_SPAWN_ALIGN_FAULT \
			| POSIX_SPAWN_ALIGN_NOFAULT \
		)

/*******************************************************************************
 * getx/setxflags - extended flags
 * 
 * Retrieve/specify all of the possible attribute flags that will/should be set
 * when a process is spawned.
 * 
 * POSIX specifies posix_spawnattr_setflags()/posix_spawnattr_getflags() routines
 * for the setting of predefined flags using a 'flags' parameter which is defined
 * as a 'short' type. QNX provides several extended attributes settings and so
 * we also extend the range of possible flags values using a uint32_t type for
 * these extended flags.
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL for any invalid parameter
 * 		ENOMEM if the partition id could not be added to the attributes object
*/
int posix_spawnattr_setxflags(posix_spawnattr_t *attrp, uint32_t flags)
{
	if ((!valid_attrp(attrp)) || (flags & ~POSIX_FLAGS_IMPLEMENTED)) {
		return EINVAL;
	} else {
		_posix_spawnattr_t *_attrp = GET_ATTRP(attrp);

		if (_attrp == NULL) {
			if ((_attrp = _posix_spawnattr_t_realloc(NULL, 0)) == NULL) return ENOMEM;
			SET_ATTRP(attrp, _attrp);
		}
		_attrp->flags = flags;
		return EOK;
	}
}


#endif	/* _POSIX_SPAWN */

