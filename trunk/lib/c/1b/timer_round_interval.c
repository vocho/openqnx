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



/* _timer_round_interval() - round the given interval up to the nearest multiple of
 *      the clock resolution.
 */
void _timer_round_interval( _Uint64t* intervalp ) {
	_Uint64t res = SYSPAGE_ENTRY(qtime)->nsec_inc;

	/* round up to the nearest clock tick */
	*intervalp = (((*intervalp - 1) + res) / res) * res;

}

__SRCVERSION("timer_round_interval.c $Rev: 159797 $");
