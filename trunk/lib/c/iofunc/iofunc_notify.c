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
#include <errno.h>
#include <sys/iomsg.h>
#include <sys/iofunc.h>
#include "iofunc.h"


static iofunc_notify_event_t *
notify_lookup(int rcvid, iofunc_notify_t *nop) {
	iofunc_notify_event_t	*nep;

	// A quick match and return will be the normal steady state case.
	for(nep = nop->list ; nep ; nep = nep->next) {
		if(nep->rcvid == rcvid) {
			return nep;
		}
	}

	if((nep = malloc(sizeof(*nep))) == NULL) {
		return NULL;
	}

	nep->next = nop->list;
	nop->list = nep;
			
	return nep;
}


int iofunc_notify(resmgr_context_t *ctp, io_notify_t *msg, iofunc_notify_t *nop, int trig, const int *notifycnts, int *armed) {
	iofunc_notify_event_t	*nep;
	unsigned				msgflags;
	int						i, action, dummy, lim, ext;
	static const int		ones[_NOTIFY_MAXCOND] = {
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1,
		1, 1, 1, 1, 1
	};



	if(notifycnts == NULL) {
		notifycnts = ones;
	}

	lim = 3;
	if(armed == NULL) {
		armed = &dummy;
	}	
	else if(trig & _NOTIFY_COND_EXTEN) {
		lim = min(_NOTIFY_MAXCOND, *armed);
	}
	*armed = 0;

	/*
	 * Set corresponding _NOTIFY_COND / _NOTIFY_CONDE bits
	 * to catch clients that set either set in a single test.
	 */
	trig |= (unsigned)trig << 28;
	trig &= ~_NOTIFY_COND_EXTEN;
	trig |= (unsigned)trig >> 28;


	// Calculate trigger conditions and disarm each source.
	action = msg->i.action;

	msgflags = msg->i.flags;
	ext = msgflags & _NOTIFY_COND_EXTEN;
	msgflags &= ~_NOTIFY_COND_EXTEN;

	if(action != _NOTIFY_ACTION_TRANARM) {
		trig &= msgflags;
	} else {
		//
		// POSIX message queues only allow one transition arm (mq_notify)
		// at a time. We apply the rule for all resource managers.
		//
		for(nep = nop[IOFUNC_NOTIFY_INPUT].list ; nep ; nep = nep->next) {
			if(SIGEV_GET_TYPE(&nep->event) != SIGEV_NONE) {
				if(nep->scoid != ctp->info.scoid ||
				    SIGEV_GET_TYPE(&msg->i.event) != SIGEV_NONE) {
					return EBUSY;
				}
			}
		}
		trig = 0;
	}

	/*
	 * trig now contains satisfied events as asked for
	 * by the client (_NOTIFY_COND* or _NOTIFY_CONDE*).
	 * We don't change this so that the flags the client
	 * gets in the reply are the same as it sent.  However,
	 * we'll convert msgflags to the extended set
	 * (_NOTIFY_CONDE) to facilitate an early out of the loop
	 * as clients will only ever be looking past the first
	 * 3 events for poll().
	 */

	msgflags |= msgflags >> 28;
	msgflags &= ((1 << _NOTIFY_MAXCOND) - 1);

	/* Lookup a notify entry for each src. */
	for(i = 0 ; i < lim && msgflags; i++, msgflags >>= 1, nop++) {
		/* Only affect sources specified */
		if((msgflags & 1) == 0) {
			continue;
		}

		/* Lookup and create a notify entry if needed. */
		if((nep = notify_lookup(ctp->rcvid, nop)) == NULL)
			return ENOMEM;

		nep->rcvid = ctp->rcvid;
		nep->scoid = ctp->info.scoid;
		nep->coid = ctp->info.coid;
		nep->flags = ext;

		/*
		 * Arm each source if no triggers and not a straight poll,
		 * otherwise ensured disarmed.
		 */
		if(trig == 0 && action != _NOTIFY_ACTION_POLL) {
			nep->event = msg->i.event;

			if(action == _NOTIFY_ACTION_TRANARM) {
				nep->cnt = 0;
			} else {
				nep->cnt = notifycnts[i];
				*armed = 1;
			}

			if(nep->cnt < nop->cnt) {
				nop->cnt = nep->cnt;
			}
		}
		else {
			SIGEV_NONE_INIT(&nep->event);
			nep->cnt = INT_MAX;
		}
	}

	if ((msg->o.flags = trig))
		msg->o.flags |= ext;

	// A poll tries to return status bits.
	if(action & _NOTIFY_ACTION_POLL) {	// *_POLL or *_POLLARM
		return _RESMGR_PTR(ctp, &msg->o, sizeof(msg->o));
	}

	if(trig || action != _NOTIFY_ACTION_CONDARM) {
		return _RESMGR_PTR(ctp, &msg->o, sizeof(msg->o));
	}

	return EAGAIN;
}

__SRCVERSION("iofunc_notify.c $Rev: 168079 $");
