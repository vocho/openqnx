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
 *  ppc/context.h
 *

 */

#ifndef __PPC_CONTEXT_H_INCLUDED
#define __PPC_CONTEXT_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifdef __BIGREGS__
	typedef _Uint64t	ppcint;
#else
	typedef _Uint32t	ppcint;
#endif

typedef union {
	_Uint64t		u64;
	double			f;
} ppcfloat;

typedef union {
	_Uint64t		u64[2];
	_Uint32t		u32[4];
	_Uint8t			u8[16];
	float			f32[4];
} ppcvmx;

typedef struct ppc_cpu_registers {
	ppcint		gpr[32];
	ppcint		ctr;
	ppcint		lr;
	ppcint		msr;
	ppcint		iar;
	_Uint32t	cr;
	_Uint32t	xer;
	_Uint32t	ear;	/* not present on all chips */
	union {
		_Uint32t	mq;		/* only on the 601 */
		_Uint32t	usprg0;	/* only on the BookE's */
		_Uint32t	msr_u;	/* used by 32-bit procnto running on 64-bit implementations */
	}			u;
	union {
		_Uint32t	vrsave;		/* on Altivec CPU's, SPR256 */
		_Uint32t	spefscr;	/* on E500 CPU's, SPR512 */
	} 			u2;
} PPC_CPU_REGISTERS;

typedef struct ppc_fpu_registers {
	ppcfloat	fpr[32];
	_Uint32t	fpscr;
	_Uint32t	fpscr_val;
	/* load/store of fpscr is done through fprs, the real value is in [32,63] of a fpr.
		fpscr is the address for lfd, stfd. fpscr_val is the real value of fpscr */
} PPC_FPU_REGISTERS;

typedef struct ppc_vmx_registers {
	ppcvmx		vmxr[32];
	ppcvmx		vscr;
} PPC_VMX_REGISTERS;

typedef struct ppc_spe_registers {
	_Uint64t	acc;
	_Uint32t	gpr_hi[32];
} PPC_SPE_REGISTERS;

typedef struct ppc_alt_registers {
	PPC_VMX_REGISTERS	vmx;
} PPC_ALT_REGISTERS;

typedef struct {
	_Uint32t	id;
	_Uint32t	align;
	_Uint32t	pmc[6];
	_Uint32t	mmc[3];
	_Uint32t	sia;
	_Uint32t	spares[4];
} PPC_7450_PERFREGS;

typedef union {
	_Uint32t			id;
	PPC_7450_PERFREGS	mpc7450;
} PPC_PERFREGS;

#define PPC_GET_REGIP(regs)	 		((regs)->iar)
#define PPC_GET_REGSP(regs)			((regs)->gpr[1])
#define PPC_SET_REGIP(regs,v)		(((regs)->iar) = v)
#define PPC_SET_REGSP(regs,v)		(((regs)->gpr[1]) = v)

#endif /* __PPC_CONTEXT_H_INCLUDED */

/* __SRCVERSION("context.h $Rev: 153052 $"); */
