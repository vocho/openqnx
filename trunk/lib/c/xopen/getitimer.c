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
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>

static int timer_tid = -1, timer_pid;

int getitimer(int which, struct itimerval *value) {
    struct itimerspec timer;

    switch (which) {
      case ITIMER_REAL:
	if (timer_tid == -1) {
	    errno = EINVAL;
	    return -1;
	}

	if (timer_gettime(timer_tid, &timer) == -1)
	    return -1;

	value->it_interval.tv_sec = timer.it_interval.tv_sec;
	value->it_interval.tv_usec = timer.it_interval.tv_nsec / 1000;
	value->it_value.tv_sec = timer.it_value.tv_sec;
	value->it_value.tv_usec = timer.it_value.tv_nsec / 1000;
	return 0;

      case ITIMER_VIRTUAL:
      case ITIMER_PROF:
      default:
	errno = EINVAL;
	return -1;
    }
}

int setitimer(int which, const struct itimerval *value, struct itimerval *ovalue) {
    struct itimerspec timer;
    struct sigevent event;

    switch (which) {
      case ITIMER_REAL:
	if (timer_tid == -1 || timer_pid != getpid()) {
	    event.sigev_notify = SIGEV_SIGNAL;
	    event.sigev_signo = SIGALRM;
	    event.sigev_value.sival_int = SI_USER;
	    (void)timer_create(CLOCK_REALTIME, &event, &timer_tid);
	    timer_pid = getpid();
	}
    
	timer.it_interval.tv_sec = value->it_interval.tv_sec;
	timer.it_interval.tv_nsec = 1000 * value->it_interval.tv_usec;
	timer.it_value.tv_sec = value->it_value.tv_sec;
	timer.it_value.tv_nsec = 1000 * value->it_value.tv_usec;

	if (timer_settime(timer_tid, 0, &timer, &timer) == -1)
	    return -1;

	if (ovalue) {
	    ovalue->it_interval.tv_sec = timer.it_interval.tv_sec;
	    ovalue->it_interval.tv_usec = timer.it_interval.tv_nsec / 1000;
	    ovalue->it_value.tv_sec = timer.it_value.tv_sec;
	    ovalue->it_value.tv_usec = timer.it_value.tv_nsec / 1000;
	}
	return 0;

      case ITIMER_VIRTUAL:
      case ITIMER_PROF:
      default:
	errno = EINVAL;
	return -1;
    }
}

__SRCVERSION("getitimer.c $Rev: 153052 $");
