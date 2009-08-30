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
#include "mt_kertrace.h"

/*********************************************************************
  Notes about the interlocking in this file.

  The active timer list(s) can be examined/manipulated by two
  environments - the interrupt subsystem (via timer_expiry()) and
  normal kernel call operations. To prevent the two from mucking with
  each other, the code called from normal kernel operations surround
  themselves with KEROP_START()/KEROP_STOP() macros. This
  sets the timers_kerop variable. When code invoked from the
  interrupt subsystem wants to modify the active timer list(s) linkages
  (basically removing the timer from the list after firing and possibly
  re-adding it for an interval timer), it first checks the timers_kerop
  variable and, if set, does not touch the linkages. Instead it puts
  the timer on the pending list and causes timer_expiry() to do
  an intrevent_add() that ultimately causes the timer_pending() function
  to be invoked as we're leaving the kernel. The timer_pending() function
  then goes and performs the delayed timer operations.

  On an SMP system, the kernel invoked code also has to wait for the
  timer_expiry() function to exit before continuing on (done via the
  TICKER_WAIT() macro, invoked by KEROP_START()). This is so we can be
  sure that the interrupt code sees timers_kerop is set, since the kernel
  operation and interrupt handling code might be running on different CPU's
  at the same time.

*********************************************************************/

enum fire_source {
	FS_INTR,
	FS_ACTIVATE,
	FS_PENDING,
};

#define TQ_FLAG_TOD			0x0001
#define TQ_FLAG_OLD			0x0002

struct timer_queue {
	struct timer_queue		*next;
	unsigned				downshift;
	unsigned				mask;
	unsigned				num_active;
	unsigned				flags;
	unsigned				fire_idx;
	unsigned				last_tick_idx;
	struct timer_link		slot[1]; // variable sized
};

// Can't use NULL to mark the end of the list, because we want to be able
// to tell timers are pending by checking tip->pending for non-NULL.
#define PENDING_TIMER_END	((TIMER *)0x1)

static volatile uint8_t		timer_ops_delayed;
static volatile uint8_t		timers_kerop;
static TIMER				*pending_head = PENDING_TIMER_END;
static struct timer_queue	*queue_head;
static struct timer_queue	*queue_dead;
static struct timer_queue	*mon_timers;	// Ptr to active timers (relative).
static struct timer_queue	*tod_timers;	// Ptr to active timers (absolute).

// We can get away with only downshifting the low order 32 bits of
// the 64-bit nanosecond value because the downshift and mask bits will
// never select anything in the upper 32 bits.
#define GET_QUEUE_IDX(q, t)		(((((uint32_t)(t)) >> (q)->downshift)) & (q)->mask)

#define ADD_IN_NEW_QUEUE(head, old, new)	\
		do {								\
			(new)->flags = (old)->flags;	\
			(new)->next = (old)->next;		\
			(old)->next = (new);			\
			MEM_BARRIER_WR();				\
			*(head) = (new);				\
			(old)->flags |= TQ_FLAG_OLD;	\
		} while(0)

// Consistency check that the forward and backward links are sane
#define CHECK_LINKAGES(tip)	\
		CRASHCHECK((tip)->link.prev->link.next != (tip));	\
		CRASHCHECK((tip)->link.next->link.prev != (tip));	\

#define MAX_SEARCH		100
#define MAX_MASK		(128-1)


#if defined(VARIANT_smp)
	static volatile uint8_t		timers_ticking;

	#define TICKER_START()	(timers_ticking = 1, MEM_BARRIER_RW())
	#define TICKER_STOP()	MEM_BARRIER_WR(); (timers_ticking = 0)
	#define TICKER_WAIT()	do { MEM_BARRIER_RW(); } while(timers_ticking)
#else
	#define TICKER_START()
	#define TICKER_STOP()
	#define TICKER_WAIT()
#endif

#define KEROP_START()	(timers_kerop = 1); TICKER_WAIT()
#define KEROP_STOP()	MEM_BARRIER_WR(); (timers_kerop = 0)



static struct timer_queue *
new_queue(unsigned entries, unsigned downshift) {
	struct timer_queue	*new;
	struct timer_link	*link;

	new = _scalloc(sizeof(*new) + (entries-1)*sizeof(new->slot[0]));
	if(new != NULL) {
		new->mask = entries - 1;
		new->downshift = downshift;
		do {
			link = &new->slot[--entries];
			link->prev = link->next = (TIMER *)link;
		} while(entries != 0);
	}
	return new;
}


static void
kill_queue(struct timer_queue *queue) {
	CRASHCHECK(queue == mon_timers || queue == tod_timers);
	_sfree(queue, sizeof(*queue) + (queue->mask)*sizeof(queue->slot[0]));
}


static void
timer_insert(TIMER *tip, enum fire_source src) {
	TIMER				*fore;
	TIMER				*back;
	struct timer_queue	**head;
	struct timer_queue	*queue;
	struct timer_queue	*new;
	unsigned			max_search;
	unsigned			idx;

	if(tip->flags & _NTO_TI_TOD_BASED) {
		head = &tod_timers;
	} else {
		head = &mon_timers;
	}
	queue = *head;

retry:
	tip->queue = queue;

	if(src == FS_INTR) {
		// If we're being called from within the clock isr, we can't
		// grow the queue - set max_search such that it never triggers.
		max_search = 0;
	} else if(queue->num_active >= (2*MAX_SEARCH)*(queue->mask+1)) {
		// If we've got a lot of entries in the queue, go ahead
		// and grow it even if we don't hit one long list - rearming
		// timers might move things around.
		max_search = 1;
	} else {
		max_search = MAX_SEARCH;
	}
	idx = GET_QUEUE_IDX(queue, tip->itime.nsec);
	back = fore = (TIMER *)&queue->slot[idx];
	if(fore->link.next != back) {
		// Not an empty list - search backwards and forwards
		for( ;; ) {
			CHECK_LINKAGES(fore);
			CHECK_LINKAGES(back);
			fore = fore->link.next;
			back = back->link.prev;
			if((--max_search == 0) && (queue->mask < MAX_MASK)) {
				new = new_queue((queue->mask+1) << 1, queue->downshift);
				if(new != NULL) {
					ADD_IN_NEW_QUEUE(head, queue, new);
					queue = new;
					goto retry;
				}
			}
			if(tip->itime.nsec >= back->itime.nsec) goto insert_after;
			if(tip->itime.nsec <  fore->itime.nsec) goto insert_before;
		}
	}

	// Link the timer in, careful about order - timer_expiry could be running

insert_after:
	// insert after the test point
	tip->link.next = back->link.next;
	tip->link.prev = back;
	back->link.next->link.prev = tip;
	back->link.next = tip;
	CHECK_LINKAGES(tip);
	CHECK_LINKAGES(back);
	queue->num_active += 1;
	return;

insert_before:
	// insert before the test point
	tip->link.prev = fore->link.prev;
	tip->link.next = fore;
	fore->link.prev->link.next = tip;
	fore->link.prev = tip;
	CHECK_LINKAGES(tip);
	CHECK_LINKAGES(fore);
	queue->num_active += 1;
	return;
}


static void
timer_rearm(TIMER *tip, enum fire_source src) {
	uint64_t	interval;
	QTIME		*qtp;

	CRASHCHECK(!(tip->flags & _NTO_TI_ACTIVE));
	if(src != FS_ACTIVATE) {
		// Careful about how things are pulled from the active
		// list - timer_pending & timer_expiry might be running at the
		// same time.
		tip->link.next->link.prev = tip->link.prev;
		tip->link.prev->link.next = tip->link.next;
		CHECK_LINKAGES(tip->link.prev);
		CHECK_LINKAGES(tip->link.next);
		tip->queue->num_active -= 1;
	}
	// Once we're off the active list, we can clear the pending indicator
	// so that (if we rearm) the timer_fire() code will know that it can
	// actually do something with this timer.
#if defined(VARIANT_smp)
	// On an SMP system, we need to wait until timer_expiry() has completed
	// before setting tip->pending to NULL so that it doesn't get confused
	// and re-fire the timer (it might have managed to pick up a pointer to
	// the timer up before we removed it from the active timer list)
	if(src != FS_INTR) {
		TICKER_WAIT();
	}
#endif
	tip->pending = NULL;

	if(tip->itime.interval_nsec == 0) {
		// Timer expired normally
		tip->flags = (tip->flags & ~_NTO_TI_ACTIVE) | _NTO_TI_EXPIRED;
		tip->queue = NULL;
		return;
	}

	// An interval timer, so we must reactivate it with a new time.

	qtp = qtimeptr;
	// The following check prevents infinite loops.
	interval = tip->itime.interval_nsec;
	if(interval < qtp->nsec_inc) {
		interval = qtp->nsec_inc;
	}

	// Recalculate the trigger time.
	tip->itime.nsec += interval;

	/*
		FIX PR-8735
		if we're CLOCK_SOFTTIME, and after recalculating the trigger
		time we're still less then current tod, we need to resync
		ourselves
	*/
	if(tip->clockid == CLOCK_SOFTTIME) {
		uint64_t		tod;

		snap_time(&tod, tip->flags & _NTO_TI_TOD_BASED);
		if(tip->itime.nsec < tod) {
			tip->itime.nsec = tod + interval;
		}
	}
	timer_insert(tip, src);
}


static TIMER *
timer_fire(TIMER *tip) {
	THREAD		*thp;
	TIMER		*next;

	next = tip->link.next;
	CRASHCHECK(next == NULL);
	if((tip->pending != NULL) || (tip->queue == NULL)) {
		// Already fired or being deleted
		return next;
	}
	CRASHCHECK(!(tip->flags & _NTO_TI_ACTIVE));
	CRASHCHECK(tip->link.prev == NULL);

	if(timers_kerop) {
		// We're being invoked from an interrupt handler and
		// the kernel is inspecting/manipulating the active timer
		// queue, so we can't manipulate the linkages right now.
		// Just put the timer on the pending list and we'll take
		// care of re-arming the timer in timer_pending().
		INTR_LOCK(&clock_slock);
		tip->pending = pending_head;
		pending_head = tip;
		INTR_UNLOCK(&clock_slock);
	} else {
		CHECK_LINKAGES(tip);
	}

	thp = tip->thread;

	// Try and act on the event associated with this timer.
	// Note: In the case of a timeout this may release tip
	if(thp != NULL) {
		overrun = 0;
		intrevent_add(&tip->event, thp, clock_isr);
		mt_TRACE_DEBUG("timer_fire back");
		if(overrun && (tip->overruns < DELAYTIMER_MAX - 1)) {
			mt_TRACE_DEBUG("timer_fire overrun");
			++tip->overruns;
		}
	} else {
		// thread has gone away, deactivate the timer
		tip->itime.interval_nsec = 0;
	}

	// We have to check for tip->queue != NULL again, even though
	// we did it at the top of the function, because we might
	// have done a sigevent_exe() on a TimerTimeout timer and the
	// code would have called timer_deactivate() after delivering the
	// event.
	if((tip->pending == NULL) && (tip->queue != NULL)) {
		TIMER *prev = tip->link.prev;

		timer_rearm(tip, FS_INTR);
		if(prev->link.next == tip) {
			// It's been re-armed in front of the next entry, so we have
			// to check it again.
			next = tip;
		}
	}
	return next;
}


void rdecl
timer_expiry(QTIME *qtp) {
	uint64_t				tod = qtp->nsec;
	struct timer_queue		*queue;
	struct timer_queue		*done;
	struct timer_queue		**owner;
	TIMER					*tip;
	TIMER					*first;
	unsigned				nfires;
	unsigned				curr_tick_idx;
	unsigned				count;
	int						adjusted = 0;

	nfires = 0;
	TICKER_START();
	owner = &queue_head;
	for( ;; ) {
		queue = *owner;
		if(queue == NULL) break;
		if(!adjusted && (queue->flags & TQ_FLAG_TOD)) {
			adjusted = 1;
			tod += qtp->nsec_tod_adjust;
		}
		curr_tick_idx = GET_QUEUE_IDX(queue, tod);
		if(queue->num_active != 0) {
			owner = &queue->next;
			count = (queue->last_tick_idx - queue->fire_idx) & queue->mask;
			count += (curr_tick_idx - queue->last_tick_idx) & queue->mask;
			if(count > queue->mask) {
				// We've wrapped around, have to examine all the slots
				queue->fire_idx = (curr_tick_idx + 1) & queue->mask;
			}
			for( ;; ) {
				first = (TIMER *)&queue->slot[queue->fire_idx];
				tip = first->link.next;
				for( ;; ) {
					if(tip == first) break;
					CRASHCHECK((tip->queue != queue) && (tip->queue != NULL));
					if(tip->itime.nsec > tod) break;

					tip = timer_fire(tip);

					//
					// - Only fire up to 50 timers in one interrupt to minimize
					//   the latency in processing interrupt events.
					// - If we're running low on interrupt events, kick out of
					//   here so we get a chance to drain the queue.

					if((++nfires > 50) || (queued_event_priority >= NUM_PRI)) {
						// Setting 'owner' to 'done' will cause the
						// main loop to terminate the next time we get to
						// the top of it.
						if (timer_expiry_hook_max_timer_fires) {
							timer_expiry_hook_max_timer_fires(nfires);
						}
						done = NULL;
						owner = &done;
						break;
					}
				}
				if(queue->fire_idx == curr_tick_idx) break;
				queue->fire_idx = (queue->fire_idx + 1) & queue->mask;
			}
		} else if((queue->flags & TQ_FLAG_OLD) && !timers_kerop) {
			*owner = queue->next;
			queue->next = queue_dead;
			queue_dead = queue;
		} else {
			owner = &queue->next;
			queue->fire_idx = curr_tick_idx;
		}
		queue->last_tick_idx = curr_tick_idx;
	}
	TICKER_STOP();
	if(!timer_ops_delayed &&
		((pending_head != PENDING_TIMER_END) || (queue_dead != NULL))) {
		struct sigevent	none;

		// Queue an event to get the pending timer items dealt with.
		// When things are in a safe state, we'll start executing
		// the timer_pending() routine below.
		timer_ops_delayed = 1;
		SIGEV_NONE_INIT(&none);
		(void)intrevent_add(&none, procnto_prp->threads.vector[0], clock_isr);
	}
}


void rdecl
timer_pending(TIMER *skip) {
	TIMER				*tip;
	struct timer_queue	*queue;
	struct timer_queue	*next;

	//Mark pending items as processed
	timer_ops_delayed = 0;

	// process all the pending timer re-arms
	INTR_LOCK(&clock_slock);
	tip = pending_head;
	pending_head = PENDING_TIMER_END;
	INTR_UNLOCK(&clock_slock);

	KEROP_START();

	while(tip != PENDING_TIMER_END) {
		TIMER	*pending;

		pending = tip->pending;
		if(tip == skip) {
			// we're deactivating this timer, don't
			// try to rearm (tip->queue is NULL)
			tip->pending = NULL;
		} else {
			timer_rearm(tip, FS_PENDING);
		}
		tip = pending;
	}

	// free up all the timer_queues that aren't needed anymore
	queue = queue_dead;
	queue_dead = NULL;
	KEROP_STOP();

	while(queue != NULL) {
		next = queue->next;
		kill_queue(queue);
		queue = next;
	}
}


void rdecl
timer_init(void) {
	tod_timers = new_queue(4, 0);
	tod_timers->flags = TQ_FLAG_TOD;
	mon_timers = new_queue(4, 0);
	mon_timers->next = tod_timers;
	queue_head = mon_timers;
}


static void
new_period(struct timer_queue **head, unsigned downshift) {
	struct timer_queue	*new;
	struct timer_queue	*old;

	old = *head;
	if(old->downshift != downshift) {
		if(old->num_active == 0) {
			// nobody using it, just update entry
			old->downshift = downshift;
		} else {
			new = new_queue(old->mask + 1, downshift);
			if(new != NULL) {
				ADD_IN_NEW_QUEUE(head, old, new);
			}
		}
	}
}


void rdecl
timer_period(void) {
	unsigned long	incr;
	unsigned 		downshift;

	// Clock resolution has been updated, need to change the
	// downshift value in the timer queues. We want to choose
	// a value such that the difference between the start
	// and end time that goes into each slot is approximately
	// one tick's worth. That way the timer_expiry() code will
	// usually need to examine only 1 or 2 slots worth of data.

	downshift = 0;
	incr = SYSPAGE_ENTRY(qtime)->nsec_inc;
	do {
		++downshift;
		incr >>= 1;
	} while(incr != 0);

	KEROP_START();
	new_period(&mon_timers, downshift);
	new_period(&tod_timers, downshift);
	KEROP_STOP();
}


void rdecl
timer_remaining(TIMER *tip, struct _itimer *left) {
	uint64_t		curtime;

	if(!(tip->flags & _NTO_TI_ACTIVE)) {
		left->nsec = left->interval_nsec = 0;
		return;
	}

	left->interval_nsec = tip->itime.interval_nsec;

	snap_time(&curtime, tip->flags & _NTO_TI_TOD_BASED);
	left->nsec = tip->itime.nsec - curtime;

	// Don't return negative time. Limit it to zero.
	if((int64_t)left->nsec < 0) {
		left->nsec = 0;
	}
}


//
// Return 1 if the timers time has already past.
// A relative wait of 0 is considered past.
//
int rdecl
timer_past(int clockid, int flags, uint64_t *firetime) {
	uint64_t	curtime;

	if(flags & TIMER_ABSTIME) {
		snap_time(&curtime, (clockid == CLOCK_MONOTONIC) ? 0 : 1);
		if(*firetime < curtime) {
			return(1);
		}
	} else if(*firetime == 0) {
		return(1);
	}

	return(0);
}


void rdecl
timer_activate(TIMER *tip) {
	uint64_t	tod;

	KEROP_START();

	CRASHCHECK(tip->queue != NULL);
	tip->flags |= _NTO_TI_ACTIVE;

	if(tip->flags & _NTO_TI_ABSOLUTE) {
		SNAP_TIME_INLINE(tod, tip->flags & _NTO_TI_TOD_BASED);
		if(tip->itime.nsec <= tod) {
			// We don't have to check for overruns because we
			// know that intrevent_add() will queue the event (since
			// we're invoking it while we're in the kernel).
			(void)intrevent_add(&tip->event, tip->thread, clock_isr);
			timer_rearm(tip, FS_ACTIVATE);
		} else {
			timer_insert(tip, FS_ACTIVATE);
		}
	} else {
		timer_insert(tip, FS_ACTIVATE);
	}

	KEROP_STOP();
}


//
// If a timer is active remove it from the timer list.
//
int rdecl
timer_deactivate(TIMER *tip) {
    unsigned			flags;
	struct timer_queue	*queue;

	KEROP_START();
	flags = tip->flags;
	// Set interval to zero to prevent timer_rearm() from doing anything
	tip->itime.interval_nsec = 0;
	queue = tip->queue;
	if(queue != NULL) {
		atomic_order(tip->queue = NULL);
		// It's important on weakly ordered memory systems (e.g. PPC)
		// that the above "tip->queue = NULL;" gets seen by all processors
		// before we proceed with the pending check. Otherwise we can
		// have the timer_fire() code not know that we're deactivating
		// the timer. If timer_expiry() is executing, we have to
		// wait for that to complete before checking tip->pending
		// to make sure that any timer_fire() in progress has a
		// chance to complete and set the pending indicator before
		// we look at it.
		TICKER_WAIT();
		if(tip->pending != NULL) {
			// If it's on the pending list, get it off.
			timer_pending(tip);
			flags = 0; // timer had already fired the event
			// timer_pending() does a KEROP_START/KEROP_STOP sequence and
			// they don't nest. We have to re-establish the fact that
			// the kernel is in the process of mucking with timer entries
			// so timer_expiry() and friends keep their fingers off.
			KEROP_START();
		}
	}

	if(tip->flags & _NTO_TI_ACTIVE) {
		tip->flags &= ~_NTO_TI_ACTIVE;
		// Careful of order in which entry is unlinked, timer_expiry()
		// might be running.
		CHECK_LINKAGES(tip);
		tip->link.next->link.prev = tip->link.prev;
		tip->link.prev->link.next = tip->link.next;
		CRASHCHECK(queue == NULL);
		--queue->num_active;
	}

	KEROP_STOP();
	return flags & _NTO_TI_ACTIVE;
}


TIMER * rdecl
timer_alloc(PROCESS *prp) {
	return object_alloc(prp, &timer_souls);
}


void rdecl
timer_free(PROCESS *prp, TIMER *tip) {
	// We have to wait for the ticker to exit (on SMP) so that
	// we know it's not looking at the timer that we're about to free.
	// We know that timer_expiry() won't look at the timer the next time it
	// executes because the timer has been deactivated and won't be in
	// the active timer list(s).
	TICKER_WAIT();

	CRASHCHECK(tip->queue != NULL);
	CRASHCHECK(tip->pending != NULL);
	object_free(prp, &timer_souls, tip);
}


static void
timer_next_preempt(THREAD *thp, CPU_REGISTERS *regs) {
	KEROP_STOP();
}

static const struct fault_handlers timer_next_handlers = {
	0, timer_next_preempt
};


void rdecl
timer_next(uint64_t *np) {
	struct timer_queue	*queue;
	TIMER				*tip;
	TIMER				*first;
	unsigned			i;
	uint64_t			tspec;
	uint64_t			check;
	uint64_t			tod_adj;

	tspec = ~(uint64_t)0;

	SET_XFER_HANDLER(&timer_next_handlers);
	KEROP_START();

	{
	extern volatile uint64_t	*nssptr;
	do {
		check = qtimeptr->nsec;
		tod_adj = qtimeptr->nsec_tod_adjust;
		// make sure we don't pick up the tod adjust value
		// while it's being updated
	} while(check != *nssptr);
	}

	for(queue = queue_head; queue != NULL; queue = queue->next) {
		if(queue->num_active != 0) {
			for(i = 0; i <= queue->mask; ++i) {
				tip = first = (TIMER *)&queue->slot[i];
				for( ;; ) {
					tip = tip->link.next;
					if(tip == first) goto next;
					if((tip->clockid != CLOCK_SOFTTIME) && (tip->pending == NULL)) break;
				}
				check = tip->itime.nsec;
				if(!(tip->flags & _NTO_TI_TOD_BASED)) {
					check += tod_adj;
				}
				if(check < tspec) tspec = check;
next:			;
			}
		}
	}
	KEROP_STOP();
	SET_XFER_HANDLER(NULL);

	*np = tspec;
}


void rdecl
timeout_start(THREAD *thp) {

	if(thp->timeout && (thp->timeout_flags & (1 << thp->state))) {
		switch(thp->state) {
		case STATE_WAITPAGE:
		case STATE_STACK:
		case STATE_WAITCTX:
			// These are non-kernel call triggered states, so they shouldn't
			// cause a TimerTimeout() to activate.
			break;
		default:
			// The timeout_start() function can be called multiple times
			// for the same timer (e.g block_and_ready(), which then invokes
			// the block() function will both call it). So we have to be
			// careful not to try to call timer_activate() multiple times.
			if(!(thp->timeout_flags & _NTO_TIMEOUT_ACTIVE)) {
				thp->timeout_flags |= _NTO_TIMEOUT_ACTIVE;
				timer_activate(thp->timeout);
			}
			break;
		}
	}
}


void rdecl
timeout_stop(THREAD *thp) {
	TIMER *tip;

	switch(thp->state) {
	case STATE_WAITPAGE:
	case STATE_STACK:
	case STATE_WAITCTX:
		// These are non-kernel call triggered states, so they shouldn't
		// deactivate a TimerTimeout()
		break;
	default:
		if(thp->timeout_flags & _NTO_TIMEOUT_ACTIVE) {
			tip = thp->timeout;
			if(tip == NULL) crash();

			// This is a special case since it returns a value.
			if(thp->state == STATE_NANOSLEEP) {

				// Calculate time remaining if we were interrupted.
				thp->flags |= _NTO_TF_NANOSLEEP;
				// Save time unslept for.
				if(KSTATUS(thp) != 0) {
					timer_remaining(thp->timeout, &thp->args.to.left);
				} else {
					thp->args.to.left.nsec = 0;
				}
			}
			timer_deactivate(tip);
		}
		thp->timeout_flags = 0;
		break;
	}
}

__SRCVERSION("nano_timer.c $Rev: 204314 $");
