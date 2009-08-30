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

/*temp*/
#include <sh/cpu.h>
#include <sh/inline.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef	__KERCPU_H_
/*
 * Need to examine __cpu_flags to determine whether we can use the SH4A
 * atomic operations.
 * We can't include <sys/syspage.h> due to namespace pollution, so we
 * hardcode the SH_CPU_FLAG_MOVLICO from <sh/syspage.h>
 */
extern unsigned	__cpu_flags;
#ifndef	__SH_CPU_FLAG_MOVLICO
#define	__SH_CPU_FLAG_MOVLICO	(1<<0)
#endif

/*
 *  code sequences for SMP atomic exchanging of mutex stuff.
 */
static __inline__ unsigned __attribute__((__unused__))
_smp_cmpxchg(volatile unsigned *__dst, unsigned __cmp, unsigned __src) {
	unsigned	__res;

	if (__cpu_flags & __SH_CPU_FLAG_MOVLICO) {
		__asm__ __volatile__(
		"	mov		%1,r1;"
		"0:	.word	0x0163;"	/* movli.l @r1,r0 */
		"	cmp/eq	r0,%2;"
		"	mov		r0,%0;"
		"	bf		1f;"
		"	mov		%3,r0;"
		"	.word	0x0173;"	/* movco.l r0,@r1 */
		"	bf		0b;"
		"1:;"
		: "=&r" (__res)
		: "r" (__dst), "r" (__cmp), "r" (__src)
		: "r0", "r1", "cc", "memory"
		);
	}
	else {
		__asm__ __volatile__(
		"	mov		%2,r0;"
		"	mov		%3,r5;"
		"	mov		%1,r4;"
		"	trapa	#0xff;"
		"	mov		r0,%0;"
		: "=&r" (__res)
		: "r" (__dst), "r" (__cmp), "r" (__src)
		: "r0", "r4", "r5","cc","memory"
		);
	}
	return(__res);
}

static __inline__ unsigned __attribute__((__unused__))
_smp_xchg(volatile unsigned *__dst, unsigned __new) {
	unsigned	__res;

	if (__cpu_flags & __SH_CPU_FLAG_MOVLICO) {
		__asm__ __volatile__(
		"	mov		%1,r1;"
		"0:	.word	0x0163;"	/* movli.l @r1,r0 */
		"	mov		r0,%0;"
		"	mov		%2,r0;"
		"	.word	0x0173;"	/* movco.l r0,@r1 */
		"	bf		0b;"
		: "=&r" (__res)
		: "r" (__dst), "r" (__new)
		: "r0", "r1", "cc", "memory"
		);
	}
	else {
		__asm__ __volatile__(
		"	mov.l	@%1,r0;"
		"	mov		%1,r4;"
		"1:;"
		"	mov		%2,r5;"
		"	trapa	#0xff;"
		"	bf		1b;"
		"	mov		r0,%0;"
		: "=&r" (__res)
		: "r" (__dst), "r" (__new)
		: "r0", "r4", "r5","cc","memory"
		);
	}
	return(__res);
}

/*
 *  code sequences for SMP atomic exchanging - used only for mutexes
 */
/* fast mutex */
static __inline__ unsigned __attribute__((__unused__))
_mux_smp_cmpxchg(volatile unsigned *__dst, unsigned __cmp, unsigned __src) {
	unsigned	__res;

	if (__cpu_flags & __SH_CPU_FLAG_MOVLICO) {
		__asm__ __volatile__(
		"	mov		%1,r1;"
		"0:	.word	0x0163;"	/* movli.l @r1,r0 */
		"	mov		r0,%0;"
		"	cmp/eq	r0,%2;"
		"	bf		1f;"
		"	mov		%3,r0;"
		"	.word	0x0173;"	/* movco.l r0,@r1 */
		"	bf		0b;"
		"1:;"
		: "=&r" (__res)
		: "r" (__dst), "r" (__cmp), "r" (__src)
		: "r0", "r1", "cc", "memory"
		);
	}
	else {
		__asm__ __volatile__(
		"mova	1f,r0;"
 		".balignw 4, 0x9;"
		"mov	#2f-1f,r1;"
		"add	#-1,r15;"
		"1:;"
		"mov.l	@%1,%0;"
		"cmp/eq	%0,%2;"
		"bf	2f;"
		"mov.l	%3,@%1;"
		"2:;"
		"add 	#1,r15;"
		: "=&r" (__res)
		: "r" (__dst), "r" (__cmp), "r" (__src)
		: "r0", "r1", "cc", "memory"
		);
	}
	return(__res);
}

static __inline__ unsigned __attribute__((__unused__))
_mux_smp_xchg(volatile unsigned *__dst, unsigned __new) {
	unsigned	__res;

	if (__cpu_flags & __SH_CPU_FLAG_MOVLICO) {
		__asm__ __volatile__(
		"	mov		%1,r1;"
		"0:	.word	0x0163;"	/* movli.l @r1,r0 */
		"	mov		r0,%0;"
		"	mov		%2,r0;"
		"	.word	0x0173;"	/* movco.l r0,@r1 */
		"	bf		0b;"
		: "=&r" (__res)
		: "r" (__dst), "r" (__new)
		: "r0", "r1", "cc", "memory"
		);
	}
	else {
		__asm__ __volatile__(
		"mova	1f,r0;"
 		".balignw 4, 0x9;"
		"mov	#2f-1f,r1;"
		"add	#-1,r15;"
		"1:;"
		"mov.l 	@%1,%0;"
		"mov.l	%2,@%1;"
		"2:;"
		"add 	#1,r15;"
		: "=&r" (__res)
		: "r" (__dst), "r" (__new)
		: "r0", "r1", "memory"
		);
	}
	return(__res);
}

#define __mutex_smp_cmpxchg(d, c, s)	_mux_smp_cmpxchg((d), (c), (s))
#define __mutex_smp_xchg(d, n)			_mux_smp_xchg((d), (n))

#endif
#ifdef __cplusplus
}
#endif
#endif

/* __SRCVERSION("smpxchg.h $Rev: 164019 $"); */
