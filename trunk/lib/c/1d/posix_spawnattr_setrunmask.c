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


/*
 * =============================================================================
 * 
 * 				The following are QNX posix_spawnattr_t extensions
 * 
 * =============================================================================
*/

/*******************************************************************************
 * get/setrunmask
 * 
 * Retrieves/specifies a bit mask of processors on which the spawned program
 * will/should be restricted to run on. Processor numbering starts at 0 with each
 * processor corresponding to a bit in the runmask.
 * 
 * Note that this setting only takes effect if the POSIX_SPAWN_EXPLICIT_CPU flag
 * is set.
 * The default mask is 0xFFFFFFFFU indicating that the spawned process can run
 * on any processor. The default value can also be retrieved with a call to
 * posix_spawnattr_getrunmask() using an initialized 'posix_spawnattr_t' object.
 * 
 * ex.
 * 		a <runmask> == 0x00000005 would indicate that the spawned process is to
 * 		be restricted to CPU's 0 and 2
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL if there are any parameter errors
*/
int posix_spawnattr_setrunmask(posix_spawnattr_t *attrp, uint32_t runmask)
{
	if (!valid_attrp(attrp)) {
		return EINVAL;
	} else {
		_posix_spawnattr_t *_attrp = GET_ATTRP(attrp);

		if (_attrp == NULL) {
			if ((_attrp = _posix_spawnattr_t_realloc(NULL, 0)) == NULL) return ENOMEM;
			SET_ATTRP(attrp, _attrp);
		}
		_attrp->sched.runmask = runmask;
		return EOK;
	}
}


#endif	/* _POSIX_SPAWN */

