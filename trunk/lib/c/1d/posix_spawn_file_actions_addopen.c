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
 * addopen
 * 
 * behaviour as per POSIX
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL for any invalid parameter
 * 		ENOMEM if the action could not be added to the file actions object
*/
int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t *_Restrict fact_p, int new_fd,
									 const char *_Restrict path, int oflags, mode_t omode)
{
	if (!valid_factp(fact_p) || (new_fd < 0)) {
		return EINVAL;
	} else {
		_posix_spawn_file_actions_t *_fact_p = GET_FACTP(fact_p);

		if (_fact_p == NULL) {
			if ((_fact_p = calloc(1, sizeof(*_fact_p))) == NULL) return ENOMEM;
			SET_FACTP(fact_p, _fact_p);
			return posix_spawn_file_actions_addopen(fact_p, new_fd, path, oflags, omode);
		} else {
			unsigned num = _fact_p->num_entries + 1;
			_posix_spawn_file_actions_open_t  *open_action;
			size_t  len = ROUNDUP(sizeof(*open_action) + strlen(path), 4);
			if ((open_action = malloc(len)) == NULL) return ENOMEM;

			if (num > 1) {		// not the first time
				_fact_p = realloc(_fact_p, FILE_ACTIONS_T_SIZE(num));
				if (_fact_p == NULL) {
					free(open_action);
					return ENOMEM;
				}
				SET_FACTP(fact_p, _fact_p);
			}
			_fact_p->action[_fact_p->num_entries].type = posix_file_action_type_OPEN;
			_fact_p->action[_fact_p->num_entries]._type.open = open_action;
			_fact_p->action[_fact_p->num_entries]._type.open->size = len;
			_fact_p->action[_fact_p->num_entries]._type.open->mode = omode;
			_fact_p->action[_fact_p->num_entries]._type.open->flags = oflags;
			_fact_p->action[_fact_p->num_entries]._type.open->new_fd = new_fd;
			memcpy(_fact_p->action[_fact_p->num_entries]._type.open->path, path, strlen(path)+1);
			++_fact_p->num_entries;
			return EOK;
		}
	}
}


#endif	/* _POSIX_SPAWN */

