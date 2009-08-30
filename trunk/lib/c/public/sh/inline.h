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
 *  sh/inline.h
 *

 */

#ifndef _SH_INLINE_H_INCLUDED
#define _SH_INLINE_H_INCLUDED

/*
    some handy pragma's for low-level work: 
    Most of the inline fuctions can only be used by startup & kernel
*/

#ifndef _SH_INOUT_INCLUDED
#include <sh/inout.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include <sh/cpu.h>
#include <sh/ccn.h>

__BEGIN_DECLS

	/* load up the stack pointer */
#define ldesp(stack) __asm__ __volatile__( "mov %0,r15" : : "r" (stack))

/* flush whole icache */
#define icache_flush(addr) {						\
		unsigned t1, a1, c1, amask;					\
		amask = 0xa0000000;							\
		a1 = SH_MMR_CCN_CCR;						\
		c1 = SH_CCN_CCR_ICI; 						\
		__asm__ __volatile__(						\
		"ocbwb 	@%1;" 								\
		"mov.l	@%2,%0;"							\
		"or		%3,%0;"								\
		"mova	1f,r0;"								\
		"bra	2f;"								\
		"nop;"										\
		".align 2;"									\
		"1:;"										\
		"mov.l	%0,@%2;"							\
		"nop;nop;nop;nop;"							\
		"nop;nop;nop;nop;"							\
		"rts;"										\
		"nop;"										\
		"2:;"										\
		"or	%4,r0;"									\
		"jsr @r0;"									\
		"nop;"										\
		 :"=&r" (t1)								\
		 :"r" (addr), "r" (a1), "r" (c1), "r"(amask)\
		 :"r0", "pr", "memory");}

/* invalidate one dcache line */
#define dcache_invalidate(addr) {				\
		__asm__ __volatile__(							\
		"ocbi 	@%0;" 							\
		 : : "r" (addr));}
		 
/* flush and invalidate one dcache line */
#define dcache_flush(addr) {					\
		__asm__ __volatile__(							\
		"ocbp 	@%0;" 							\
		 : : "r" (addr));}
		 
#define	get_sr()		({unsigned __val;__asm__ __volatile__( "stc sr,%0" : "=&r" (__val));__val;})
#define	get_pr()		({unsigned __val;__asm__ __volatile__( "sts pr,%0" : "=&r" (__val));__val;})
#define	get_vbr()		({unsigned __val;__asm__ __volatile__( "stc vbr,%0" : "=&r" (__val));__val;})
#define	get_fpscr()		({unsigned __val;__asm__ __volatile__( "sts fpscr,%0" : "=&r" (__val));__val;})
#define	set_sr(val)		__asm__ __volatile__( "ldc %0,sr" : : "r" (val))
#define	set_vbr(val)	__asm__ __volatile__( "ldc %0,vbr" : : "r" (val))
#define	set_gbr(val)	__asm__ __volatile__( "ldc %0,gbr" : : "r" (val))
#define	set_fpscr(val)	__asm__ __volatile__( "lds %0,fpscr" : : "r" (val))
#define	load_tlb()		__asm__ __volatile__( "ldtlb" )

__END_DECLS

#endif

/* __SRCVERSION("inline.h $Rev: 153052 $"); */
