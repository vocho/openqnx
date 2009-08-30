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
 * 			The following are QNX posix_spawn_file_actions_t extensions
 * 
 * =============================================================================
*/

/*******************************************************************************
 * getactions
 * 
 * Retrieve the file actions currently set in the 'posix_spawn_file_actions_t'
 * object
 * 
 * The caller must provide the storage for _posix_spawn_file_actions_t's to be
 * returned. The <num> pointer is initialized by the caller with the number of
 * '_posix_spawn_file_actions_t' elements <fact_list> has space for.
 * Upon return, <num> will be set as follows ...
 * 		- if the actual number of entries is <= the number for which space was
 * 		  allocated by the caller, then the actual number will be filled in and
 * 		  <*num> will be set to the actual number of entries.
 * 		- if the actual number of entries is > the number for which space was
 * 		  allocated by the caller and as specified by <*num>, then <*num> entries
 * 		  will be filled in and <*num> will be set to negative of the number of
 * 		  entries remaining that would not fit.
 * 
 * 		  For example, if the caller provides space for 4 entries and hence
 * 		  sets <*num> == 4 and there are 6 entries in the posix_spawn_file_actions_t
 * 		  object, then 4 entries will be placed into the structure and <*num>
 * 		  will be set to -2. If the caller sets <*num> == 0, then they can easly
 * 		  find out how many entries there are and allocate enough space to
 * 		  retrieve all of the entries.
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL if there are any parameter errors
*/
int posix_spawn_file_getactions(const posix_spawn_file_actions_t *_Restrict fact_p,
								int *num, posix_spawn_file_actions_list_t fact_list[])
{
	if (!valid_factp(fact_p) || (num == NULL) || (*num < 0)) {
		return EINVAL;
	} else {
		_posix_spawn_file_actions_t *_fact_p = GET_FACTP(fact_p);

		if (_fact_p == NULL) {
			*num = 0;
		} else {
			unsigned num_entries = min(_fact_p->num_entries, *num);
			unsigned i;

			for (i=0; i<num_entries; i++) {
				fact_list[i] = _fact_p->action[i];
			}
			if (num_entries < _fact_p->num_entries) {
				*num = -(_fact_p->num_entries - num_entries);
			} else {
				*num = _fact_p->num_entries;
			}
		}
		return EOK;
	}
}


#endif	/* _POSIX_SPAWN */

