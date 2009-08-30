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

int kdecl
ker_interrupt_attach(THREAD *act,struct kerargs_interrupt_attach *kap) {
	int			intr;
	int			id;
	int			flags;
	void		*area;
	int			r;

	if((kap->intr >= _NTO_INTR_CLASS_SYNTHETIC) && (kap->intr <= _NTO_HOOK_LAST)) {
		if(!kerisroot(act)) return EPERM;
#if !defined(VARIANT_instr)
		if(kap->intr == _NTO_HOOK_TRACE) return ENOTSUP;
#endif
	} else {
		if(!(act->flags & _NTO_TF_IOPRIV)) return EPERM;
	}

    if((intr = get_interrupt_level(act, kap->intr)) == -1) return ENOERROR;

	// If no handler is provided we assume the area points to a sigevent
	// which will be triggered on each interrupt.
	area = kap->area;
	flags = kap->flags;
	if(kap->handler == NULL) {
		struct sigevent	*event;

		RD_VERIFY_PTR(act, kap->area, sizeof(struct sigevent));
		RD_PROBE_INT(act, kap->area, sizeof(struct sigevent) / sizeof(int));
		lock_kernel();

		if((event = _smalloc(sizeof(struct sigevent))) == NULL) {
			return ENOMEM;
		}

		*event = *(struct sigevent *)kap->area;
		switch(SIGEV_GET_TYPE(event)) {
		case SIGEV_SIGNAL:
		case SIGEV_SIGNAL_CODE:
		case SIGEV_PULSE:
			flags |= _NTO_INTR_FLAGS_PROCESS;
			break;
		default:
			break;
		}
		area = event;
	} else {
		lock_kernel();
		//Check with the memory manager to make sure that
		//It's locked all the memory for the process
		r = memmgr.mlock(act->process, 0, 0, -1);
		if(r == -1) {
			// The memmgr has blocked this thread and sent
			// a pulse to lock the memory down. Restart the
			// kernel call - when we come in again all should be well.
			KERCALL_RESTART(act);
			return ENOERROR;
		}
		if(r != EOK) return r;
	}

	id = interrupt_attach(intr, kap->handler, area, flags);
	if(id >= 0) {
		SETKSTATUS(act, id);
	}
	return ENOERROR;
}


int kdecl
ker_interrupt_detach_func(THREAD *act,struct kerargs_interrupt_detach_func *kap) {
	int			 intr;

	if(!(act->flags & _NTO_TF_IOPRIV)) return EPERM;

    if((intr = get_interrupt_level(act, kap->intr)) == -1) return ENOERROR;

	interrupt_detach(intr, kap->handler);

	return EOK;
}


int kdecl
ker_interrupt_detach(THREAD *act,struct kerargs_interrupt_detach *kap) {
	INTERRUPT	*itp;

	if(!(act->flags & _NTO_TF_IOPRIV)) return EPERM;

	itp = vector_lookup(&interrupt_vector, kap->id);
	if(itp == NULL) return ESRCH;

	if(itp->thread->process != act->process) return EPERM;

	interrupt_detach_entry(act->process, kap->id);

	return EOK;
}


int kdecl
ker_interrupt_mask(THREAD *act,struct kerargs_interrupt_mask *kap) {
	int		status;
	int		intr;

	if(!(act->flags & _NTO_TF_IOPRIV)) return EPERM;

    if((intr = get_interrupt_level(act, kap->intr)) == -1) return ENOERROR;

	lock_kernel();
	status = interrupt_mask(intr, vector_lookup(&interrupt_vector, kap->id));
	if(status == -1) kererr(act, EINVAL);
	SETKSTATUS(act, status);
	return ENOERROR;
}


int kdecl
ker_interrupt_unmask(THREAD *act,struct kerargs_interrupt_unmask *kap) {
	int		status;
	int		intr;

	if(!(act->flags & _NTO_TF_IOPRIV)) return EPERM;

    if((intr = get_interrupt_level(act, kap->intr)) == -1) return ENOERROR;

	lock_kernel();
	status = interrupt_unmask(intr, vector_lookup(&interrupt_vector, kap->id));
	if(status == -1) kererr(act, EINVAL);
	SETKSTATUS(act, status);
	return ENOERROR;
}


int kdecl
ker_interrupt_wait(THREAD *act,struct kerargs_interrupt_wait *kap) {

	if(PENDCAN(act->un.lcl.tls->__flags)) {
		lock_kernel();
		SETKIP_FUNC(act, act->process->canstub);
		return ENOERROR;
	}

	if(act->flags & _NTO_TF_INTR_PENDING) {
		lock_kernel();
		act->flags &= ~_NTO_TF_INTR_PENDING;
		act->timeout_flags = 0;
		//run threads getting a SIG_INTR as critical
		AP_MARK_THREAD_CRITICAL(act);
		return EOK;
	}

	if(kap->timeout) {
		return ENOTSUP;
	}

	if(IMTO(act, STATE_INTR)) {
		return ETIMEDOUT;
	}

	lock_kernel();

#if 0
	if(kap->timeout) {
		struct kerargs_timer_timeout timeout;
		struct sigevent event;

		event.sigev_notify = SIGEV_UNBLOCK;
		timeout.timeout_flags = 1 << STATE_INTR;
		timeout.event = &event;
		timeout.ntime = kap->timeout;
		timeout.otime = NULL;
		ker_timer_timeout(&timeout);
		if(KSTATUS(act) != EOK) return EOK;
	}
#endif

	SETKSTATUS(act, 0);

	act->state = STATE_INTR;
	_TRACE_TH_EMIT_STATE(act, INTR);
	block();
	return EOK;
}

__SRCVERSION("ker_interrupt.c $Rev: 153052 $");
