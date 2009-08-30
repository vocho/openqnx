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

static __inline__ unsigned __attribute__((__unused__))
_smp_cmpxchg(volatile unsigned *__dst, unsigned __cmp, unsigned __src) {
	unsigned	__res;

	__asm__ __volatile__(
		"	sync;"
		"1:;"
		"	lwarx	%0,0,%1;"
		"	cmpw	%0,%2;"
		"	bne-	2f;"
		"   dcbt	0,%1;"
		"	stwcx.	%3,0,%1;"
		"	bne-	1b;"
		"2:;"
		"	isync"
		: "=&r" (__res)
		: "b" (__dst), "r" (__cmp), "r" (__src)
		: "memory"
	);
	return(__res);
}

static __inline__ unsigned __attribute__((__unused__))
_smp_xchg(volatile unsigned *__dst, unsigned __new) {
	unsigned	__res;

	__asm__ __volatile__(
		"	sync;"
		"1:;"
		"	lwarx	%0,0,%1;"
		"   dcbt	0,%1;"
		"	stwcx.	%2,0,%1;"
		"	bne-	1b;"
		"	isync"
		: "=&r" (__res)
		: "b" (__dst), "r" (__new)
		: "memory"
	);
	return(__res);
}

#endif

/* __SRCVERSION("smpxchg.h $Rev: 153052 $"); */
