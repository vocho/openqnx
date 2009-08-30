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





#include <stdlib.h>
#include <errno.h>
#include <sys/resmgr.h>
#include "resmgr.h"

int
resmgr_handle_tune(int min_handles, int min_clients, int max_client_handles,
	int *min_handles_old, int *min_clients_old, int *max_client_handles_old)
{
	struct _resmgr_handle_list *list;
	int i;

	_mutex_lock(&_resmgr_io_table.mutex);

	if (min_handles_old != NULL)
		*min_handles_old = _resmgr_io_table.min;

	if (min_clients_old != NULL)
		*min_clients_old = _resmgr_io_table.min_buckets;

	if (max_client_handles_old != NULL)
		*max_client_handles_old = _resmgr_io_table.nlists_max;



	/* _resmgr_handle() will store them away as it goes */
	if (min_handles >= 0)
		_resmgr_io_table.min = min_handles;

	/* _resmgr_handle() will store them away as it goes */
	if (min_clients >= 0)
		_resmgr_io_table.min_buckets = min_clients;

	if (max_client_handles >= 0) {
		max_client_handles = max(max_client_handles, _RESMGR_CLIENT_FD_MIN);
		_resmgr_io_table.nlists_max = max_client_handles;
		for (i = 0, list = _resmgr_io_table.vector; i <  _resmgr_io_table.nentries; i++, list++) {
			/*
			 * We can't change it if there's already entries
			 * as they've been sorted based on this value.
			 * The new value will take effect when the client
			 * closes its coids.
			 */
			if (list->list == NULL) {
				list->nlists_max = _resmgr_io_table.nlists_max;
			}
		}
	}

	_mutex_unlock(&_resmgr_io_table.mutex);

	return 0;
}

__SRCVERSION("resmgr_handle_tune.c $Rev: 153052 $");
