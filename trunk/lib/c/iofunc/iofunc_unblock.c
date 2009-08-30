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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_unblock(resmgr_context_t *ctp, iofunc_attr_t *attr) {
	iofunc_lock_list_t				*l;

	(void)_iofunc_llist_lock(attr);
	for(l = PTR_VALUE(attr->lock_list, iofunc_lock_list_t); l; l = l->next) {
		struct _iofunc_lock_blocked		*p, **pp;
		struct _msg_info				info;

		for(pp = &l->blocked; (p = *pp); pp = &p->next) {
			if(p->rcvid == ctp->rcvid &&
				MsgInfo_r(p->rcvid, &info) == EOK &&
				(info.flags & _NTO_MI_UNBLOCK_REQ)) {
				*pp = p->next;
				MsgError(p->rcvid, EINTR);
				free(p);
				_iofunc_llist_unlock(attr);
				return _RESMGR_NOREPLY;
			}
		}
	}
	_iofunc_llist_unlock(attr);
	return _RESMGR_DEFAULT;
}

__SRCVERSION("iofunc_unblock.c $Rev: 153052 $");
