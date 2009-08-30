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




#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include <sys/resmgr.h>
#include "iofunc.h"


void
iofunc_notify_trigger(iofunc_notify_t *nop, int cnt, int index) {
	iofunc_notify_trigger_strict(NULL, nop, cnt, index);
	return;
}

void
iofunc_notify_trigger_strict(resmgr_context_t *ctp, iofunc_notify_t *nop, int cnt, int index) {
	iofunc_notify_event_t	*nep;

	// Trigger all events which have a trigger cnt.
	
	nop += index;
	nop->cnt = INT_MAX;
	for(nep = nop->list ; nep ; nep = nep->next) {
		/* SIGEV_NONE also implies nep->cnt == INT_MAX */
		if(SIGEV_GET_TYPE(&nep->event) == SIGEV_NONE)
			continue;
		if((ctp == NULL || (ctp->info.scoid == nep->scoid &&
		    ctp->info.coid == nep->coid)) && nep->cnt <= cnt) {
			if(SIGEV_GET_TYPE(&nep->event) >= SIGEV_SIGNAL  &&
			   SIGEV_GET_TYPE(&nep->event) <= SIGEV_PULSE  &&
			   nep->event.sigev_code == SI_NOTIFY) {
				nep->event.sigev_value.sival_int |= _NOTIFY_COND_INPUT << index;
				if(nep->flags & _NOTIFY_COND_EXTEN) {
					nep->event.sigev_value.sival_int |= _NOTIFY_COND_EXTEN;
					/*
					 * Don't have enough bits in sival_int for
					 * extended flags.  If they've asked for
					 * the extended events, it means they're
					 * expecting the code to be overwritten.
					 */
					nep->event.sigev_code = -(_NOTIFY_CONDE_RDNORM << index);
				}
			}
			(void)MsgDeliverEvent(nep->rcvid, &nep->event);
			nep->event.sigev_notify = SIGEV_NONE;
			nep->cnt = INT_MAX;
		}

		// Calculate a new min cnt on the fly for the interrupt handler.
		if(nop->cnt > nep->cnt)
			nop->cnt = nep->cnt;
	}
}

__SRCVERSION("iofunc_notify_trigger.c $Rev: 168079 $");
