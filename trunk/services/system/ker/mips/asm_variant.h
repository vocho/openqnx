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

#ifndef ASM_VARIANT_H
#define ASM_VARIANT_H

/*
 * asm_variant.h
 *	Kernel specific assembler stuff
 *  Generic version
 */

//
// If 'full' is 1 in GET_INKERNEL or ADJUST_INKERNEL, we haven't set up the
// kernel's gp register yet and have to use full 32-bit addresses to get
// at the inkernel variable.
//

#define GET_INKERNEL(reg,full)			\
		.if full; 						\
			lui reg,%hi(inkernel);		\
			lw reg,%lo(inkernel)(reg);	\
		.else; 							\
			lw reg,inkernel; 			\
		.endif

#define SET_INKERNEL(reg)		sw		reg,inkernel
	 
#if defined(VARIANT_smp)
	#define LDINS	ll
	#define STINS	sc
	#define	VERIFY_STORE(reg,lbl)	beq reg,zero,lbl; nop
	#define MEMBARRIER	sync
#else
	#define LDINS	lw
	#define STINS	sw
	#define	VERIFY_STORE(reg,lbl)
	#define MEMBARRIER	
#endif

#define ADJUST_INKERNEL(areg,nreg,oreg,adj,full)	\
		999:;								\
		.if full; 							\
			lui	areg,%hi(inkernel);			\
			LDINS oreg,%lo(inkernel)(areg);	\
			addiu nreg,oreg,adj;			\
			STINS nreg,%lo(inkernel)(areg);	\
			VERIFY_STORE(nreg,999b);		\
		.else;								\
			LDINS oreg,inkernel;			\
			addiu areg,oreg,adj;			\
			move  nreg,areg;				\
			STINS areg,inkernel;			\
			VERIFY_STORE(areg,999b);		\
		.endif;								\
		MEMBARRIER

#endif /* ASM_VARIANT_H */

/* __SRCVERSION("asm_variant.h $Rev: 153052 $"); */
