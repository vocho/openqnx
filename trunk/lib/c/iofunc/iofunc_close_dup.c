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
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_close_dup(resmgr_context_t *ctp, io_close_t *msg, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
	if((ocb->ioflag & (_IO_FLAG_RD | _IO_FLAG_WR)) && (ocb->flags & IOFUNC_OCB_MMAP) == 0) {
		iofunc_lock_list_t			*head, *p, **pp;
		int							(*attr_lock)(iofunc_attr_t *), (*attr_unlock)(iofunc_attr_t *);
		iofunc_funcs_t				*funcs;

		if (attr->mount == NULL || (funcs = attr->mount->funcs) == NULL || funcs->nfuncs < (offsetof(iofunc_funcs_t, attr_unlock) / sizeof(void *)) || (attr_lock = funcs->attr_lock) == NULL || (attr_unlock = funcs->attr_unlock) == NULL) {
			attr_lock = iofunc_attr_lock, attr_unlock = iofunc_attr_unlock;
		}

		(void)(*attr_lock)(attr);
		(void)_iofunc_llist_lock(attr);
		head = attr->lock_list, PTR_UNLOCK(head);
		
		pp = &head;
		while((p = *pp)) {
			if(p->scoid == ctp->info.scoid) {
				/* Remove any pending block on this scoid
				   TODO: Push this code into a purge scoid section
				struct _iofunc_lock_blocked *tb, **pb;
				for (tb=p->blocked, pb=&p->blocked; 
					 tb; 
					 tb=tb->next, pb = &tb->next) {
					if (tblocked->pflock && 
						tblocked->pflock->l_sysid == p->scoid) {
						*pb = tb->next;
						_iofunc_blocked_free(tb);
						tb = *pb;
					}
				}
				*/

				(void)_iofunc_unlock_scoid(&head, p->scoid, p->start, p->end);
				/* Don't adjust the pointer, since it will now
				   be filled in assuming that unblock did it's 
				   job properly and removed/unblocked items. */
			} else {
				pp = &p->next;
			}
		}

		PTR_LOCK(head), attr->lock_list = head;
		_iofunc_llist_unlock(attr);
		(void)(*attr_unlock)(attr);
	}
	return EOK;
}

__SRCVERSION("iofunc_close_dup.c $Rev: 153052 $");
