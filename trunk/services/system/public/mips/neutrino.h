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
 *  mips/neutrino.h
 *

 */
#ifndef __NEUTRINO_H_INCLUDED
#error mips/neutrino.h should not be included directly.
#endif

#ifndef __MIPS_NEUTRINO_H_INCLUDED
#define __MIPS_NEUTRINO_H_INCLUDED

__BEGIN_DECLS

/*
 * __inline_InterruptDisable()
 *	Disable the maskable processor interrupts.
 */
__inline__ static void __attribute__((__unused__))
(__inline_InterruptDisable)(void) {
	__asm__ __volatile__ (
		" .set noreorder ;"
		" .set noat ;"

		" mfc0 $1,$12 ;"
		"  nop ;"
		" ori $1,$1,1 ;"
		" xori $1,$1,1 ;"
		" mtc0 $1,$12 ;"
		"  nop; nop; nop; nop; "

		" .set  at ;"
		" .set reorder ;"

		:		/* no outputs */
		:		/* no inputs */
	);
}

/*
 * __inline_InterruptEnable()
 *	Enable maskable interrupts
 */
__inline__ static void __attribute__((__unused__))
(__inline_InterruptEnable)(void) {
 	unsigned	t1, t2;
 	/*
 	 * __shadow_imask is to deal with the mis-feature of putting
 	 * interrupt level masks in same register as IE bit with no
 	 * way of atomically modifying the latter. We need it to avoid
 	 * leaving levels in the wrong masking state if an interrupt
 	 * comes in after the status register is moved into AT.
 	 */
 	extern volatile unsigned long *__shadow_imask;
 
	__asm__ __volatile__ (
		" .set noreorder ;"
		" .set noat ;"

		" mfc0 $1, $12 ;"
 		"   li %0,~0xff00 ;"
		"  ori $1, $1, 1 ;"
 		"  and $1, %0 ;"
 		"   lw %1,(%2) ;"
 		"   or $1, %1 ;"
		" mtc0 $1, $12 ;"
		"  nop; nop; nop; nop; "

		" .set at ;"
		" .set reorder ;"

 		: "=&r" (t1), "=r" (t2)
 		: "r" (__shadow_imask)
	);
}

/*
 * __inline_InterruptLock(struct intrspin *__spin)
 *	Disable interrupts, handling SMP
 */
__inline__ static void __attribute__((__unused__))
(__inline_InterruptLock)(struct intrspin *__spin) {
	unsigned	__result;

	__inline_InterruptDisable();
	/* aquire the spinlock */
	__asm__ __volatile__ (
		" .set noreorder ;"
		" .set mips2	 ;"
		" .set noat		 ;"

		"	li	$1,1	 ;"
		"1:	ll	%0,0(%1) ;"
		"	sc	$1,0(%1) ;"
		"	beq	$1,$0,1b ;"
		"	 li	$1,1	 ;"
		"	bne %0,$0,1b ;"
		"    nop         ;"

		" .set at		 ;"
		" .set reorder   ;"
		: "=&r" (__result)
		: "r" (__spin));
}

/*
 * __inline_InterruptUnlock(struct inspin *)
 *	Re-enable interrupts, handling SMP
 */
__inline__ static void __attribute__((__unused__))
(__inline_InterruptUnlock)(struct intrspin *__spin) {
	__asm__ __volatile__ ("sync");
	__spin->value = 0;
	__inline_InterruptEnable();
}

/*
 * __inline_InterruptStatus(void)
 */
__inline__ static unsigned
(__inline_InterruptStatus)(void) {
	_Uint32t	__sreg;				
		__asm__ __volatile__ (		
			".set noreorder		;"	
			"mfc0 	%0, $12 	;"	
			"sll	$0,$0,1		;"	 /* ssnop */
			".set reorder		;"	
			: "=r" (__sreg));
	return __sreg & 0x1;
}

/*
 * __inline_DebugBreak()
 *	Request break out to the process debugger
 */
static __inline__ void __attribute__((__unused__))
(__inline_DebugBreak)(void) {
	__asm__ __volatile__(".set noreorder; nop; break 10; nop; .set reorder;");
}

/*
 * __inline_DebugKDBreak()
 *	Request break out to the kernel debugger
 */
static __inline__ void __attribute__((__unused__))
(__inline_DebugKDBreak)(void) {
	__asm__ __volatile__ (".set noreorder; nop; break 9; .set reorder;");
}

/*
 * __inline_DebugKDOutput()
 *	Get the kernel debugger to display the message
 *
 * The handling for "break 8" knows that the arguments were passed
 * in a0/a1.
 */
static __inline__ void __attribute__((__unused__))
(__inline_DebugKDOutput)(const char *__text, _CSTD size_t __len) {
	__asm__ __volatile__(".set noreorder;"
		"move $4,%0;"
		"move $5,%1;"
		"break 8;"
		".set reorder;"
		: /* No outputs */
		: "r" (__text), "r" (__len));
}

#define CLOCKCYCLES_INCR_BIT	32

static __inline__ _Uint64t __attribute__((__unused__))
ClockCycles(void) {
	unsigned long __lo;

	__asm__ __volatile__ ("mfc0	%0,$9 ; nop"
		: "=r" (__lo)
		: /* No inputs */
		);

	return (_Uint64t)__lo << CLOCKCYCLES_INCR_BIT;
}

__END_DECLS

#endif /* __MIPS_NEUTRINO_H_INCLUDED */

/* __SRCVERSION("neutrino.h $Rev: 174980 $"); */
