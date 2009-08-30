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
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


    Authors: Jerome Stadelmann (JSN), Pietro Descombes (PDB), Daniel Rossier (DRE)
    Emails: <firstname.lastname@heig-vd.ch>
    Copyright (c) 2009 Reconfigurable Embedded Digital Systems (REDS) Institute from HEIG-VD, Switzerland
 */

#include "externs.h"
//
// NOTE: The order of manipulation of the queued_event_priority and
//		intrevent_pending variables in the intrevent_add() and
//		intrevent_drain() routines are _extremely_ critical. Consider
//		the case on an SMP box where one CPU is in intrevent_add() while
//		the other is in intrevent_drain(). If you're not careful, you can
//		end up with queued_event_priority > 0 while intrevent_pending == NULL.
//		That can lead to bouncing around in __ker_exit without ever getting
//		out. Alternatively, you may miss draining the queue when you should.
//

#define INTERNAL_CONFIG_FLAG_MASKED	0x8000
static volatile int 		drain_active;
static volatile int			drain_last_mask;
static INTREVENT			**intrevent_tail = &intrevent_pending;
static INTREVENT			*intrevent_free;
static unsigned				num_pev_free;
static unsigned				num_pev_alloc;
static unsigned				num_pev_trigger;
static INTREVENT			preempt_ev	= {NULL, NULL, {SIGEV_NONE}};
SMP_SPINVAR(static, intrevent_lock);


static void
intrevent_alloc(int num, int system_running) {
	int			 i;
	INTREVENT	*irp;
	INTREVENT	*head;
	INTREVENT	**tail;

	tail = &head;
	for(i = 0 ; i < num ; ++i) {
		irp = _scalloc(sizeof(*irp));
		if(irp == NULL) break;
		*tail = irp;
		tail = &irp->next;
	}

	// Want to keep interrupts disabled while we're initializing
	if(system_running) {
		INTR_LOCK(&intrevent_lock);
	}
	*tail = intrevent_free;
	intrevent_free = head;
	num_pev_free += i;
	if(system_running) {
		INTR_UNLOCK(&intrevent_lock);
	}
	num_pev_alloc += i;
	// Get more when we're down to 1/4 of the total number of entries on
	// the free list
	num_pev_trigger = num_pev_alloc / 4;
}

/*
 * We've run out of intrevent structures. This means that some process
 * has messed up its ISR. We'll find the errant process and kill it
 * without mercy.
 *
 * This code is actually bogus, since it doesn't really solve the
 * problem - the messed up ISR may not be adding any events to the
 * intrevent_pending queue, so it won't be found by scanning the
 * list. We'd need to keep track of active interrupts and find the
 * deepest nesting one that keeps on asserting when we re-enable interrupts
 * in the kernel interrupt exit processing, but I can't see any way
 * of doing that without slowing down the normal interrupt handling code,
 * which we don't want to do. It's also got race conditions - think
 * about what happens if another nested interrupt goes off while in
 * here and the code is re-entrantly started. Things to think about...
 *
 * Beginings of an idea.
 *  In intrevent_add(), when get down to a critical number of
 *  free INTREVENT's (~20), turn on a flag. In the interrupt()
 *  processing loop, if that flag is on, mask any interrupt that
 *  occurs and set a flag on the INTERRUPT structure saying that
 *  it's been masked. Eventually the problem interrupt will be masked
 *  and forward progress will be made. Once we get into intrevent_drain(),
 *  and have drained all the pending events, check the global flag. If
 *  it's on, scan the INTERRUPT structures looking for ones that have
 *  been masked by the interrupt() loop. For each, clear a state flag,
 *  unmask the level and then set the state flag. In the interrupt() loop,
 *  more code counts the number of times it gets entered. If we get above
 *  a predetermined number (~100) without seeing the state flag gets set,
 *  we assume that this is the permanently asserted interrupt and
 *  remask it. All the processes with ISR's attached to that interrupt
 *  need to be killed. Where this has problems is with SMP, since the
 *  interrupt() loop may be handled by a different CPU than the one
 *  that's doing intrevent_drain().

 */
static int
intrevent_error(THREAD *thp, INTERRUPT *isr) {
	INTREVENT	*curr_intr;
	INTREVENT	**owner;
	PROCESS		*curr_proc;
	PROCESS		*high_intrs_proc;
	pid_t		curr_pid;
	int			high_intrs_count;
	INTRLEVEL	*intr_level;
	unsigned	intr_vector;
	INTREVENT	*killer;

	/*
	 * First, run thru the pending interrupt list and 'mark' each process
	 * with the number of pending events.
	 */
	INTR_LOCK(&intrevent_lock);
	for(curr_intr = intrevent_pending; curr_intr != NULL; curr_intr = curr_intr->next) {
		curr_intr->thread->process->pending_interrupts++;
	}
	INTR_UNLOCK(&intrevent_lock);

	/*
	 * Walk the process table and find the process with the most pending
	 * interrupts. Zero the list behind us (so we don't have to do another
	 * pass later).
	 */
	high_intrs_proc = NULL;
	high_intrs_count = 0;
	for(curr_pid = 2; curr_pid < process_vector.nentries; ++curr_pid) {
		if(VECP(curr_proc, &process_vector, curr_pid)) {
			if(curr_proc->pending_interrupts > high_intrs_count) {
				high_intrs_count = curr_proc->pending_interrupts;
				high_intrs_proc = curr_proc;
			}
			curr_proc->pending_interrupts = 0;
		}
	}

	intr_level = &interrupt_level[isr->level];
	intr_vector = intr_level->info->vector_base + isr->level - intr_level->level_base;
#define MIN_INTRS_BAD 10
	if(high_intrs_count < MIN_INTRS_BAD) {
		/* There wasn't enough pending interrupts on any particular process to justify
		 * canceling them.
		 */
		char	*name = thp->process->debug_name;

		if(name == NULL) name = "";
		kprintf("Out of interrupt events! (vector=%u process=%u [%s])\n",
				intr_vector, thp->process->pid, name);

		if(ker_verbose >= 2) {
		/* debug assist information when of out interrupt events occurs */
			unsigned  i;
			INTREVENT * irplocal = intrevent_pending;
#if defined(__GNUC__)
/* this enum order safe init is not supported by the WATCOM compiler */
	#define ARRAY_EL(e)	[(e)] =
#else
	#define ARRAY_EL(e)
#endif
			static const char * const fmt[] =
			{
				ARRAY_EL(SIGEV_NONE) "t-%02u:  SIGEV_NONE(%u) -> %u (%s)  (--/--/--/--)\n",
				ARRAY_EL(SIGEV_SIGNAL) "t-%02u:  SIGEV_SIGNAL(%u) -> %u (%s)  (0x%x/0x%x/--/--)\n",
				ARRAY_EL(SIGEV_SIGNAL_CODE) "t-%02u:  SIGEV_SIGNAL_CODE(%u) -> %u (%s)  (0x%x/0x%x/0x%x/--)\n",
				ARRAY_EL(SIGEV_SIGNAL_THREAD) "t-%02u:  SIGEV_SIGNAL_THREAD(%u) -> %u (%s)  (0x%x/0x%x/0x%x/--)\n",
				ARRAY_EL(SIGEV_PULSE) "t-%02u:  SIGEV_PULSE(%u) -> %u (%s)  (0x%x/0x%x/0x%x/0x%x)\n",
				ARRAY_EL(SIGEV_UNBLOCK) "t-%02u:  SIGEV_UNBLOCK(%u) -> %u (%s)  (--/--/--/--)\n",
				ARRAY_EL(SIGEV_INTR) "t-%02u:  SIGEV_INTR(%u) -> %u (%s)  (--/--/--/--)\n",
				ARRAY_EL(SIGEV_THREAD) "t-%02u:  SIGEV_THREAD(%u) -> %u (%s)  (--/--/--/--)\n",
			};
			kprintf("Last %u:   event  ->   pid  (signo/val/code/pri)\n", 2 * MIN_INTRS_BAD);
			for (i=0; i<(2 * MIN_INTRS_BAD); i++, irplocal=irplocal->next)
				kprintf(fmt[SIGEV_GET_TYPE(&irplocal->event) % NUM_ELTS(fmt)],
							i+1, SIGEV_GET_TYPE(&irplocal->event),
							(irplocal->thread && irplocal->thread->process) ? irplocal->thread->process->pid : 0,
							(irplocal->thread && irplocal->thread->process) ? irplocal->thread->process->debug_name : "?",
							irplocal->event.sigev_signo, irplocal->event.sigev_value.sival_int,
							irplocal->event.sigev_code, irplocal->event.sigev_priority);
		}
		return 0;
	}


	/*
	 * Cancel all interrupts pending for that process.
	 */
	killer = NULL;
	INTR_LOCK(&intrevent_lock);
	owner = &intrevent_pending;
	for( ;; ) {
		curr_intr = *owner;
		if(curr_intr == NULL) break;
		if(curr_intr->thread->process == high_intrs_proc) {
			*owner = curr_intr->next;
			if(intrevent_tail == &curr_intr->next) {
				intrevent_tail = owner;
			}
			if(killer == NULL) {
				killer = curr_intr;
			} else {
				curr_intr->next = intrevent_free;
				intrevent_free = curr_intr;
				++num_pev_free;
			}
		} else {
			owner = &curr_intr->next;
		}
	}

	if((killer == NULL) || (high_intrs_proc == NULL)) crash();
	killer->thread = high_intrs_proc->valid_thp;
	// Use a uniqe code that people can recognize
	SIGEV_SIGNAL_CODE_INIT(&killer->event, SIGKILL, 1, SI_IRQ);
	killer->next = intrevent_pending;
	intrevent_pending = killer;

	INTR_UNLOCK(&intrevent_lock);

	if(high_intrs_proc == thp->process) {
		if(intr_vector != SYSPAGE_ENTRY(qtime)->intr) {
			// The current interrupt came from the failed process. Mask it.
			//NYI: There should be a tracevent for this....
			(void) interrupt_mask(isr->level, isr);
		}
	}

	// Tell intrevent_add() to try again, we've freed stuff up.
	return 1;
}


void
intrevent_preemption(unsigned prio) {
	if(!nopreempt) {
		INTR_LOCK(&intrevent_lock);
		if(queued_event_priority < prio) {
			queued_event_priority = prio;
			// If there isn't anything queued right now, stick a dummy
			// entry on so that we'll go into intrevent_drain() and
			// reset queued_event_priority back to zero
			if(intrevent_pending == NULL) {
				preempt_ev.next = NULL;
				*intrevent_tail = &preempt_ev;
				intrevent_tail = &preempt_ev.next;
			}
		}
		INTR_UNLOCK(&intrevent_lock);
	}
}


//
// This routine is called from an interrupt handler.
// If it is safe, we do the work immediately.
// Otherwise we queue the event which will be acted
// on at the first safe moment. Probably when we are
// about to leave the microkernel.
//
// The low bit of *thp may be set to indicate intrevent_add was called from an IO interrupt. (Yes, it's ugly.)
// use kermacros.c:  AP_INTREVENT_FROM_IO(thp), which sets the low bit, to pass the thp parm, if you're calling
// intrevent_add from IO.
int intrevent_add_attr
intrevent_add(const struct sigevent *evp, THREAD *thp, INTERRUPT *isr) {
	INTREVENT		*irp;
	int		 		prio;
	struct sigevent	ev_copy;

	//make events sent from IO interrupt critical, but dont clobber the user's sigevent
	if(((uintptr_t)(thp)) & AP_INTREVENT_FROM_IO_FLAG) {
		thp = (THREAD*) ((uintptr_t)(thp) & ~AP_INTREVENT_FROM_IO_FLAG);
		if(intrs_aps_critical) {
			ev_copy = *evp;
			SIGEV_MAKE_CRITICAL(&ev_copy);
			evp = &ev_copy;
		}
	};

#if !defined(VARIANT_smp)	/* PDB: condition is true */
	//
	// If we were in user mode before, we can deliver the event right away.
	// We can't, however, allocate any memory. We might need to allocate
	// up to two pulses - one for the actual event being delivered, and
	// one for an UNBLOCK pulse to be delivered to the server that the first
	// thread is replied blocked on. We check for 4 available entries as
	// an extra security blanket.
	//
	if((get_inkernel() == 1) && ((pulse_souls.total - pulse_souls.used) >= 4)) {
		int				status;

		// Probably safe to handle it right away (SIGEV_THREAD is a problem).
		if((status = sigevent_exe(evp, thp, 1)) >= 0) {
			return(status);
		}
	}
#endif

	for( ;; ) {
		INTR_LOCK(&intrevent_lock);

		// get a free event from the list and fill it in
		irp = intrevent_free;
		if(irp != NULL) break;
		if(ker_verbose >= 2) {
			DebugKDBreak();
		}
		INTR_UNLOCK(&intrevent_lock);
		if(!intrevent_error(thp, isr)) {
			return(-1);
		}
	}
	intrevent_free = irp->next;

	irp->thread = thp;
	irp->event = *evp;

	// Keep track of the maximum queued event priority for pre-emption.
	if(--num_pev_free <= num_pev_trigger) {
		if(drain_active) {
			int		level =	isr->level;

			// We're trying to drain the queue, don't let this interrupt
			// happen again until we're finished
			INTR_UNLOCK(&intrevent_lock);
			(void) interrupt_mask(level, NULL);
			INTR_LOCK(&intrevent_lock);
			interrupt_level[level].config |= INTERNAL_CONFIG_FLAG_MASKED;
			if(drain_last_mask < level) drain_last_mask = level;
		}
		// If we are running low on free pending event structures, try to
		// preempt sooner to drain the queue....
		prio = NUM_PRI;
	} else if(nopreempt) {
		prio = 0;
	} else if(SIGEV_GET_TYPE(evp) == SIGEV_PULSE && evp->sigev_priority >= 0) {
		prio = evp->sigev_priority;
	} else {
		prio = thp->priority;
	}

	if(queued_event_priority < prio) {
		queued_event_priority = prio;
	}

    // now put this event at the end of pending list
	irp->next = NULL;
	*intrevent_tail = irp;
	intrevent_tail = &irp->next;
    INTR_UNLOCK(&intrevent_lock);

	return(0);
}

/*
 * event_add
 *
 * this routine is like intrevent_add() in that it is intended to queue events
 * that cannot be delivered immediately onto the 'intrevent_pending' queue for
 * delivery at kernel exit. Difference is that we will only queue the event if
 * there is room.
 *
 * Returns: EOK if the event was queued
 * 			ECANCELED is the event could not be queued
*/
int
event_add(struct sigevent *evp, THREAD *thp) {
	INTREVENT	*irp;
	int 		r;

	INTR_LOCK(&intrevent_lock);

	// get a free event from the head of the list and fill it in if
	// we have space.
	if(num_pev_free > (num_pev_trigger+1)) {
		int prio;

		irp = intrevent_free;
		intrevent_free = irp->next;
		--num_pev_free;

		irp->thread = thp;
		irp->event = *evp;

		if(nopreempt) {
			prio = 0;
		} else if(SIGEV_GET_TYPE(evp) == SIGEV_PULSE && evp->sigev_priority >= 0) {
			prio = evp->sigev_priority;
		} else {
			prio = thp->priority;
		}

		if(queued_event_priority < prio) {
			queued_event_priority = prio;
		}

		// now put this event at the end of pending list
		irp->next = NULL;
		*intrevent_tail = irp;
		intrevent_tail = &irp->next;
		r = EOK;
	} else {
		r = ECANCELED;
	}

	INTR_UNLOCK(&intrevent_lock);
	return r;
}

void intrevent_drain_attr
intrevent_drain(void) {
	INTREVENT		*irp;
	INTREVENT		*next;
	int				need_more = 0;
	int				level;
	THREAD			*thp;
	uint64_t		id_hook_context;

	if (intrevent_drain_hook_enter) {
		id_hook_context = intrevent_drain_hook_enter();
	}

	drain_last_mask = -1;
	drain_active = 1;

	for( ;; ) {
		INTR_LOCK(&intrevent_lock);
		// Get the list of pending events so far....
		irp = intrevent_pending;
		intrevent_pending = NULL;
		intrevent_tail = &intrevent_pending;
		if(num_pev_free <= num_pev_trigger) {
			need_more = 1;
		}
		queued_event_priority = 0;
		if(irp == &preempt_ev) {
			// We don't need to process the preempt_ev entry, nor
			// do we want to put it on the intrevent free list.
			// It's always going to be at the start of the pending list
			// if it's present at all, so we only need to check for it here.
			irp = irp->next;
		}
		INTR_UNLOCK(&intrevent_lock);

		if(irp == NULL) break;

		do {
			unsigned				id;

			thp = irp->thread;
			id = irp->event.sigev_notify & 0xffff0000;
			if(id != 0) {
				// Try and act on the event associated with this timer.
				overrun = 0;
				sigevent_exe(&irp->event, thp, 1);
				if(overrun) {
					TIMER	*tip;

					if((tip = vector_lookup(&thp->process->timers, (id >> 16) - 1))) {
						if(tip->overruns < DELAYTIMER_MAX - 1) {
							++tip->overruns;
						}
					}
				}
			} else {
				sigevent_exe(&irp->event, thp, 1);
			}

			next = irp->next;

			// Add the irp back to the free list.
			// We do it after processing each entry through rather than one
			// big go after we've done the whole list so that we make the
			// entries available to intrevent_add() as soon as possible.
			// More interrupts might be coming in while we're draining.
			INTR_LOCK(&intrevent_lock);
			irp->next = intrevent_free;
			intrevent_free = irp;
			++num_pev_free;
			INTR_UNLOCK(&intrevent_lock);

			irp = next;
		} while(irp != NULL);
	}

	// Nothing left in the queue.

	drain_active = 0;

	// If we dropped below our trigger point for free entries, get some more
	if(need_more) {
		if(ker_verbose >= 2) {
			kprintf("Grow intrevents by 25%%\n");
		}
		intrevent_alloc(num_pev_alloc/4, 1);
	}

	// intrevent_add() [might have] turned off some interrupts while we were
	// draining the queue. Look through the list and unmask them now that
	// we're done.
	for(level = drain_last_mask; level >= 0; --level) {
		if(interrupt_level[level].config & INTERNAL_CONFIG_FLAG_MASKED) {
			interrupt_level[level].config &= ~INTERNAL_CONFIG_FLAG_MASKED;
			(void) interrupt_unmask(level, NULL);
		}
	}

	if (intrevent_drain_hook_exit) {
		intrevent_drain_hook_exit(id_hook_context);
	}
}


void
intrevent_flush(void) {
#ifdef VARIANT_smp
#ifndef NDEBUG
	unsigned	start = (unsigned)qtimeptr->nsec;
#endif
	// Wait for all processors to get out of the interrupt handling
	// code since we're about to destroy the something that the
	// code might reference.
	for( ;; ) {
		// Make sure all the reads & writes are globally visible,
		// also stay off the bus to give other processors a chance
		// at it.
		MEM_BARRIER_RW();
		if((get_inkernel() & INKERNEL_INTRMASK) == 0) break;
		// If we're running low on free intrevents, drain the queue
		if(num_pev_free <= num_pev_trigger) {
			intrevent_drain();
		}
#ifndef NDEBUG
		if(((unsigned)qtimeptr->nsec - start) > 1000000000) {
			// if we spent more than 1 second in the loop without
			// finishing all the interrupt handlers, something's gone wrong
			crash();
		}
#endif
	}
#endif
	if(intrevent_pending != NULL) {
		intrevent_drain();
	}
}


void
intrevent_init(unsigned num) {
	if(num == 0) num = 200; // Set default
	intrevent_alloc(num, 0);
}


//
// This routine will call the low level primitive to execute a
// sigevent. The src may be passed directly from an interrupt handler or
// from an event which was queued.
//
int rdecl
sigevent_exe(const struct sigevent *evp, THREAD *thp, int deliver) {
	CONNECT	*cop;
	CHANNEL	*chp;
	PROCESS	*prp;
	int		status;
	int		prio;

	switch(SIGEV_GET_TYPE(evp)) {
	case SIGEV_NONE:
		if(thp == procnto_prp->threads.vector[0]) {
			if(!(get_inkernel() & INKERNEL_NOW)) {
				/*
					If we're being invoked by an interrupt handler, we
					must queue the event - timer_pending() needs to be
					able to free memory.
				*/
				return -1;
			}
			// If the thread that signaled the event is the idle
			// thread, that was an indicator that we have pending
			// timer operations that need to be dealt with.
			timer_pending(NULL);
		}
		break;

	case SIGEV_SIGNAL_THREAD:
		if(evp->sigev_signo <= 0  ||  evp->sigev_signo > _SIGMAX  || evp->sigev_code > SI_USER) {
			return(EINVAL);
		}
		if(deliver) {
			prp = thp->process;
			(void)signal_kill_thread(prp,
					thp,
					evp->sigev_signo,
					evp->sigev_code,
					evp->sigev_value.sival_int,
					prp->pid,
					evp->sigev_notify & SIGEV_FLAG_CRITICAL ? SIGNAL_KILL_APS_CRITICAL_FLAG : 0
					);
		}
		break;

	case SIGEV_SIGNAL:
		if(evp->sigev_signo <= 0  ||  evp->sigev_signo > _SIGMAX) {
			return(EINVAL);
		}
		if(deliver) {
			prp = thp->process;
			signal_kill_process(prp,
					evp->sigev_signo,
					SI_USER,
					evp->sigev_value.sival_int,
					prp->pid,
					evp->sigev_notify & SIGEV_FLAG_CRITICAL ? SIGNAL_KILL_APS_CRITICAL_FLAG : 0
					);
		}
		break;

	case SIGEV_SIGNAL_CODE:
		if(evp->sigev_signo <= 0  ||  evp->sigev_signo > _SIGMAX  || evp->sigev_code > SI_USER) {
			return(EINVAL);
		}
		if(deliver) {
			prp = thp->process;
			signal_kill_process(prp,
					evp->sigev_signo,
					evp->sigev_code,
					evp->sigev_value.sival_int,
					prp->pid,
					evp->sigev_notify & SIGEV_FLAG_CRITICAL ? SIGNAL_KILL_APS_CRITICAL_FLAG : 0
					);
		}
		break;

	case SIGEV_THREAD:
		if(thp->process->flags & _NTO_PF_DESTROYALL) {
			return EBUSY;
		}
		if(!(get_inkernel() & INKERNEL_NOW)) {
			/*
				If we're being invoked by an interrupt handler, we
				must queue the event - thread_create() might need to
				allocate memory.
			*/
			return -1;
		}
		if(!deliver || (status = thread_create(thp, thp->process, evp,
			evp->sigev_notify & SIGEV_FLAG_CRITICAL ? THREAD_CREATE_APS_CRITICAL_FLAG : 0 )) == ENOERROR) {
			status = EOK;
		}
		return status;

	case SIGEV_PULSE:
		{
		VECTOR		*vec;
		unsigned	coid = evp->sigev_coid;

		prp = thp->process;
		vec = &prp->fdcons;
		if(coid & _NTO_SIDE_CHANNEL) {
			coid &= ~_NTO_SIDE_CHANNEL;
			vec = &prp->chancons;
		}

		if(vec && (coid < vec->nentries) && (VECAND(cop = VEC(vec, coid), 1) == 0)) {
			cop = VECAND(cop, ~3);
		} else {
			cop = NULL;
		}

		if(cop == NULL  ||  cop->type != TYPE_CONNECTION) {
			return(ESRCH);
		}

		if((chp = cop->channel) == NULL) {
			return(EBADF);
		}

		if(deliver) {
			//A -1 pulse priority means use the priority of the *process* of the receiver
			prio = (evp->sigev_priority == -1) ? chp->process->process_priority : evp->sigev_priority & _PULSE_PRIO_PUBLIC_MASK;
			_TRACE_COMM_EMIT_SPULSE_EXE(cop, cop->scoid | _NTO_SIDE_CHANNEL, prio);
			if ((status = pulse_deliver(chp, prio, evp->sigev_code, evp->sigev_value.sival_int,
				cop->scoid | _NTO_SIDE_CHANNEL,
				(evp->sigev_notify&SIGEV_FLAG_CRITICAL ? PULSE_DELIVER_APS_CRITICAL_FLAG : 0) ))) {
				return(status);
			}
		}
		break;
	}

	case SIGEV_UNBLOCK:
		if(deliver) {
			// a critical unblock event seems unlikely, but wth.
			if (evp->sigev_notify & SIGEV_FLAG_CRITICAL) AP_MARK_THREAD_CRITICAL(thp);

			if(thp->state == STATE_NANOSLEEP) {
				ready(thp);
			} else {
				force_ready(thp, ETIMEDOUT);
			}
		}
		break;

	case SIGEV_INTR:
		if(deliver) {
			if(thp->state == STATE_INTR) {
				if (evp->sigev_notify & SIGEV_FLAG_CRITICAL) AP_MARK_THREAD_CRITICAL(thp);
				ready(thp);
			} else {
				thp->flags |= _NTO_TF_INTR_PENDING;
			}
		}
		break;

	default:
		if(deliver && ker_verbose > 1) {
			kprintf("Pid %d: Invalid event queued (%d)\n", thp->process->pid, evp->sigev_notify);
		}
		return(EINVAL);
	}


	return(EOK);
}

//
// This routine is used by kernel routines to enqueue a sigevent to
// the process manager. It assumes the first thread is the idle thread
// and will always exist.
//
int rdecl
sigevent_proc(const struct sigevent *evp) {
	if(evp) {
		CRASHCHECK(SIGEV_GET_TYPE(evp) == SIGEV_PULSE
						&& evp->sigev_priority == -1);
		return sigevent_exe(evp, sysmgr_prp->threads.vector[0], 1);
	}

	return -1;
}

__SRCVERSION("nano_event.c $Rev: 204178 $");
