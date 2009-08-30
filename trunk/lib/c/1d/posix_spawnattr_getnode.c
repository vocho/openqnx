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
 * get/setnode
 * 
 * Retrieves/specifies the remote node on which the spawn will/should be done.
 * The netmgr_strtond() call can be used to obtain a valid node identifier for
 * a named remote node.
 * 
 * Note that this setting only takes effect if the POSIX_SPAWN_SETND flag is set
 * The default value will be the node from which the posix_spawn() call is
 * executed.
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL if there are any parameter errors
*/
int posix_spawnattr_getnode(const posix_spawnattr_t *_Restrict attrp, uint32_t *_Restrict node_p)
{
	if (!valid_attrp(attrp) || (node_p == NULL)) {
		return EINVAL;
	} else {
		_posix_spawnattr_t *_attrp = GET_ATTRP(attrp);
		*node_p = (_attrp == NULL) ? _default_posix_spawnattr_t.node : _attrp->node;
	}
	return EOK;
}


#endif	/* _POSIX_SPAWN */

