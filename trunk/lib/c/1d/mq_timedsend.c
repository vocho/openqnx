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
#include <inttypes.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/iomsg.h>


int mq_timedsend_woption(mqd_t mq, const char *buff, size_t nbytes, 
           unsigned msgprio, const struct timespec *timeout, clockid_t clock_choice) {
	uint64_t	t = timespec2nsec(timeout);

	
	if(!TIMESPEC_VALID(timeout)) {
		errno = EINVAL;
		return -1;
	}
	if(TimerTimeout(clock_choice, TIMER_ABSTIME | _NTO_TIMEOUT_SEND | _NTO_TIMEOUT_REPLY, 0, &t, 0) == -1) {
		return -1;
	}
	return (_writex(mq, buff, nbytes, _IO_XTYPE_MQUEUE | (msgprio << 16), 0, 0) == -1) ? -1 : 0;
}

int mq_timedsend(mqd_t mq, const char *buff, size_t nbytes, 
           unsigned msgprio, const struct timespec *abs_timeout) {

/* the posix version must use CLOCK_REALTIME */ 
	return mq_timedsend_woption(mq, buff, nbytes, msgprio, abs_timeout, CLOCK_REALTIME);
}

int mq_timedsend_monotonic(mqd_t mq, const char *buff, size_t nbytes, 
           unsigned msgprio, const struct timespec *abs_mono_timeout) {
/* a version of mq_timedsend() which is not affected by TOD changes. Not POSIX */
	return mq_timedsend_woption(mq, buff, nbytes, msgprio, abs_mono_timeout, CLOCK_MONOTONIC);
}


__SRCVERSION("mq_timedsend.c $Rev: 199491 $");
