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




#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <util/fish_time.h>

#ifdef USE_OLD_NTO_SYSPAGE_FISHTIME
struct qtime_entry *fish_qnx_time(void)
{
	return(_syspage_ptr->qtimeptr);
}

time_t fish_time ( time_t *time )
{
	struct qtime_entry *ptr;
	time_t result;

	if ((ptr=fish_qnx_time())==NULL) result = 0;
	else {
		do {
			result = ptr->sec;
		} while (result != ptr->sec);
	}

	if (time!=NULL) *time = result;
	return(result);
}
#else

time_t fish_time (time_t *tsec)
{
	// time() in nto calls ClockTime_r (kernel call, not msg pass to Proc)
	return time(tsec);
}

#endif

