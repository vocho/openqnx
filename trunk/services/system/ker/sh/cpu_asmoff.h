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

#include <sh/cpu.h>
#include <sh/sh4acpu.h>

VALUE(BUS_ADRERR,			BUS_ADRERR);
VALUE(REG_GR,	 			offsetof(SH_CPU_REGISTERS, gr[0]));
VALUE(REG_IAR,			 	offsetof(SH_CPU_REGISTERS, pc));
VALUE(REG_R0,				offsetof(SH_CPU_REGISTERS, gr[0]));
VALUE(REG_R4,				offsetof(SH_CPU_REGISTERS, gr[4]));
VALUE(REG_R5,				offsetof(SH_CPU_REGISTERS, gr[5]));
VALUE(REG_R15,				offsetof(SH_CPU_REGISTERS, gr[15]));
VALUE(REG_PC,				offsetof(SH_CPU_REGISTERS, pc));
VALUE(REG_SR,				offsetof(SH_CPU_REGISTERS, sr));
VALUE(REG_FPSCR,			offsetof(SH_FPU_REGISTERS, fpscr));
VALUE(KERERR_SKIPAHEAD,		KERERR_SKIPAHEAD);
VALUE(SH_SR_RB,				SH_SR_RB);
VALUE(SH_SR_BL,				SH_SR_BL);
VALUE(SH_SR_FD,				SH_SR_FD);
VALUE(SH_SR_IMASK,			SH_SR_IMASK);
VALUE(SH_FPU_REG_SIZE,		sizeof(SH_FPU_REGISTERS));
VALUE(SH_FPSCR_PR,			SH_FPSCR_PR);
VALUE(SH_FPSCR_DN,			SH_FPSCR_DN);
VALUE(SH_FPSCR_SZ,			SH_FPSCR_SZ);
VALUE(SH_FPSCR_FR,			SH_FPSCR_FR);
VALUE(SH_MMR_CCN_EXPEVT,	SH_MMR_CCN_EXPEVT);
VALUE(SH_MMR_CCN_TRA,		SH_MMR_CCN_TRA);
VALUE(SH_MMR_CCN_MMUCR_OFF,	SH_MMR_CCN_MMUCR_OFF);
VALUE(SH_MMR_CCN_TEA,		SH_MMR_CCN_TEA);
VALUE(SH_MMR_CCN_PTEH,		SH_MMR_CCN_PTEH);
VALUE(SH_MMR_CCN_TEA_OFF,	SH_MMR_CCN_TEA_OFF);
VALUE(SH_MMR_CCN_TTB_OFF,	SH_MMR_CCN_TTB_OFF);
VALUE(SH_MMR_CCN_PTEL_OFF,	SH_MMR_CCN_PTEL_OFF);		
VALUE(SH_MMR_CCN_PTEH_OFF,	SH_MMR_CCN_PTEH_OFF);
VALUE(SH_MMR_CCN_PTEA_OFF,	SH_MMR_CCN_PTEA_OFF);
VALUE(SH_CCN_BASE,			SH_CCN_BASE);
VALUE(SH_CCN_PTEL_V,		SH_CCN_PTEL_V);
VALUE(SH_CCN_PTEL_D,		SH_CCN_PTEL_D);
VALUE(VM_FAULT_INKERNEL,	VM_FAULT_INKERNEL);
VALUE(VM_FAULT_WRITE,		VM_FAULT_WRITE);
VALUE(VM_MSG_XFER_START,	VM_MSG_XFER_START);
VALUE(VM_USER_SPACE_BOUNDRY,VM_USER_SPACE_BOUNDRY);
VALUE(SH_KER_TRAP_BOUNDARY,	SH_KER_TRAP_BOUNDARY);
VALUE(SH_EXC_CODE_TRAP,		SH_EXC_CODE_TRAP);
VALUE(SH_EXC_CODE_GILLINST,	SH_EXC_CODE_GILLINST);
VALUE(SH_EXC_CODE_SILLINST,	SH_EXC_CODE_SILLINST);
VALUE(SH4A_MMR_CPIDR,		SH4A_MMR_CPIDR);
VALUE(SIGCODE_FLAGS_MASK,	SIGCODE_FLAGS_MASK);
VALUE(FI_CPU_FLAGS,			offsetof(struct fault_info, cpu.flags));

/* __SRCVERSION("cpu_asmoff.h $Rev: 170865 $"); */
