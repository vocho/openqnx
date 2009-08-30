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
 *  ppc/neutrino.h
 *

 */
#ifndef __NEUTRINO_H_INCLUDED
#error ppc/neutrino.h should not be included directly.
#endif

__BEGIN_DECLS

static __inline__ void __attribute__((__unused__))
(__inline_InterruptEnable)(void) {
	unsigned	__tmp;

	__asm__ __volatile__(
		"mfmsr	%0;"
		"ori	%0,%0,0x8000;"
		"mtmsr	%0;"
		: "=r" (__tmp)
	);
}

static __inline__ void __attribute__((__unused__))
(__inline_InterruptDisable)(void) {
	unsigned	__tmp;

	__asm__ __volatile__(
		"mfmsr	%0;"
		"rlwinm	%0,%0,0,17,15;"
		"mtmsr	%0;"
		: "=r" (__tmp)
	);
}

static __inline__ void __attribute__((__unused__))
(__inline_InterruptLock)(struct intrspin *__spin) {
	unsigned	__res;
	unsigned __lock_val;

	__inline_InterruptDisable();
	/* aquire the spin lock */
	__asm__ __volatile__(
		"	li	%1, 1;"
		"1:;"
		"	lwz		%0,0(%2);"
		"	mr.		%0,%0;"
		"bne-		1b;"
		"	lwarx	%0,0,%2;"
		"	dcbt	0,%2;"
		"	stwcx.	%1,0,%2;"
		"	bne-	1b;"
		"   mr.		%0,%0;"
		"bne-		1b;"
		"	isync"
		: "=&r" (__res), "=&r" (__lock_val)
		: "b" (__spin)
	);
}

static __inline__ void __attribute__((__unused__))
(__inline_InterruptUnlock)(struct intrspin *__spin) {
	__asm__ __volatile__ ( "sync" );
	__spin->value = 0;
	__inline_InterruptEnable();
}

static __inline__ unsigned
(__inline_InterruptStatus)(void) {
	unsigned __msr;
   	__asm__ __volatile__(	"mfmsr	%0"	: "=r" (__msr) );
   	return __msr & 0x8000;
}


static __inline__ void __attribute__((__unused__))
(__inline_DebugBreak)(void) {
	__asm__ __volatile__( "tw 31,%r0,%r0" );
}

static __inline__ void __attribute__((__unused__))
(__inline_DebugKDBreak)(void) {
	__asm__ __volatile__( "tw 31,%r1,%r1" );
}

	/* Display a message in the kernel debugger */
static __inline__ void __attribute__((__unused__))
(__inline_DebugKDOutput)(const char *__text, _CSTD size_t __len) {
	__asm__ __volatile__(
		"mr	%%r3,%0;"
		"mr %%r4,%1;"
		"tw	31,%%r2,%%r2"
		:
		: "r" (__text), "r" (__len)
		: "3", "4"
	);
}

#define CLOCKCYCLES_INCR_BIT	0

static __inline__ _Uint64t __attribute__((__unused__))
ClockCycles(void) {
	unsigned __lo;
	unsigned __hi;
	unsigned __tmp;
#if defined(__SLIB_DATA_INDIRECT) && !defined(__SLIB)
extern unsigned __get_cpu_flags(void);
#define __cpu_flags (__get_cpu_flags())
#else
        extern unsigned __cpu_flags;
#endif

	/*
	 * check to see if PPC_CPU_NO_MFTB is set. Can't use the macro
	 * directly since we don't want to pollute the namespace by including
	 * <ppc/syspage.h>
	 */
	if(__cpu_flags & (1 << 11)) {
		__asm__ __volatile__(
			"1:;"
			"  mfspr	%0,269;" /*TBU*/
			"  mfspr	%1,268;" /*TBL*/
			"  mfspr	%2,269;" /*TBU*/
			"  cmplw	%0,%2;"
			"bne- 1b;"
			: "=r" (__hi), "=r" (__lo), "=r" (__tmp) : : "cr0"
		);
	} else {	
		__asm__ __volatile__(
			"1:;"
			"  mftbu	%0;"
			"  mftb		%1;"
			"  mftbu	%2;"
			"  cmplw	%0,%2;"
			"bne- 1b;"
			: "=r" (__hi), "=r" (__lo), "=r" (__tmp) : : "cr0" 
		);
	}
		
	return (_Uint64t) __hi << 32 | (_Uint64t) __lo;
}

__END_DECLS

/* __SRCVERSION("neutrino.h $Rev: 200054 $"); */
