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
 * get/setsigdfault
 * 
 * behaviour as per POSIX
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL for any invalid parameter
*/
int posix_spawnattr_setsigdefault(posix_spawnattr_t *_Restrict attrp, const sigset_t *_Restrict sigset_p)
{
	if (!valid_attrp(attrp) || (sigset_p == NULL)) {
		return EINVAL;
	} else {
		_posix_spawnattr_t *_attrp = GET_ATTRP(attrp);

		if (_attrp == NULL) {
			if ((_attrp = _posix_spawnattr_t_realloc(NULL, 0)) == NULL) return ENOMEM;
			SET_ATTRP(attrp, _attrp);
		}
		_attrp->sig.dflt = *sigset_p;
		return EOK;
	}
}

/*******************************************************************************
 * get/setsigmask
 * 
 * behaviour as per POSIX
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL for any invalid parameter
*/
int posix_spawnattr_setsigmask(posix_spawnattr_t *_Restrict attrp, const sigset_t *_Restrict sigset_p)
{
	if (!valid_attrp(attrp) || (sigset_p == NULL)) {
		return EINVAL;
	} else {
		_posix_spawnattr_t *_attrp = GET_ATTRP(attrp);

		if (_attrp == NULL) {
			if ((_attrp = _posix_spawnattr_t_realloc(NULL, 0)) == NULL) return ENOMEM;
			SET_ATTRP(attrp, _attrp);
		}
		_attrp->sig.mask = *sigset_p;
		return EOK;
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
 * get/setsigignore
 * 
 * Retrieve/specify the set of signals that will/should be ignored by the spawned
 * process.
 * 
 * Note that this setting only takes effect if the POSIX_SPAWN_SETSIGIGN flag
 * is set.
 * The default value can be retrieved with a call to posix_spawnattr_getsigignore()
 * using an initialized 'posix_spawnattr_t' object.
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL if there are any parameter errors
*/
int posix_spawnattr_setsigignore(posix_spawnattr_t *_Restrict attrp, const sigset_t *_Restrict sigset_p)
{
	if (!valid_attrp(attrp) || (sigset_p == NULL)) {
		return EINVAL;
	} else {
		_posix_spawnattr_t *_attrp = GET_ATTRP(attrp);

		if (_attrp == NULL) {
			if ((_attrp = _posix_spawnattr_t_realloc(NULL, 0)) == NULL) return ENOMEM;
			SET_ATTRP(attrp, _attrp);
		}
		_attrp->sig.ignore = *sigset_p;
		return EOK;
	}
}


#endif	/* _POSIX_SPAWN */

