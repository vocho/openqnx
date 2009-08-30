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
 *  sh/context.h
 *

 */

#ifndef __SH_CONTEXT_H_INCLUDED
#define __SH_CONTEXT_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

typedef _Uint32t	shint;

typedef _Uint32t	shfloat;

typedef struct sh_cpu_registers {
	shint		gr[16];
	shint		sr;
	shint		pc;
/*	shint		dbr; */
	shint		gbr;
	shint		mach;
	shint		macl;
	shint		pr;
} SH_CPU_REGISTERS;

typedef struct sh_fpu_registers {
	shfloat		fpr_bank0[16];
	shfloat		fpr_bank1[16];
	_Uint32t	fpul;
	_Uint32t	fpscr;
} SH_FPU_REGISTERS;

typedef struct sh_alt_registers {
	/*
	 * There are no architecturally defined alt regs
	 */
	unsigned	__dummy;
} SH_ALT_REGISTERS;

typedef union {
	_Uint32t	id;
} SH_PERFREGS;

#define SH_GET_REGIP(regs)			((regs)->pc)
#define SH_GET_REGSP(regs)			((regs)->gr[15])
#define SH_SET_REGIP(regs,v)		(((regs)->pc) = v)
#define SH_SET_REGSP(regs,v)		(((regs)->gr[15]) = v)

#endif /* __SH_CONTEXT_H_INCLUDED */

/* __SRCVERSION("context.h $Rev: 153052 $"); */
