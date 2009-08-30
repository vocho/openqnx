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
 *  sh/neutrino.h
 *

 */

#ifndef __NEUTRINO_H_INCLUDED
#error sh/neutrino.h should not be included directly.
#endif

__BEGIN_DECLS

#ifdef	__KERCPU_H_
extern volatile unsigned long	*__shadow_imask;

static __inline__ void __attribute__((__unused__))
(__inline_InterruptLock)(struct intrspin *__spin) {	

	__inline_InterruptDisable();
#if defined(VARIANT_smp)
{
	unsigned	__tmp;
 
	/* Refer to kernel.s for an explanation of why we don't go straight to
	 * the movli.l instruction, but instead test the water first.
	 */

	__asm__ __volatile__(
	"	mov		%1, r1;"
	"0:	mov.l	@r1, r0;"	/* grab old __spin->value  */
	"	tst		r0, r0;"	/* is it zero?			   */
	"	bf		0b;"		/* it is non-zero -- spin  */
	"	.word	0x0163;"	/* movli.l @r1, r0         */
	"	mov		r0, %0;"	/* old __spin->value       */
	"	mov		#1, r0;"	/* set __spin->value to 1  */
	"	.word	0x0173;"	/* movco.l r0, @r1         */
	"	bf		0b;"		/* retry if movco.l failed */
	"	tst		%0, %0;"
	"	bf		0b;"		/* old __spin->value != 0  */
	: "=&r" (__tmp)
	: "r" (&__spin->value)
	: "r0", "r1", "cc", "memory"
	);
}
#else
	/* aquire the spin lock */
	__spin->value = 1;
#endif
}

static __inline__ void __attribute__((__unused__))
(__inline_InterruptUnlock)(struct intrspin *__spin) {
	/*
	 * WARNING: SH4A SMP does not currently require a memory barrier here.
	 */
	__spin->value = 0;
	__inline_InterruptEnable();
}
#else
#include <sh/cpu.h>
extern volatile unsigned long	*__shadow_imask;

/*
 * Need to examine __cpu_flags to determine if we need SMP spin locks
 * We can't include <sys/syspage.h> due to name space pollution so we
 * hardcode the SH_CPU_FLAG_SMP value defined in <sh/syspage.h>
 */
extern unsigned					__cpu_flags;
#ifndef	__SH_CPU_FLAG_SMP
#define	__SH_CPU_FLAG_SMP	(1<<1)
#endif

static __inline__ void __attribute__((__unused__))
(__inline_InterruptEnable)(void) {
	unsigned	__tmp;

	__tmp = *__shadow_imask | ~SH_SR_IMASK;

	__asm__ __volatile__(
		"stc	sr,r0;"
		"and	%0,r0;"
		"ldc	r0,sr;"
		 :
		 : "r"(__tmp)
		 : "r0"
	);
}

static __inline__ void __attribute__((__unused__))
(__inline_InterruptDisable)(void) {

	__asm__ __volatile__(
		"stc	sr,r0;"
		"or		#0xf0,r0;"
		"ldc	r0,sr;"
		: 
		:
		: "r0"
	);
}

static __inline__ void __attribute__((__unused__))
(__inline_InterruptLock)(struct intrspin *__spin) {	

	__inline_InterruptDisable();

	/*
	 * Acquire spin lock if running on SMP platform
	 */
	if (__cpu_flags & __SH_CPU_FLAG_SMP) {
		unsigned	__tmp;

		__asm__ __volatile__(
		"0:	mov		%1, r1;"
		"	mov.l	@r1, r0;"
		"	tst		r0, r0;"
		"	bf		0b;"
		"	.word	0x0163;"	/* movli.l @r1, r0         */
		"	mov		r0, %0;"	/* old __spin->value       */
		"	mov		#1, r0;"	/* set __spin->value to 1  */
		"	.word	0x0173;"	/* movco.l r0, @r1         */
		"	bf		0b;"		/* retry if movco.l failed */
		"	tst		%0, %0;"
		"	bf		0b;"		/* old __spin->value != 0  */
		: "=&r" (__tmp)
		: "r" (&__spin->value)
		: "r0", "r1", "cc", "memory"
		);
	} else {
		__spin->value = 1;
	}
}

static __inline__ void __attribute__((__unused__))
(__inline_InterruptUnlock)(struct intrspin *__spin) {
	/*
	 * WARNING: SH4A SMP does not currently require a memory barrier here.
	 */
	__spin->value = 0;
	__inline_InterruptEnable();
}
#endif

static __inline__ unsigned
(__inline_InterruptStatus)(void) {
	unsigned __val;
	__asm__ __volatile__( "stc sr,%0" : "=&r" (__val));
	return (__val & 0xf0) ^ 0xf0;
}

/* Note that the 0xfe magic number comes from SH_KER_TRAP_PDBREAK
 * which is defined in ker/sh/kercpu.h
 */
static __inline__ void __attribute__((__unused__))
(__inline_DebugBreak)(void) {
	__asm__ __volatile__( "trapa	#0xfe" );
}

/* Note that the 0xfffd magic number generates an illegal instruction.
 * The kernel debugger intercepts illegal instruction traps to enter
 * the debugger.
 */
static __inline__ void __attribute__((__unused__))
(__inline_DebugKDBreak)(void) {
	__asm__ __volatile__( ".word	0xfffd" );
}

/* Display a message in the kernel debugger.
 * Note that the 0xfd magic number comes from SH_KER_TRAP_KDOUTPUT
 * which is defined in ker/sh/kercpu.h
 */
static __inline__ void __attribute__((__unused__))
(__inline_DebugKDOutput)(const char *__text, _CSTD size_t __len) {
	__asm__ __volatile__(
		"mov		%0,r4;"
		"mov 		%1,r5;"
		"trapa		#0xfd"
		:
		: "r" (__text), "r" (__len)
		: "r4", "r5"
	);
}

#define CLOCKCYCLES_INCR_BIT	32

#ifndef __KERCPU_H_
/* v_addr for read system timer */
#define __SH_VM_TIMERPAGE_ADDR	0x7bff8000	/* U0 */

#if defined(SH_VM_TIMERPAGE_ADDR) && (__SH_VM_TIMERPAGE_ADDR != SH_VM_TIMERPAGE_ADDR)
    #error "SH_VM_TIMERPAGE_ADDR != __SH_VM_TIMERPAGE_ADDR"
#endif

static __inline__ _Uint64t __attribute__((__unused__))
ClockCycles(void) {
	unsigned __lo, __tmp;
	unsigned __addr_tcor1, __addr_tcnt1;

	__addr_tcor1 = __SH_VM_TIMERPAGE_ADDR+SH_MMR_TMU_TCOR1_OFFSET;
	__addr_tcnt1 = __SH_VM_TIMERPAGE_ADDR+SH_MMR_TMU_TCNT1_OFFSET;

	__asm__ __volatile__(
		"mov.l	@%2,%0;"
		"mov.l	@%3,%1;"
		"sub	%1,%0;"
		: "=&r" (__lo), "=&r" (__tmp)
		: "r" (__addr_tcor1), "r" (__addr_tcnt1)
	);
	return (_Uint64t) __lo << CLOCKCYCLES_INCR_BIT;
}
#endif

__END_DECLS

/* __SRCVERSION("neutrino.h $Rev: 199716 $"); */
