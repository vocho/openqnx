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
 * attribute init/destroy

 * behaviour as per POSIX with the following implementation defined behaviour
 * 
 * POSIX says results are undefined when re-initializing an already initialized
 * 'posix_spawnattr_t' object. Our implementation does not support the
 * re-initialization of a 'posix_spawnattr_t' object without an intervening
 * destuction of the object. Repeated calls to posix_spawnattr_init() with the
 * same 'posix_spawnattr_t' object without an intervening posix_spawnattr_destroy()
 * on the object could result in a memory leak. 
 * 
 * posix_spawnattr_destroy() will render the 'posix_spawnattr_t' unusable. Before
 * the object can be reused, it must be initialized with posix_spawnattr_init().
 * This includes calling posix_spawnattr_destroy() on an already destroyed object.
 * 
 * Note that POSIX says results are undefined when using a 'posix_spawnattr_t'
 * object after it has been destroyed. Our implementation will cause all get/set
 * operations as well as posix_spawn() to return EINVAL when attempting to
 * reference a destroyed atributes object.
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL for any invalid parameter
 * 
 * 
 * 
 * Implementation Note: (not necessary for interface documentation) 
 * 
 * init will initialize a 'posix_spawnattr_t' structure by setting <*attrp> == NULL.
 * This value indicates the use of default values for posix_spawn() the same way
 * passing NULL for the 'posix_spawnattr_t' parameter specifies the use of default
 * values. The reason that memory is not allocated in the init is so that if
 * a user program calls posix_spawnattr_init() without calling any of the _setxxx()
 * functions, we don't allocate any memory. Also we do not need to allocate
 * memory for a defaults structure. The first _setxxx() call made will cause
 * memory to be allocated. This also works nicely for a statically initialized
 * 'posix_spawnattr_t' object.
*/
int posix_spawnattr_init(posix_spawnattr_t *attrp)
{
	if (attrp == NULL) return EINVAL;
	SET_ATTRP(attrp, (_posix_spawnattr_t *)AKEY(KEY_MASK));	// will cause NULL to be stored
	return EOK;
}

int posix_spawnattr_destroy(posix_spawnattr_t *attrp)
{
	if (!valid_attrp(attrp)) {
		return EINVAL;
	} else {
		_posix_spawnattr_t *_attrp = GET_ATTRP(attrp);
		if (_attrp != NULL) {
			free(_attrp);
			SET_ATTRP(attrp, NULL);
		}
		return EOK;
	}
}


#endif	/* _POSIX_SPAWN */

