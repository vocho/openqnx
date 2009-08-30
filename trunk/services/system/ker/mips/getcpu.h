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

#ifndef GETCPU_H
#define GETCPU_H

#if defined(VARIANT_smp)
//
// Remember that if the GETCPU_? macros change, you need to update the
// code that implements INTR_GENFLAG_LOAD_CPUNUM in interrupt.c
//
	#if defined(VARIANT_r3k) || defined(VARIANT_32) || defined(VARIANT_tx79)
		#error SMP needs a 64 bit XCONTEXT reg
	#endif

	// Store the CPU number in the upper 7 bits of the XCONTEXT register.
	// The minimum number of bits in the PTEBase field of the register
	// is 9. That allows us to have a maximum 'scale' factor of 2 without
	// having to bother clearing the low order bits of 'reg'.

	// Note that if we don't mind losing one more bit and restricting the
	// maximum number of CPU's to 64, we could store the number in the
	// upper part of the CONTEXT register, since we've got 8 bits there
	// (except for NEC 41xx's, but I don't think we'll be running
	// SMP on that :-). That would let us do SMP on MIPS32 architectures.

	#define XCTX_SHIFT_AMOUNT	(64-7)

	#define GETCPU_1(reg, scale)	dmfc0	reg,CP0_XCONTEXT
	#define GETCPU_2(reg, scale)	dsrl	reg,XCTX_SHIFT_AMOUNT-(scale)
	#define SETCPU(reg) \
		.set mips3; 	\
		.ifnc reg,zero;	\
			dsll reg,XCTX_SHIFT_AMOUNT; \
		.endif;			\
		dmtc0 reg,CP0_XCONTEXT
#else
	#define GETCPU_1(reg, scale)
	#define GETCPU_2(reg, scale)
	#define SETCPU(reg)
#endif

#endif

/* __SRCVERSION("getcpu.h $Rev: 153052 $"); */
