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
 *  arm/context.h
 *

 */

#ifndef __ARM_CONTEXT_H_INCLUDED
#define __ARM_CONTEXT_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

typedef struct arm_cpu_registers {
	_Uint32t	gpr[16];
	_Uint32t	spsr;
} ARM_CPU_REGISTERS;

/*
 * Register names
 */
#define	ARM_REG_R0		0
#define	ARM_REG_R1		1
#define	ARM_REG_R2		2
#define	ARM_REG_R3		3
#define	ARM_REG_R4		4
#define	ARM_REG_R5		5
#define	ARM_REG_R6		6
#define	ARM_REG_R7		7
#define	ARM_REG_R8		8
#define	ARM_REG_R9		9
#define	ARM_REG_R10		10
#define	ARM_REG_R11		11
#define	ARM_REG_R12		12
#define	ARM_REG_R13		13
#define	ARM_REG_R14		14
#define	ARM_REG_R15		15
#define	ARM_REG_SPSR	16

/*
 * Register name aliases
 */
#define	ARM_REG_FP		11
#define	ARM_REG_IP		12
#define	ARM_REG_SP		13
#define	ARM_REG_LR		14
#define	ARM_REG_PC		15

/*
 * Register manipulation
 */
#define ARM_GET_REGIP(regs)			((regs)->gpr[ARM_REG_PC])
#define ARM_GET_REGSP(regs)			((regs)->gpr[ARM_REG_SP])
#define ARM_SET_REGIP(regs,v)		((regs)->gpr[ARM_REG_PC] = v)
#define ARM_SET_REGSP(regs,v)		((regs)->gpr[ARM_REG_SP] = v)

union vfpv2 {
	_Uint32t	S[32];
	_Uint64t	D[16];
};

union vfpv3 {
	_Uint32t	S[32];
	_Uint64t	D[32];
};

typedef struct arm_fpu_registers {
	union {

		struct vfp {		/* VFP register context */
			union {
				_Uint64t	X[32];	/* largest register file size */
				union vfpv2	v2;
				union vfpv3	v3;
			} reg;
			_Uint32t	fpscr;
			_Uint32t	fpexc;
			_Uint32t	fpinst;		/* implementation specific register */
			_Uint32t	fpinst2;	/* implementation specific register */
		} vfp;

	} un;
} ARM_FPU_REGISTERS;

typedef struct arm_fpemu_context {
	ARM_CPU_REGISTERS	reg;

	/*
	 * Additional coprocessor specific state supplied by the kernel
	 * for software handling required even if hardware is present
	 */
	union {

		struct fpemu_vfp {			/* VFP registers passed to fpemu */
			_Uint32t	fpexc;
			_Uint32t	fpinst;
			_Uint32t	fpinst2;
		} vfp;

	} un;
} ARM_FPEMU_CONTEXT;

typedef struct arm_alt_registers {
	union {
		struct xscale_cp0 {
			unsigned	acc0_lo;
			unsigned	acc0_hi;
		} xscale;
	} un;
} ARM_ALT_REGISTERS;

typedef union {
	_Uint32t	id;
} ARM_PERFREGS;

#endif /* __ARM_CONTEXT_H_INCLUDED */

/* __SRCVERSION("context.h $Rev: 160127 $"); */
