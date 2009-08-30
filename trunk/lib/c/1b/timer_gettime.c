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




#include <time.h>
#include <sys/neutrino.h>


extern void _timer_round_interval( _Uint64t* );

int timer_gettime(timer_t timerid, struct itimerspec *value) {
	struct _timer_info		info;
	
	if(TimerInfo(0, timerid, 0, &info) == -1) {
		return -1;
	}
	if(value) {
		_timer_round_interval( &info.otime.interval_nsec );
		_timer_round_interval( &info.otime.nsec );
		nsec2timespec(&value->it_value, info.otime.nsec);
		nsec2timespec(&value->it_interval, info.otime.interval_nsec);
	}
	return 0;
}

__SRCVERSION("timer_gettime.c $Rev: 153052 $");
