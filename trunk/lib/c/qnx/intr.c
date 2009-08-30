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




#include <errno.h>
#include <signal.h>
#include <intr.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <inttypes.h>

#if defined(__WATCOMC__) && __WATCOMC__ <= 1100
#define FIXCONST	__based(__segname("CONST2"))
#else
#define FIXCONST
#endif

#ifndef _NTO_TIMEOUT_INTR
#define _NTO_TIMEOUT_INTR		(1<<STATE_INTR)
#endif

static struct handlers {
	intr_t			intr;
	int				(*handler)(volatile void *area);
	volatile void	*area;
	int				iid;
} handlers[_POSIX_INTR_CONNECT_MAX];

static const struct sigevent *intr_stub(void *data, int id) {
	struct handlers					*handler = data;
	static const struct sigevent	FIXCONST unblock = {
		SIGEV_INTR
	};

	switch(handler->handler(handler->area)) {
	case INTR_HANDLED_NOTIFY:
		return &unblock;

	case INTR_HANDLED_DO_NOT_NOTIFY:
		// should stop checking rest of interrupts???
		break;

	case INTR_NOT_HANDLED:
		break;
	default:
		break;
	}
	return 0;
}

int intr_capture(intr_t intr, int (*intr_handler)(volatile void *area), volatile void *area, size_t areasize) {
	int						ret;
	struct handlers			*handler;

	for(handler = handlers; handler < &handlers[sizeof handlers / sizeof *handlers]; handler++) {
		if(!handler->handler) {
			handler->intr = intr;
			handler->handler = intr_handler;
			handler->area = area;
#if 0
//			mlock(area, areasize);
//			mlock(_text, _data - _text);
#else
//			mlockall(MCL_FUTURE);
#endif
			if((ret = InterruptAttach_r(intr, intr_stub, handler, sizeof *handler, 0)) < EOK) {
				handler->handler = 0;
				return -ret;
			}
			handler->iid = ret;
			return EOK;
		}
	}
	return EAGAIN;
}

int intr_release(intr_t intr, int (*intr_handler)(volatile void *area)) {
	int						ret;
	struct handlers			*handler;

	for(handler = handlers; handler < &handlers[sizeof handlers / sizeof *handlers]; handler++) {
		if(intr == handler->intr && intr_handler == handler->handler) {
			ret = InterruptDetach_r(handler->iid);
			handler->handler = 0;
			return ret;
		}
	}
	return EINVAL; // ENOISR;
}

int intr_timed_wait(int flags, const struct timespec *timeout) {
	int						ret;

	if(timeout) {
		uint64_t		t = timespec2nsec(timeout);

		if((ret = TimerTimeout_r(CLOCK_REALTIME, _NTO_TIMEOUT_INTR, 0, &t, 0)) != EOK) {
			return ret;
		}
	}
	return InterruptWait_r(flags, 0);
}

int intr_lock(intr_t intr) {
	return InterruptMask(intr, -1) == -1 ? EACCES : EOK;
}

int intr_unlock(intr_t intr) {
	return InterruptUnmask(intr, -1) == -1 ? EACCES : EOK;
}

__SRCVERSION("intr.c $Rev: 153052 $");
