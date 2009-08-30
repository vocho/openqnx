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

// Don't use top bit so the clockid_t is positive
#define CLOCK_ID_RUNTIME	0x40000000

#define MAKE_CLOCKID(pid, tid)	\
	(CLOCK_ID_RUNTIME | (PINDEX(pid) << 16) | ((tid) & 0xffff))

//
// This kernel call allows you to get and/or set the time.
// It also can return the running time of an arbitrary process or thread.
// The time is provided in seconds and nanoseconds.
//
int kdecl
ker_clock_time(THREAD *act, struct kerargs_clock_time *kap) {
	uint64_t		old, new;
	unsigned		id;

	id = kap->id;

	switch(id) {
	case CLOCK_PROCESS_CPUTIME_ID:
		id = MAKE_CLOCKID(act->process->pid, 0);
		break;
	case CLOCK_THREAD_CPUTIME_ID:
		id = MAKE_CLOCKID(act->process->pid, act->tid + 1);
		break;
	default:
		break;
	}

	if(id & CLOCK_ID_RUNTIME) {
		unsigned			pidx;
		unsigned			tid;
		PROCESS				*prp;
		THREAD				*thp;
		volatile uint64_t	*rtp;

		// Get the running time of a process or thread

		if(kap->new != NULL) {
			return EINVAL;
		}
		pidx = SYNC_PINDEX(id);
		tid = SYNC_TID(id);
		thp = NULL;
		if((pidx == 0) || (pidx >= process_vector.nentries) || !VECP(prp, &process_vector, pidx)) {
			return ESRCH;
		}
		if((tid + 1) == 0) {
			// Get process running time
			rtp = &prp->running_time;
		} else {
			thp = vector_lookup(&prp->threads, tid);
			if(thp == NULL) {
				return ESRCH;
			}
			rtp = &thp->running_time;
		}

		// Loop until we get a consistent time
		do {
			old = *rtp;
		} while(old != *rtp);

		if(kap->old) {
			WR_VERIFY_PTR(act, kap->old, sizeof(*kap->old));
			WR_PROBE_OPT(act, kap->old, sizeof(*kap->old) / sizeof(int));
			*kap->old = old;
		}
		return EOK;
	}

	if((id != CLOCK_REALTIME) && ((id != CLOCK_MONOTONIC) || kap->new)) {
		return EINVAL;
	}

	// Make local copy incase kap->new==kap->old since we change old.
	if(kap->new) {
		RD_VERIFY_PTR(act, kap->new, sizeof(*kap->new));
		new = *kap->new;
	} else {
		new = 0;
	}

	// Get current time.
	if(kap->old) {
		WR_VERIFY_PTR(act, kap->old, sizeof(*kap->old));
		WR_PROBE_INT(act, kap->old, sizeof(*kap->old) / sizeof(int));
		lock_kernel();	 // have to lock because of possible new & old overlap
		snap_time(kap->old, id != CLOCK_MONOTONIC);
	}

	lock_kernel();

	SETKSTATUS(act, 0);

	// Set new time.
	if(kap->new && kerisroot(act)) {
		QTIME	*qtp = qtimeptr;

		INTR_LOCK(&clock_slock);

		// Kill any adjustment in progress
		memset(&qtp->adjust, 0, sizeof(qtp->adjust));
		
		qtp->nsec_tod_adjust = new - qtp->nsec;

		INTR_UNLOCK(&clock_slock);
	}
	return ENOERROR;
}


//
// This kernel call allows you to set an adjustment to the
// system clock. The adjustment is provided as a nanosecond
// adjustment for each timer tick for n ticks. After n ticks
// the adjustment is removed. The total adjustment is therefore
// nansecond_adjustment * num_ticks. By picking small nanosecond
// adjustments and large number of ticks you can adjust the time slowly
// without large disruptions in the time flow.
//
int kdecl
ker_clock_adjust(THREAD *act, struct kerargs_clock_adjust *kap) {
	QTIME			*qtp;
	struct _clockadjust	new;
	
	new.tick_count = 0; new.tick_nsec_inc = 0;

	if(kap->id != CLOCK_REALTIME) {
		return EINVAL;
	}

	// Make local copy incase kap->new==kap->old since we change old.
	if(kap->new) {
		RD_VERIFY_PTR(act, kap->new, sizeof(*kap->new));
		new = *kap->new;
	}

	// Get current adjustment.
	qtp = qtimeptr;
	if(kap->old) {
		WR_VERIFY_PTR(act, kap->old, sizeof(*kap->old));
		// Probe address so we don't fault while holding a spin lock
		WR_PROBE_INT(act, kap->old, sizeof(*kap->old) / sizeof(int));
		INTR_LOCK(&clock_slock);
		*kap->old     = qtp->adjust;
		INTR_UNLOCK(&clock_slock);
	}
	lock_kernel();

	SETKSTATUS(act,0);

	// Set new adjustment.
	if(kap->new  &&  kerisroot(act)) {
		INTR_LOCK(&clock_slock);
		qtp->adjust = new;
		INTR_UNLOCK(&clock_slock);
	}
	return ENOERROR;
}


//
// This kernel call allows you to get and/or set the tick size of
// the clock.
//
int kdecl
ker_clock_period(THREAD *act, struct kerargs_clock_period *kap) {
	struct _clockperiod	new;
	
	new.nsec = 0; new.fract = 0;

	if(kap->id != CLOCK_REALTIME) {
		return EINVAL;
	}

	// Make local copy incase kap->new==kap->old since we change old.
	if(kap->new) {
		RD_VERIFY_PTR(act, kap->new, sizeof(*kap->new));
		new = *kap->new;
	}

	// Get current resolution.
	if(kap->old) {
		WR_VERIFY_PTR(act, kap->old, sizeof(*kap->old));
		WR_PROBE_INT(act, kap->old, sizeof(*kap->old) / sizeof(int));
		lock_kernel(); /* just in case "kap->old==kap->new" */
		kap->old->fract = 0;
		kap->old->nsec = qtimeptr->nsec_inc;
	}
	lock_kernel();

	KSTATUS(act) = 0;

	// Set new resolution.
	if(kap->new  &&  kerisroot(act)) {
		if(	(new.nsec < tick_minimum)  ||
			(new.nsec > _NTO_TICKSIZE_MAX)) {
			return EINVAL;
		}

		clock_resolution(new.nsec);
		timer_period();
	}
	return ENOERROR;
}



//
// This kernel call returns a clock id that can be used to obtain the
// running time of a process or thread.
//
int kdecl
ker_clock_id(THREAD *act, struct kerargs_clock_id *kap) {
	unsigned	id;
	unsigned	tid;
	PROCESS		*prp;

	// Verify the target process exists.
	if((prp = (kap->pid ? lookup_pid(kap->pid) : act->process)) == NULL) {
		return ESRCH;
	}

	tid = kap->tid;

	// Verify the target thread exists.
	if(tid != 0 && vector_lookup(&prp->threads, tid - 1) == NULL) {
		return ESRCH;
	}

	id = MAKE_CLOCKID(prp->pid, tid);

	lock_kernel();

	KSTATUS(act) = id;

	return ENOERROR;
}

__SRCVERSION("ker_clock.c $Rev: 163913 $");
