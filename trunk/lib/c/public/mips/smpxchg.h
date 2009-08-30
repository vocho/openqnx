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

#if defined(__GNUC__)

static __inline__ unsigned __attribute__((__unused__))
_smp_cmpxchg(volatile unsigned *__dst, unsigned __cmp, unsigned __src) {
	unsigned __result;
	unsigned __tmp;

	__tmp = __src;
	__asm__ __volatile__ (
		" .set noreorder ;"

		"1:	ll	%0,0(%1) ;"
		"	bne	%0,%2,2f ;"
		"	 nop		 ;"
		"	move	%0,%3	 ;"
		"	sc	%0,0(%1) ;"
		"	beq	%0,$0,1b ;"
		"	 move	%0,%2	 ;"
		"2:			 ;"

		" .set reorder ;"
		: "=&r" (__result)
		: "r" (__dst), "r" (__cmp), "r" (__src)
		: "memory");
	return(__result);
}

static __inline__ unsigned __attribute__((__unused__))
_smp_xchg(volatile unsigned *__dst, unsigned __new) {
	unsigned __result;
	unsigned __tmp;
	unsigned __temp;

	__temp = *__dst;
	__asm__ __volatile__ (
		" .set noreorder ;"

		"	move	%1,%3	 ;"
		"1:	ll	%0,0(%2) ;"
		"	sc	%1,0(%2) ;"
		"	beq	%1,$0,1b ;"
		"	  move	%1,%3	 ;"

		" .set reorder ;"
		: "=&r" (__result), "=&r" (__tmp)
		: "r" (__dst), "r" (__new)
		: "memory");
	return(__result);
}

#elif defined(__MWERKS__)

asm static /*inline*/ unsigned
_smp_cmpxchg(volatile unsigned *__dst, unsigned __cmp, unsigned __src) {
	.set noreorder
	/* v0 = result, a0 = __dst, a1 = __cmp, a2 = __src */
top:
	ll v0, 0(a0)	/* load *__dst into __result */
	bne v0, a1, out	/* if __cmp != __result branch to 2 */
	nop		/* (branch delay slot) */
	or v0, a2, zero /* move __src into __result */
	sc v0, 0(a0)	/* __result is stored into *__dst if `atomic' */
	beq v0, $0, top	/* if v0 is 0 the store was unsuccessful */
	or v0, a1, zero	/* stuff __cmp as return value -- branch delay slot */
out:
	jr ra
	nop		/* branch delay slot */
}

asm static /*inline*/ unsigned
_smp_xchg(volatile unsigned *__dst, unsigned __new) {
	.set noreorder
	/* v0 = __result, v1 = __temp, a0 = __dst, a1 = __new */
	or v1, a1, zero	/* move __new into __tmp */
top:
	ll v0, 0(a0)	/* load *__dst into __result */
	sc v1, 0(a0)	/* *__dst is store into __tmp */
	beq v1, $0, top	/* see if store was successful */
	or v1, a1, zero	/* move __new into __tmp -- branch delay slot */

	jr ra
	nop		/* branch delay slot */
}
#endif

#endif

/* __SRCVERSION("smpxchg.h $Rev: 153052 $"); */
