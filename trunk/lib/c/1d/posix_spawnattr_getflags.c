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
int posix_spawnattr_getflags(const posix_spawnattr_t *_Restrict attrp, short *_Restrict flags_p)
{

	if (!valid_attrp(attrp) || (flags_p == NULL)) {
		return EINVAL;
	} else {
		uint32_t  flags;
#define _posix_getflags_mask	(0xFFFFFFFFU >> ((sizeof(flags) - sizeof(*flags_p)) * 8))
		int r = posix_spawnattr_getxflags(attrp, &flags);
		*flags_p = flags & _posix_getflags_mask;
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
int posix_spawnattr_getxflags(const posix_spawnattr_t *_Restrict attrp, uint32_t *_Restrict flags_p)
{
	if (!valid_attrp(attrp) || (flags_p == NULL)) {
		return EINVAL;
	} else {
		_posix_spawnattr_t *_attrp = GET_ATTRP(attrp);
		*flags_p = (_attrp == NULL) ? _default_posix_spawnattr_t.flags : _attrp->flags;
		return EOK;
	}
}


#endif	/* _POSIX_SPAWN */

