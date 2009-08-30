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
#include "apmgr/apmgr.h"
#include <kernel/event.h>

/*
 * apm_deliver_event
 * 
 * An apm event can be the result of a procnto thread or a kernel call by either
 * a procnto or user thread.
 * 
 * Kernel Call
 * 		If we are already in the kernel, then we defer the event delivery. This
 * 		is because many of the kernel call functions which cause memory to be
 * 		allocated (and hence land us in this routine) do not expect the side
 * 		effects (ie scheduling ops) performed by sigevent_exe() which would
 * 		normally be used to deliver the event.
 * 
 * procnto thread
 * 		Normally, you would expect that MsgDeliverEvent() could be used to send
 * 		the event from procnto to the registered user process and most of the
 * 		time this works. However, when a new process is created we can have a
 * 		the loader thread working on behalf of the new process and in that case
 * 		MsgDeliverEvent() won't work (returns ESRCH) because the receive id is
 * 		relevant to procnto and not the process the loader thread is working on
 * 
 * Bottom line solution for both of these cases is that we record the 'pid_t'
 * for the process which registers for event notification and then use that
 * information to build up the parameters to either the sigevent_exe() or
 * event_add() call.
 * 
*/
#ifndef NDEBUG
static unsigned evt_delivery_fail_cnt = 0;
static unsigned inker_evt_delivery_fail_cnt = 0;
static unsigned delivered_events = 0;
static unsigned undelivered_events = 0;
#endif	/* NDEBUG */


struct kerargs_deliver_event
{
	evtdest_t *evtdest;
	struct sigevent *event;
	int err;
};

static void ker_deliver_event(void *data)
{
	THREAD	*thp;
	struct kerargs_deliver_event *kap = data;
	PROCESS *prp = lookup_pid(kap->evtdest->pid);
	
	kap->err = ESRCH;
	if ((prp != NULL) && 
		((thp = vector_lookup(&prp->threads, MTINDEX(kap->evtdest->rcvid))) != NULL))
	{
		KerextLock();
		kap->evtdest->in_progress = bool_t_TRUE;

		// Since this is a user request for an event and can handle
		// an error return, we shouldn't delve into the critical list
		// for any pulse that we need to allocate for the event - save
		// the critical heap for important pulses like hwintrs and
		// CPU exception fault delivery
		pulse_souls.flags &= ~SOUL_CRITICAL;
		kap->err = sigevent_exe(kap->event, thp, 1);
		pulse_souls.flags |= SOUL_CRITICAL;
		
		kap->evtdest->in_progress = bool_t_FALSE;
	}
}

int apm_deliver_event(evtdest_t *evtdest, struct sigevent *se)
{

	int r;
	/*
	 * in the process of sending an event, we may cause memory to be allocated
	 * (ex. growing the pulse soul list) which could trigger an event (in the
	 * case where the heap needed growing to accomodate the soul list grow).
	 * This mechanism breaks the recursion
	*/
	if (evtdest->in_progress) return EINPROGRESS;

	if (!KerextAmInKernel())
	{
		struct kerargs_deliver_event data;
		data.err = EOK;
		data.evtdest = evtdest;
		data.event = se;

		__Ring0(ker_deliver_event, &data);
		r = data.err;

#ifndef NDEBUG

		if (r != EOK) {
			if ((++evt_delivery_fail_cnt % 100) == 0) {
/*				kprintf("%u event delivery failures @ %d, err = %d\n",
						evt_delivery_fail_cnt, __LINE__, r);*/
			}
		}
#endif	/* NDEBUG */
	}
	else
	{
		/*
		 * in the case we are already in the kernel,it may be a procnto thread
		 * or a user thread that is triggering the event delivery. Because there
		 * are many places in the kernel which are not re-entrant we will not
		 * deliver the event now but instead, add it to the event queue for
		 * delivery at kernel exit. Because the 'evtdest->rcvid' relates to the
		 * connection established with 'procnto', we cannot lookup the connection
		 * and instead must know the process to deliver the event to as recorded
		 * in the 'evtdest' structure at registration time. We use this info to
		 * find a thread to deliver to.
		*/
		THREAD  *thp = NULL;
		PROCESS *prp = lookup_pid(evtdest->pid);

		r = ESRCH;
		if (prp != NULL) {
			thp = vector_lookup(&prp->threads, MTINDEX(evtdest->rcvid));
		}

		if (thp != NULL) {
			evtdest->in_progress = bool_t_TRUE;
			r = event_add(se, thp);
			evtdest->in_progress = bool_t_FALSE;
		}
#ifndef NDEBUG
		if ((thp == NULL) || (r != EOK)) {
			if ((++inker_evt_delivery_fail_cnt % 100) == 0) {
				kprintf("%u inkernel event delivery failures @ %d, err = %d\n",
						inker_evt_delivery_fail_cnt, __LINE__, r);
			}
		}
#endif	/* NDEBUG */
	}

#ifndef NDEBUG
	if (r == EOK) ++delivered_events;
	else ++undelivered_events;
#endif	/* NDEBUG */

	return r;
}
