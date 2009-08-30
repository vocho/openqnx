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
 *  x86/context.h
 *

 */

#ifndef __X86_CONTEXT_H_INCLUDED
#define __X86_CONTEXT_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

typedef struct x86_cpu_registers {
#ifdef __SEGMENTS__
	_Uint32t	gs, fs;
	_Uint32t	es, ds;
#endif
	_Uint32t	edi, esi, ebp, exx, ebx, edx, ecx, eax;
	_Uint32t	eip, cs, efl;
	_Uint32t	esp, ss;
} X86_CPU_REGISTERS;

typedef struct fsave_area {
	_Uint32t	fpu_control_word;
	_Uint32t	fpu_status_word;
	_Uint32t	fpu_tag_word;
	_Uint32t	fpu_ip;
	_Uint32t	fpu_cs;
	_Uint32t	fpu_op;
	_Uint32t	fpu_ds;
	_Uint8t		st_regs[80]; /* each register is 10 bytes! */
} X86_FSAVE_REGISTERS;

typedef struct fxsave_area {
	_Uint16t	fpu_control_word;
	_Uint16t	fpu_status_word;
	_Uint16t	fpu_tag_word;
	_Uint16t	fpu_operand;
	_Uint32t	fpu_ip;
	_Uint32t	fpu_cs;
	_Uint32t	fpu_op;
	_Uint32t	fpu_ds;
	_Uint32t	mxcsr;
	_Uint32t	reserved;
	_Uint8t		st_regs[128];
	_Uint8t		xmm_regs[128];
	_Uint8t		reserved2[224];
} X86_FXSAVE_REGISTERS;

typedef union x86_fpu_registers {
	X86_FSAVE_REGISTERS		fsave_area;
	X86_FXSAVE_REGISTERS	fxsave_area;
	_Uint8t					data[512];		/* Needs to be this big for the emulator. */
} X86_FPU_REGISTERS;

typedef struct x86_alt_registers {
	/*
	 * There are no architecturally defined alt regs
	 */
	unsigned	__dummy;
} X86_ALT_REGISTERS;

#define X86_GET_REGIP(regs)			((regs)->eip)
#define X86_GET_REGSP(regs)			((regs)->esp)
#define X86_SET_REGIP(regs,v)		(((regs)->eip) = v)
#define X86_SET_REGSP(regs,v)		(((regs)->esp) = v)

/* Performance Counters */

/* Pentium Class */
typedef struct x86_pentium_perfregs {
	_Uint32t	id; /* unique part id */

	_Uint32t	cesr; 
	_Uint64t	ctr0; /* 40 bit */
	_Uint64t	ctr1; /* 40 bit */
} X86_PENTIUM_PERFREGS;

/* Pentium 6 Class */
typedef struct x86_p6family_perfregs {
	_Uint32t	id; /* unique part id */
	_Uint32t	spare0;	/* make sure things are aligned */

	_Uint64t	PerfEvtSel0;
	_Uint64t	PerfEvtSel1; 
	_Uint64t	PerfCtr0; /* 40 bit */
	_Uint64t	PerfCtr1; /* 40 bit */
} X86_P6FAMILY_PERFREGS;

/* Xeon Class */
#define X86_P4XEON_BPU		4
#define X86_P4XEON_MS		4
#define X86_P4XEON_FLAME	4
#define X86_P4XEON_IQ		6

typedef struct x86_pentium4xeon_perfregs {
	_Uint32t	id; 	/* unique part id */
	_Uint32t	spare0; /* Align */
	
	_Uint64t	bsu_escr[2];
	_Uint64t	fsb_escr[2];
	_Uint64t	mob_escr[2];
	_Uint64t	pmh_escr[2];
	_Uint64t	bpu_escr[2];
	_Uint64t	is_escr[2];
	_Uint64t	itlb_escr[2];
	_Uint64t	ix_escr[2];
	_Uint64t	ms_escr[2];
	_Uint64t	tbpu_escr[2];
	_Uint64t	tc_escr[2];
	_Uint64t	firm_escr[2];
	_Uint64t	flame_escr[2];
	_Uint64t	dac_escr[2];
	_Uint64t	saat_escr[2];
	_Uint64t	u2l_escr[2];		
	_Uint64t	cru_escr[6];
	_Uint64t	iq_escr[2];
	_Uint64t	rat_escr[2];
	_Uint64t	ssu_escr0;
	_Uint64t	alf_escr[2];
	

	_Uint64t	bpu_cccr[X86_P4XEON_BPU];
	_Uint64t	ms_cccr[X86_P4XEON_MS];
	_Uint64t	flame_cccr[X86_P4XEON_FLAME];
	_Uint64t	iq_cccr[X86_P4XEON_IQ];
	
	
	_Uint64t	bpu_counter[X86_P4XEON_BPU];
	_Uint64t	ms_counter[X86_P4XEON_MS];
	_Uint64t	flame_counter[X86_P4XEON_FLAME];
	_Uint64t	iq_counter[X86_P4XEON_IQ];


	/* Extra space for expansion. */
	_Uint64t	spare1[10];	

} X86_PENTIUM4XEON_PERFREGS;

typedef union {
	_Uint32t	id;
	X86_PENTIUM_PERFREGS pentium;
	X86_P6FAMILY_PERFREGS p6family;
	X86_PENTIUM4XEON_PERFREGS pentium4xeon;
} X86_PERFREGS;

#endif /* __X86_CONTEXT_H_INCLUDED */

/* __SRCVERSION("context.h $Rev: 198895 $"); */
