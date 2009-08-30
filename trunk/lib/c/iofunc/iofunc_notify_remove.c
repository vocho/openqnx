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




#include <malloc.h>
#include <sys/iofunc.h>


void
iofunc_notify_remove(resmgr_context_t *ctp, iofunc_notify_t *nop) {
	iofunc_notify_remove_strict(ctp, nop, 3);
	return;
}

void
iofunc_notify_remove_strict(resmgr_context_t *ctp, iofunc_notify_t *nop, int lim) {
	iofunc_notify_event_t	*prev, *nep;
	int						 i;

	for(i = 0 ; i < lim ; ++i) {
		for(prev = (iofunc_notify_event_t *)&(nop + i)->list ; (nep = prev->next) ; prev = nep) {
			if(!ctp || (nep->scoid == ctp->info.scoid && nep->coid == ctp->info.coid)) {
				prev->next = nep->next;
				free(nep);
				nep = prev;
			}
		}
	}
}

__SRCVERSION("iofunc_notify_remove.c $Rev: 153052 $");
