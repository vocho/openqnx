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





#if (defined(__GNUC__) || defined(__INTEL_COMPILER)) && !defined(__MIPS__)
void resmgr_pathname_detach() __attribute__ ((weak, alias("_resmgr_pathname_detach")));
#endif
#define RESMGR_COMPAT
#include <string.h>
#include <sys/resmgr.h>
#include "resmgr.h"

int resmgr_pathname_detach(int id, unsigned flags) {
	struct _msg_info 				info;
	struct _resmgr_handle_list		*list;
	struct _resmgr_handle_entry		*entry, *nentry;
	struct binding					*binding;

	if(_resmgr_link_free(id, flags) == -1) {
		return -1;
	}

	memset(&info, 0, sizeof(info));

restart:
	_mutex_lock(&_resmgr_io_table.mutex);

	for (list = _resmgr_io_table.vector, info.scoid = 0; info.scoid < _resmgr_io_table.nentries; list++, info.scoid++) {
		for (entry = list->list; entry != NULL; entry = nentry) {
			nentry = entry->next;
			binding = (struct binding *)entry->handle;
			if (binding->id == id) {
				info.coid = entry->coid;
				info.pid = list->pid;
				info.nd = list->nd;
				info.tid = 0;
				_mutex_unlock(&_resmgr_io_table.mutex);
				_resmgr_handle(&info, binding, _RESMGR_HANDLE_REMOVE);
// 				if(flags & _RESMGR_CLOSE_ON_DETACH)	_resmgr_close_handler(&ctp, binding);
				goto restart;
			}
		}
	}

	_mutex_unlock(&_resmgr_io_table.mutex);

	return 0;
}

__SRCVERSION("resmgr_pathname_detach.c $Rev: 153052 $");
