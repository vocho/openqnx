/*
 * $QNXLicenseC: 
 * Copyright 2007, QNX Software Systems.  
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */



/*
 *  arm/neutrino.h
 *

 */

#ifndef __NEUTRINO_H_INCLUDED
#error arm/neutrino.h should not be included directly.
#endif

__BEGIN_DECLS

static __inline__ void __attribute__((__unused__))
__inline_InterruptEnable(void)
{
	unsigned	__tmp;

	__asm__ __volatile__(
		"mrs	%0, cpsr;"
		"bic	%0, %0, #0xc0;"
		"msr	cpsr, %0;"
		: "=r" (__tmp)
	);
}

static __inline__ void __attribute__((__unused__))
__inline_InterruptDisable(void)
{
	unsigned	__tmp;

	__asm__ __volatile__(
		"mrs	%0, cpsr;"
		"orr	%0, %0, #0xc0;"
		"msr	cpsr, %0;"
		: "=r" (__tmp)
	);
}

static __inline__ void __attribute__((__unused__))
__inline_InterruptLock(struct intrspin *__spin)
{
	volatile unsigned	tmp;

	__inline_InterruptDisable();
	__asm__ __volatile__(
		"	mcr	p15, 0, %3, c7, c10, 4;"
		"0:	ldr	%0, [%2];"
		"	teq	%0, #0;"
		"	bne	0b;"
		"	swp	%0, %1, [%2];"
		"	teq	%0, #0;"
		"	bne	0b;"
		"	mcr	p15, 0, %3, c7, c10, 4;"
		: "=&r" (tmp)
		: "r" (1), "r" (&__spin->value), "r" (0)
	);
}

static __inline__ void __attribute__((__unused__))
__inline_InterruptUnlock(struct intrspin *__spin)
{
	__asm__ __volatile__(
		"mcr	p15, 0, %0, c7, c10, 4;"
		: : "r" (0)
	);
	__spin->value = 0;
	__inline_InterruptEnable();
}

static __inline__ unsigned
(__inline_InterruptStatus)(void) {
	unsigned __val;
	__asm__ __volatile__( "mrs %0, cpsr" : "=&r" (__val));
	return (__val & 0xc0)^0xc0;
}

static __inline__ void __attribute__((__unused__))
__inline_DebugBreak(void)
{
	/*
	 * WARNING: must match the breakpoint instruction used by gdb.
	 */
	__asm__ __volatile__(
		"	.word	0xe7ffdefe"
	);
}

static __inline__ void __attribute__((__unused__))
__inline_DebugKDBreak(void)
{
	__asm__ __volatile__(
		"	.word	0xe7ffdeff"
	);
}


static __inline__ void __attribute__((__unused__))
__inline_DebugKDOutput(const char *__text, _CSTD size_t __len)
{
	__asm__ __volatile__(
		"	mov		r0, %0;"
		"	mov		r1, %1;"
		"	.word	0xe7ffffff"
		:
		: "r" (__text), "r" (__len)
		: "r0", "r1"
	);
}

/*
 * ClockCycles must be emulated
 */
extern _Uint64t		ClockCycles();

#define CLOCKCYCLES_INCR_BIT	0

__END_DECLS

/* __SRCVERSION("neutrino.h $Rev: 160127 $"); */
