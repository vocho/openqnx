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
 *  smpxchg.h
 *

 */

#ifndef __SMPXCHG_H_INCLUDED
#define __SMPXCHG_H_INCLUDED

/*
    code sequences for SMP atomic exchanging of mutex stuff.
*/

static __inline__ unsigned _smp_cmpxchg(volatile unsigned *__dst, unsigned __cmp, unsigned __src) {
	unsigned __result;

	return(__result);
}

static __inline__ unsigned _smp_xchg(volatile unsigned *__dst, unsigned __new) {
	unsigned __result;

	return(__result);
}

#endif

/* __SRCVERSION("smpxchg.h $Rev: 153052 $"); */
