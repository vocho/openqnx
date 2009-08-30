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

#include "externs.h"


PULSE * rdecl
pulse_add(PROCESS *prp, PRIL_HEAD *queue, int priority, int code, int value, int id) {
	PULSE	*pup;

	if(priority & _PULSE_PRIO_REPLACE) {
		pup = pril_find_insertion(queue, priority | _PULSE_PRIO_HEAD);
		// Scan the queue and replace if priority and code match.
		while((pup != NULL) && (pup->priority == (uint8_t)priority)) {
			if((pup->code == (int8_t)code) && (pup->id == id)) {
				pup->value = value;
				overrun = 1;
				return pup;
			}
			pup = pril_next(pup);
		}
	} else {
		pup = pril_find_insertion(queue, priority);
		if((pup != NULL)
		&& (pup->code == (int8_t)code)
		&& (pup->id == id)
		&& (pup->value == value)
		&& (pup->count < _PULSE_COUNT_MAX)) {
			// We can compress this pulse
			++pup->count;
			return pup;
		}
	}

	// Allocate a pulse entry.
	pup = object_alloc(prp, &pulse_souls);
	if(pup == NULL) {
		if((ker_verbose >= 2) && (pulse_souls.flags & SOUL_CRITICAL)) {
			kprintf("No memory for pulse (%d,%d,%d) - dropped.\n",
						code, value, id);
			DebugKDBreak();
		}
		return NULL;
	}

	// Initialize it.
	if(priority & _PULSE_PRIO_VTID) {
		pup->type = TYPE_VPULSE;
	} else if (priority & _PULSE_PRIO_SIGNAL) {
		pup->type = TYPE_SIGNAL;
	} else {
		pup->type = TYPE_PULSE;
	}
	if(priority & _PULSE_PRIO_BOOST) {
		pup->type |= TYPE_FLAG_FIRST;
	}
	pup->count = 1;
	pup->priority = (uint8_t)priority;
	pup->code = code;
	pup->value = value;
	pup->id = id;

	// Enqueue it.

	// FUTURE: pril_find_insertion() could have returned some information
	// about the PRIL state that would have made it possible to call a
	// different routine from pril_add() making use of the info and doing
	// the insertion faster. Something for another day....
	pril_add(queue, pup);

	return pup;
}


int rdecl
pulse_deliver(CHANNEL *chp, int priority, int code, int value, int id, unsigned pulse_flags) {
// pulse_flags are an or-ing of kermacros.h: PULSE_DELIVER_*_FLAG 
	THREAD *thp;
	int prio = priority & 0xff;

	if((prio == 0) || (((unsigned) prio) > NUM_PRI-1)) {
		return EINVAL;
	}
	
	// Was there was a waiting thread on the channel?
	thp = chp->receive_queue;
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)  
	while((thp != NULL) && (thp->internal_flags & _NTO_ITF_MSG_DELIVERY)) {
		thp = thp->next.thread;
	}
#endif
	if(thp != NULL) {
		int status = 0;

		thp->internal_flags &= ~_NTO_ITF_RCVPULSE;
		thp->restart = NULL;

		// Yes. Transfer the data.
		if(((get_inkernel() & (INKERNEL_LOCK|INKERNEL_NOW)) == INKERNEL_NOW) && thp->aspace_prp == aspaces_prp[KERNCPU]) {
			status = xferpulse(thp, thp->args.ri.rmsg, thp->args.ri.rparts, code, value, id);
			lock_kernel();
		} else {
			if(get_inkernel() & INKERNEL_NOW) {		// We also get called from an irq handler.
				lock_kernel();
			}
			thp->args.ri.code = code;
			thp->args.ri.value = value;
			thp->args.ri.id = id;
			thp->flags |= _NTO_TF_PULSE;
		}

		SETKSTATUS(thp, 0);

		if(status) {
			kererr(thp, EFAULT);
		}

		// Unlink receive thread from the receive queue and ready it.
		LINKPRIL_REM(thp);

		// By default the receiver runs with message driven priority.
		if(thp->priority != prio  &&  (chp->flags & _NTO_CHF_FIXED_PRIORITY) == 0) {
			thp->real_priority = thp->priority = prio;
		}

		if(priority & _PULSE_PRIO_VTID) {
			thp->flags |= _NTO_TF_RCVINFO;
			// If the caller set the priority to _PULSE_PRIO_VTID, then they passed in
			// the sender's THREAD object as an id (this is done in net_send2)
			thp->args.ri.thp  = (THREAD *)id;
			thp->args.ri.cop  = thp->args.ri.thp->blocked_on;
			// indicate that there is a receiver depending on data in our THREAD object
			((THREAD *)id)->internal_flags |= _NTO_ITF_SPECRET_PENDING;
		}
        if(pulse_flags & PULSE_DELIVER_APS_CRITICAL_FLAG) {
			AP_MARK_THREAD_CRITICAL(thp); 
		}
		ready(thp);
		return status ? ESRVRFAULT : EOK;
	}

	//Fail any pulse delivery attempt to a dying process.
	if(chp->process->flags & (_NTO_PF_TERMING | _NTO_PF_ZOMBIE | _NTO_PF_COREDUMP)) {
		return ENXIO;
	}

	if(get_inkernel() & INKERNEL_NOW) {		// We also get called from an irq handler.
		lock_kernel();
	}

	// No-one waiting for a msg, 
	// we loose the _PULSE_DELIVER_APS_CRITICAL state at this point. PR27529
	if(pulse_add(chp->process, &chp->send_queue, priority, code, value, id)) {
		// To prevent priority inversion, boost all threads in the server
		if((chp->flags & _NTO_CHF_FIXED_PRIORITY) == 0) {
			int i;
			//note adjust prios, but do not inherit APS partition or APS critical state
			for(i = 0 ; i < chp->process->threads.nentries ; ++i) {
				if(VECP(thp, &chp->process->threads, i)  &&  thp->priority < prio
				&&  thp->last_chid == chp->chid) {
					adjust_priority(thp, prio, thp->dpp, 1);
					thp->real_priority = thp->priority;
				}
			}
		}

		return EOK;
	}

	return EAGAIN;
}


void rdecl
pulse_remove(PROCESS *prp, PRIL_HEAD *queue, PULSE *pup) {
	if(--pup->count == 0) {
		pril_rem(queue, pup);
		object_free(prp, &pulse_souls, pup);
	}
}

__SRCVERSION("nano_pulse.c $Rev: 169542 $");
