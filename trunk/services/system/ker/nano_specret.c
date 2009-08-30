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

#include <unistd.h>
#include "externs.h"

#if defined(__X86__) || defined(__PPC__) || defined(__MIPS__) || defined(__ARM__) || defined(__SH__)
    /*
		This code will work on any two's complement machine.
		It isolates the lowest 1 bit from a word.
		Wow, only place kernel relies on two's complement machines..
	*/
	#define GET_BOTTOM_BIT( v )	(((v) & ((v)-1)) ^ (v))
#else
	#error not sure we are running on a twos complement machine
#endif

//
// This routine is called when we are about to return from the kernel and
// return to a thread when we discover special processing has to be done. This

// usually involves writing to the threads address space. We hold off till
// here because at this point we know we have addressability to the thread.
//
void specialret_attr
specialret(THREAD *thp) {
	int			 flags;

	// The following code may fault since it accesses user memory.
	// If the global inspecret is set then the fault handler takes
	// special action. It overrides the INKERNEL_LOCKED flag and
	// allows a page to be allocated or brought in (dynamic stack
	// growth/paging) or if a true fault then the special return
	// flag in the thread which caused the fault is cleared and
	// a signal is set on the thread.
	//
#ifndef NDEBUG
	if(get_inkernel() & INKERNEL_SPECRET) crash();
#endif
	specret_kernel();

	while((flags = (thp->flags & _NTO_TF_SPECRET_MASK))) {
		unlock_kernel();
		SPECRET_PREEMPT(thp);
		switch((inspecret = GET_BOTTOM_BIT(flags))) {

		case _NTO_TF_TO_BE_STOPPED: {
			lock_kernel();
			thp->flags &= ~_NTO_TF_TO_BE_STOPPED;
			thp->state = STATE_STOPPED;
			_TRACE_TH_EMIT_STATE(thp, STOPPED);
			block();
			break;
		}

		case _NTO_TF_RCVINFO: {
			struct _msg_info	*mep  = thp->args.ri.info;
			CONNECT				*cop  = thp->args.ri.cop;
			THREAD				*thp2 = thp->args.ri.thp;

			if(cop->channel->flags & _NTO_CHF_GLOBAL) {
				mep->priority = thp->args.ri.value;
				mep->msglen = thp->args.ri.id;
			} else {
				get_rcvinfo(thp2, -1, cop, mep);
				if((thp->flags & _NTO_TF_SHORT_MSG) == 0) {
					// If there is no short message, remove the connection
					// of the client thread to the server thread.
					lock_kernel();
					thp2->restart = 0;
					// Clear the flag that indicates we (the client) need data from thp2's
					// (the sender's) control block.
					thp2->internal_flags &= ~_NTO_ITF_SPECRET_PENDING;
				} else {
					if(xferlen(thp, thp->args.ri.rmsg, thp->args.ri.rparts) < mep->msglen) {
						mep->msglen = xferlen(thp, thp->args.ri.rmsg, thp->args.ri.rparts);
					}
				}
			}
			thp->flags &= ~_NTO_TF_RCVINFO;

			break;
		}

		case _NTO_TF_SHORT_MSG: {
			int					status = 0;
			THREAD				*thp2 = thp->blocked_on;

#ifndef NDEBUG
// We (thp) are receiving a short message.  If thp==thp2, then this is a MsgReply.
// If thp!=thp2 then this is a MsgSend and thp2 is the sender.  thp2 better be
// in state reply-blocked when we (the receiver) start to run.
if(thp != thp2 && thp2->state != STATE_REPLY && thp2->state != STATE_NET_REPLY) crash();
#endif

			status = xfer_cpy_diov(thp, thp->args.ri.rmsg, thp2->args.msbuff.buff, thp->args.ri.rparts, thp2->args.msbuff.msglen);

			lock_kernel();
			thp->flags &= ~(_NTO_TF_SHORT_MSG | _NTO_TF_BUFF_MSG);

			// Clear the flag that indicates we (the client) need data from thp2's
			// (the sender's) control block.
			if (thp != thp2) {
				thp2->internal_flags &= ~_NTO_ITF_SPECRET_PENDING;
			}

			// Since _NTO_TF_RCVINFO is higher priority, we can always
			// clear the client connection to the server thread.
			thp2->restart = 0;

			if(status) {
				kererr(thp, EFAULT);
				if(thp != thp2) {
					/* thp is the server and thp2 is the client,
				  		this is a server fault during receiving */
					force_ready(thp2, ESRVRFAULT);
				}
			}
			break;
		}

		case _NTO_TF_PULSE: {
			int					status;

			status = xferpulse(thp, thp->args.ri.rmsg, thp->args.ri.rparts,
						thp->args.ri.code, thp->args.ri.value, thp->args.ri.id);

			lock_kernel();
			thp->flags &= ~_NTO_TF_PULSE;
			if(status) {
				kererr(thp, EFAULT);
			}

			break;
		}

		case _NTO_TF_NANOSLEEP: {
			/* use memcpy to avoid f.p. on PPC */
			if(thp->args.to.timeptr) {
				memcpy(thp->args.to.timeptr,
						&thp->args.to.left.nsec,
						sizeof(*thp->args.to.timeptr));
			}

			thp->flags &= ~_NTO_TF_NANOSLEEP;
			break;
		}

		case _NTO_TF_JOIN: {
			if(thp->args.jo.statusptr) {
				*thp->args.jo.statusptr = thp->args.jo.status;
			}

			thp->flags &= ~_NTO_TF_JOIN;
			break;
		}

		case _NTO_TF_WAAA:
			thread_specret(thp);
			break;

		case _NTO_TF_SIGWAITINFO: {
			siginfo_t *sip = thp->args.sw.sig_info;

			if(sip) {	// May be passed as NULL
				WR_PROBE_OPT(act, sip, sizeof(*sip) / sizeof(int));
				sip->si_signo = thp->args.sw.signo;
				sip->si_code = thp->args.sw.code;
				sip->si_pid = thp->args.sw.pid;
				sip->si_value.sival_int = thp->args.sw.value;
			}

			lock_kernel();
			SETKSTATUS(thp, thp->args.sw.signo);
			thp->flags &= ~_NTO_TF_SIGWAITINFO;
			break;
		}

		case _NTO_TF_SIG_ACTIVE:
			signal_specret(thp);
			break;

		case _NTO_TF_SIGSUSPEND: {
			// Handle case where sigsuspend is unblocked without a signal.
			SIGMASK_CPY(&thp->sig_blocked, &thp->args.ss.sig_blocked);
			thp->flags &= ~_NTO_TF_SIGSUSPEND;
			break;
			}

		case _NTO_TF_ACQUIRE_MUTEX: {
			// We need to save status since sync_mutex_lock() will probably
			// set it to zero. In the case of a condvar timeout we need
			// to preserve it.
			int		status = KSTATUS(thp);
			sync_t	*mutex = thp->args.mu.mutex;

			CRASHCHECK(mutex == NULL);

			sync_mutex_lock(thp, mutex, (thp->flags & _NTO_TF_MUTEX_CEILING)?_MUTEXLOCK_FLAGS_NOCEILING:0);

			if(thp->state == STATE_RUNNING) {
				// We got the mutex, update the count. Normally incr is
				// zero, but it'll be one if this mutex is being used
				// with a condvar wait
				mutex->__count += thp->args.mu.incr;
			}

			// Timeout always overrides success on getting the mutex.
			if(status == ETIMEDOUT) {
				SETKSTATUS(thp, status);
			}

			thp->flags &= ~_NTO_TF_ACQUIRE_MUTEX;
			break;
		}

		case _NTO_TF_KILLSELF: {
			SIGMASK_ONES(&thp->sig_blocked);
			lock_kernel();
			cpu_force_thread_destroyall(thp);
			if(thp->flags & _NTO_TF_RCVINFO) {
				thp->args.ri.thp->restart = 0;
			}
			if(thp->flags & _NTO_TF_SHORT_MSG) {
				((THREAD *)thp->blocked_on)->restart = 0;
			}
			thp->flags &= ~_NTO_TF_SPECRET_MASK;
			break;
		}

		case _NTO_TF_CANCELSELF: {
			lock_kernel();
			thp->flags |= _NTO_TF_KERERR_SET;
			SETKIP_FUNC(thp, thp->process->canstub);
			thp->flags &= ~_NTO_TF_CANCELSELF;
			break;
		}

		case _NTO_TF_MUTEX_CEILING: {
			SYNC *syp;
			unsigned ceiling_old;
			int ret;

			if((syp = sync_lookup(thp->args.mu.mutex, _NTO_SYNC_MUTEX_FREE)) == NULL) {
				break;
			}

			lock_kernel();
			ret = mutex_set_prioceiling(syp, thp->args.mu.ceiling, &ceiling_old);
			sync_mutex_unlock(thp, thp->args.mu.mutex, 0);
			if(ret == ENOERROR) {
				SETKSTATUS(thp, ceiling_old);
			} else {
				kererr(thp, ret);
			}
			break;
		}

		case _NTO_TF_ASYNC_RECEIVE: {
			CHANNEL *chp;
			CONNECT *cop;
			int coid, err;

			cop = NULL;
			chp = (void*)thp->blocked_on;
			if((coid = thp->args.ri.id)) {
				cop = thp->blocked_on;
				chp = cop->channel;
			}

			err = msgreceive_gbl(thp, (CHANNELGBL*) chp, thp->args.ri.rmsg, thp->args.ri.rparts, thp->args.ri.info, cop, coid);
			if(err) {
				kererr(thp, err);
			}
			break;
		}

		default:
#ifndef NDEBUG
			crash();
#endif
			break;
		}
		if(thp != actives[KERNCPU] || (get_inkernel() & INKERNEL_SPECRET) == 0) {
			break;
		}
	}

	lock_kernel();
	unspecret_kernel();
	thp->restart = 0;
}

__SRCVERSION("nano_specret.c $Rev: 158207 $");
