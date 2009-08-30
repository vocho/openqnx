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

#define VALID_CLOCKID(kap)	\
	switch((kap)->id) {		\
	case CLOCK_MONOTONIC:	\
	case CLOCK_REALTIME:	\
	case CLOCK_SOFTTIME:	\
		break;				\
	default:				\
		return EINVAL;		\
	}

// The "+ qtimeptr->nsec_inc" is to round us up to the
// next clock tick, since POSIX says we can't wait less
// than the specified amount.
#define POSIX_REL_ROUNDUP(rel)	((rel)+qtimeptr->nsec_inc)


int kdecl
ker_timer_create(THREAD *act, struct kerargs_timer_create *kap) {
	PROCESS	*prp;
	TIMER	*tip;
	int		 id;

	VALID_CLOCKID(kap);

	if(kap->event) {
		RD_VERIFY_PTR(act, kap->event, sizeof(*kap->event));
		RD_PROBE_INT(act, kap->event, sizeof(*kap->event) / sizeof(int));
	}

	prp = act->process;
	lock_kernel();

	// Allocate a timer entry.
	if((tip = timer_alloc(prp)) == NULL) {
		return EAGAIN;
	}

	tip->clockid = kap->id;
	if(kap->event != NULL) {
		tip->event = *kap->event;

		if(SIGEV_GET_TYPE(&tip->event) == SIGEV_PULSE) {
			CONNECT	*cop;
			CHANNEL	*chp;

			// validate permission to send pulse as per MsgSendpulse()
			if((cop = lookup_connect(tip->event.sigev_coid)) == NULL || cop->type != TYPE_CONNECTION || (chp = cop->channel) == NULL) {
				timer_free(prp, tip);
				return EBADF;
			}
			if(!kerisusr(act, chp->process)) {
				timer_free(prp, tip);
				return ENOERROR;
			}
		}
	} else {
		tip->event.sigev_notify = SIGEV_SIGNAL;
		tip->event.sigev_signo = SIGALRM;
	}

	// We map SIGEV_SIGNAL to SIGEV_SIGNAL_CODE or the code will
	// always be set to SI_USER by the event delivery code.
	if(SIGEV_GET_TYPE(&tip->event) == SIGEV_SIGNAL) {
		SIGEV_SET_TYPE(&tip->event, SIGEV_SIGNAL_CODE);
		tip->event.sigev_code = SI_TIMER;
	}
	tip->flags &= ~_NTO_TI_ACTIVE;

	// Most timers target the process
	if(SIGEV_GET_TYPE(&tip->event) != SIGEV_SIGNAL_THREAD) {
		tip->flags |= _NTO_TI_TARGET_PROCESS;
	}

	// Add the channel to the process's timers vector.
	if((id = vector_add(&prp->timers, tip, 0)) == -1) {
		timer_free(prp, tip);
		return EAGAIN;
	}

	if(kap->event == NULL) {
		// POSIX says that with a NULL sigevent, the value field of
		// the signal should be the timer id.
		tip->event.sigev_value.sival_int = id;
	}

	// Put the timer id in the top 16 bits of the notify type
	// It will be used in intrevent_drain() to adjust overruns
	// interevent_exe() will only look at the lower 16 bits.
	tip->event.sigev_notify += (id + 1) << 16;

	SETKSTATUS(act, id);
	return ENOERROR;
}


int kdecl
ker_timer_destroy(THREAD *act, struct kerargs_timer_destroy *kap) {
	PROCESS	*prp;
	TIMER	*tip;
	int		 id = kap->id;

	prp = act->process;
	lock_kernel();

	// Remove the timer from the process's timer vector.
	if((tip = vector_rem(&prp->timers, id)) == NULL) {
		return EINVAL;
	}

	// If timer active we must unlink it from the list of active timers.
	timer_deactivate(tip);

	// Release the timer entry.
	timer_free(prp, tip);

	return EOK;
}


int kdecl
ker_timer_settime(THREAD *act, struct kerargs_timer_settime *kap) {
	TIMER	*tip;
	struct _itimer left;

	/*Prevent lint from complaining about an uninitialized var.*/
	left.nsec			= 0;
	left.interval_nsec	= 0;

	// Make sure timer is valid.
	if((tip = vector_lookup(&act->process->timers, kap->id)) == NULL) {
		return EINVAL;
	}
	RD_VERIFY_PTR(act, kap->itime, sizeof(*kap->itime));
	RD_PROBE_INT(act, kap->itime, sizeof(*kap->itime) / sizeof(int));

	if(kap->oitime) {
		WR_VERIFY_PTR(act, kap->oitime, sizeof(*kap->oitime));
		WR_PROBE_INT(act, kap->oitime, sizeof(*kap->oitime) / sizeof(int));
		lock_kernel();
		timer_remaining(tip, &left);
	}
	lock_kernel();

#ifdef _mt_LTT_TRACES_	/* PDB */
	//mt_TRACE_DEBUG("auto periodicity");
	//mt_trace_task_periodicity(act->process->pid, act->tid, tip->itime.interval_nsec);
	mt_trace_task_periodicity(act->process->pid, act->tid, kap->itime->interval_nsec);
#endif

	// If the timer is active we must unlink it from the list of active timers.
	// To let the user know if a real event was queued, we adjust the time left.
	// If nsec is zero, we guarantee the event was queued.
	if(!timer_deactivate(tip)) {
		left.nsec = 0;
	} else if(!left.nsec) {
		left.nsec = 1;
	}

	// Check for timer disable (nsec == 0).
	if(kap->itime->nsec != 0) {
		tip->flags &= ~_NTO_TI_TOD_BASED;
		if(kap->flags & TIMER_ABSTIME) {
			tip->flags |= _NTO_TI_ABSOLUTE;
			tip->itime.nsec = kap->itime->nsec;
			if(tip->clockid != CLOCK_MONOTONIC) {
				tip->flags |= _NTO_TI_TOD_BASED;
			}
		} else {
			tip->flags &= ~(_NTO_TI_ABSOLUTE|_NTO_TI_TOD_BASED);
			SNAP_TIME_INLINE(tip->itime.nsec, 0);
			tip->itime.nsec += POSIX_REL_ROUNDUP(kap->itime->nsec);
		}

		// Save interval of the timer (repeat rate). This may be zero.
		tip->itime.interval_nsec = kap->itime->interval_nsec;
		tip->flags &= ~_NTO_TI_EXPIRED;

		// Set the thread to use for delivery
		tip->thread = act;

		timer_activate(tip);
	}

	if(kap->oitime) {
		*kap->oitime = left;
	}

	return EOK;
}


int kdecl
ker_timer_info(THREAD *act, struct kerargs_timer_info *kap) {
	TIMER				*tip;
	PROCESS				*prp;
	unsigned			id;
	struct _timer_info	*inp = kap->info;

	// Lookup process.
	prp = kap->pid ? lookup_pid(kap->pid) : act->process;
	if(prp == NULL) {
		return ESRCH;
	}

	// Check for perms.
	if((kap->flags & _NTO_TIMER_RESET_OVERRUNS) && !kerisusr(act, prp)) {
		return ENOERROR;
	}

	// Make sure timer is valid.
	id = kap->id;
	tip = vector_search(&prp->timers, id,
			(kap->flags & _NTO_TIMER_SEARCH) ? &id : NULL);
	if(tip == NULL) {
		return EINVAL;
	}

	if(inp) {
		WR_VERIFY_PTR(act, inp, sizeof(*inp));
		WR_PROBE_INT(act, &inp->otime, sizeof(inp->otime) / sizeof(int));
		inp->event = tip->event;
		inp->event.sigev_notify &= 0xffff; // Turn off timer id in top 16 bits
		inp->notify = inp->event.sigev_notify;
		inp->flags = tip->flags;
		inp->overruns = tip->overruns;
		if(tip->flags & _NTO_TI_TARGET_PROCESS) {
			inp->tid = 0;
		} else {
			inp->tid = tip->thread->tid + 1;
		}
		inp->clockid = tip->clockid;
		inp->itime = tip->itime;
		lock_kernel();
		timer_remaining(tip, &inp->otime);
	}

	lock_kernel();

	// Clear overruns
	if(kap->flags & _NTO_TIMER_RESET_OVERRUNS) {
		tip->overruns = 0;
	}

	SETKSTATUS(act, id);
	return ENOERROR;
}


int kdecl
ker_timer_alarm(THREAD *act, struct kerargs_timer_alarm *kap) {
	PROCESS			*prp;
	TIMER			*tip;
	struct _itimer new;
	int				id = kap->id;	// kap->id may be corrupted by SETKSTATUS

	new.nsec = 0; new.interval_nsec = 0;

	VALID_CLOCKID(kap);

	if(kap->itime) {
		RD_VERIFY_PTR(act, kap->itime, sizeof(*kap->itime));
		new = *kap->itime;		// Grab from user space before locking kernel
	}

	tip = (prp = act->process)->alarm;

	// Does the caller want to know the old time?
	if(kap->otime) {
		WR_VERIFY_PTR(act, kap->otime, sizeof(*kap->otime));
		WR_PROBE_INT(act, kap->otime, sizeof(*kap->otime) / sizeof(int));
		if(tip) {
			lock_kernel();
			timer_remaining(tip, kap->otime);
		} else {
			memset(kap->otime, 0, sizeof(*kap->otime));
		}
	}

	if(kap->itime == 0) {
		lock_kernel();

		return EOK;
	}

	lock_kernel();
	SETKSTATUS(act, 0);		// Default to EOK unless an error changes it

	// Is there already an alarm timer inplace?
	if(tip) {
		timer_deactivate(tip);
		if(new.nsec == 0) {
			// Free the alarm.
			prp->alarm = NULL;
			timer_free(prp, tip);
			return ENOERROR;
		}
	} else if(new.nsec) {
		if((prp->alarm = tip = timer_alloc(prp)) == NULL) {
			return EAGAIN;
		}
	} else {
		return ENOERROR;
	}

	tip->clockid				= id;
	tip->thread					= act;
	tip->flags                  = _NTO_TI_TARGET_PROCESS;
	tip->event.sigev_notify     = SIGEV_SIGNAL;
	tip->event.sigev_signo      = SIGALRM;
	tip->event.sigev_code       = SI_USER;
	tip->event.sigev_value.sival_int = 0;
	SNAP_TIME_INLINE(tip->itime.nsec, 0);
	tip->itime.nsec += POSIX_REL_ROUNDUP(new.nsec);

	// Save interval of the timer (repeat rate). This may be zero.
	tip->itime.interval_nsec  = new.interval_nsec;

	timer_activate(tip);
	return ENOERROR;
}


int kdecl
ker_timer_timeout(THREAD *act, struct kerargs_timer_timeout *kap) {
	TIMER		*tip;
	int			tls_flags;
	int			id = kap->id;	// kap->id may be corrupted by SETKSTATUS

	VALID_CLOCKID(kap);

	if(kap->ntime) {
		RD_VERIFY_PTR(act, kap->ntime, sizeof(*kap->ntime));
		RD_PROBE_INT(act, kap->ntime, sizeof(*kap->ntime) / sizeof(int));
	}

	if(kap->event) {
		RD_VERIFY_PTR(act, kap->event, sizeof(*kap->event));
		RD_PROBE_INT(act, kap->event, sizeof(*kap->event) / sizeof(int));
	}

	if((kap->timeout_flags & (1 << STATE_NANOSLEEP)) && kap->otime) {
		WR_VERIFY_PTR(act, kap->otime, sizeof(*kap->otime));
		WR_PROBE_OPT(act, kap->otime, sizeof(*kap->otime) / sizeof(int));
	}

	tls_flags = act->un.lcl.tls->__flags;

	lock_kernel();

	// Return prev flags and load new flags.
	SETKSTATUS(act, act->timeout_flags);
	act->timeout_flags = kap->timeout_flags & _NTO_TIMEOUT_MASK;

	// If no ntime is provided they don't want to block at all.
	if(kap->ntime == NULL  ||  timer_past(id, kap->timeout_flags, kap->ntime))
		act->timeout_flags |= _NTO_TIMEOUT_IMMEDIATE;

	// Is there already a timeout timer inplace? If so replace it.
	if((tip = act->timeout)) {
		if(tip->flags & _NTO_TI_ACTIVE) crash();
		if(kap->timeout_flags == 0) {
			// Delete the timer.
			act->timeout = NULL;
			timer_free(act->process, tip);
			return ENOERROR;
		}
	} else if(kap->timeout_flags) {
		if((act->timeout = tip = timer_alloc(act->process)) == NULL) {
			return EAGAIN;
		}
	} else {
		return ENOERROR;
	}

	tip->clockid = id;
	tip->thread  = act;
	if(kap->event == NULL) {
		tip->event.sigev_notify = SIGEV_UNBLOCK;
	} else {
		tip->event	= *kap->event;
	}
	tip->itime.interval_nsec = 0;
	if(kap->ntime) {
		tip->flags &= ~_NTO_TI_TOD_BASED;
		if(kap->timeout_flags & TIMER_ABSTIME) {
			tip->flags |= _NTO_TI_ABSOLUTE;
			tip->itime.nsec = *kap->ntime;
			if(tip->clockid != CLOCK_MONOTONIC) tip->flags |= _NTO_TI_TOD_BASED;
		} else {
			tip->flags &= ~(_NTO_TI_ABSOLUTE|_NTO_TI_TOD_BASED);
			SNAP_TIME_INLINE(tip->itime.nsec, 0);

			tip->itime.nsec += POSIX_REL_ROUNDUP(*kap->ntime);
		}
	}

	// Must remember address to save time left if a signal hits us.
	if(act->timeout_flags & (1 << STATE_NANOSLEEP)) {

		if(PENDCAN(tls_flags)) {
			SETKIP_FUNC(act, act->process->canstub);
			return ENOERROR;
		}

		act->args.to.timeptr = kap->otime;			// Save return time ptr.
		SETKSTATUS(act,0);
		act->state = STATE_NANOSLEEP;
		_TRACE_TH_EMIT_STATE(act, NANOSLEEP);
		block();
	}
	return ENOERROR;
}

__SRCVERSION("ker_timer.c $Rev: 198610 $");
