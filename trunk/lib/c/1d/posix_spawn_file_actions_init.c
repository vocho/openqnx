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


/*******************************************************************************
 * file actions object init/destroy
 * 
 * behaviour as per POSIX with the following implementation defined behaviour
 * 
 * POSIX says results are undefined when re-initializing an already initialized
 * 'posix_spawn_file_actions_t' object. Our implementation does not support the
 * re-initialization of a 'posix_spawn_file_actions_t' object without an
 * intervening destruction of the object. Repeated calls to
 * posix_spawn_file_actions_init() with the same 'posix_spawn_file_actions_t'
 * object without an intervening posix_spawn_file_actions_destroy() on the
 * object could result in a memory leak. 
 * 
 * posix_spawn_file_actions_destroy() will render the 'posix_spawn_file_actions_t'
 * object unusable. Before the object can be reused, it must be initialized with
 * posix_spawn_file_actions_init().
 * This includes calling posix_spawn_file_actions_destroy() on an already
 * destroyed object.
 * 
 * Note that POSIX says results are undefined when using a
 * 'posix_spawn_file_actions_t' object after it has been destroyed. Our
 * implementation will cause all get/set operations as well as posix_spawn() to
 * return EINVAL when attempting to reference a destroyed file actions object.
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL for any invalid parameter
 * 
 * 
 * 
 * Implementation Note: (not necessary for interface documentation) 
 * 
 * init will initialize a 'posix_spawn_file_actions_t' structure by setting
 * <*factp> == NULL. This value indicates the use of default values for
 * posix_spawn() the same way passing NULL for the 'posix_spawn_file_actions_t'
 * parameter specifies the use of default values. The reason that memory is not
 * allocated in the init is so that if a user program calls
 * posix_spawn_file_actions_init() without calling any of the _setxxx()
 * functions, we don't allocate any memory. Also we do not need to allocate
 * memory for a defaults structure. The first _setxxx() call made will cause
 * memory to be allocated. This also works nicely for a statically initialized
 * 'posix_spawn_file_actions_t' object.
*/
int posix_spawn_file_actions_init(posix_spawn_file_actions_t *fact_p)
{
	if (fact_p == NULL) return EINVAL;
	SET_FACTP(fact_p, (_posix_spawn_file_actions_t *)FKEY(KEY_MASK));	// will cause NULL to be stored
	return EOK;
}

int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *fact_p)
{
	if (!valid_factp(fact_p)) {
		return EINVAL;
	} else {
		_posix_spawn_file_actions_t *_fact_p = GET_FACTP(fact_p);
		if (_fact_p != NULL) {
			unsigned i;
			for (i=0; i<_fact_p->num_entries; i++) {
				if (_fact_p->action[i].type == posix_file_action_type_OPEN) {
					free(_fact_p->action[i]._type.open);
				}
			}
			free(_fact_p);
			SET_FACTP(fact_p, NULL);
		}
		return EOK;
	}
}


#endif	/* _POSIX_SPAWN */

