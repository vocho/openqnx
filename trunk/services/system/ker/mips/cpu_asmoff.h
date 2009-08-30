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

VALUE( REG_FPCR31, 	offsetof(MIPS_FPU_REGISTERS, fpcr31) );

VALUE( FLTCACHERR,		FLTCACHERR);
VALUE( FLTUTLBREFILL,	FLTUTLBREFILL);
VALUE( FLTXTLBREFILL,	FLTXTLBREFILL);

struct mips_context {
	#ifdef VARIANT_tx79
	   MIPS_ALT_REGISTERS	hi;
	#endif
   MIPS_CPU_REGISTERS		lo;
};
VALUE(MIPS_CONTEXT_SIZE, 		sizeof(struct mips_context));
VALUE(MIPS_CONTEXT_LO_START,	offsetof(struct mips_context, lo));
VALUE(MIPSTX79_CONTEXT_GPR_HI,	offsetof(MIPS_ALT_REGISTERS, un.tx79.gpr_hi));
VALUE(MIPSTX79_CONTEXT_LO1,		offsetof(MIPS_ALT_REGISTERS, un.tx79.lo1));
VALUE(MIPSTX79_CONTEXT_HI1,		offsetof(MIPS_ALT_REGISTERS, un.tx79.hi1));
VALUE(MIPSTX79_CONTEXT_SA,		offsetof(MIPS_ALT_REGISTERS, un.tx79.sa));

VALUE(SB1_PERF_CONTROL,			offsetof(MIPS_PERFREGS, sb1.EventControl )); 
VALUE(SB1_PERF_COUNTER,			offsetof(MIPS_PERFREGS, sb1.EventCounter )); 
VALUE(SB1_PERF_PTR,				offsetof(MIPS_PERFREGS, sb1.PTR )); 

VALUE(FI_CPU_REGS, 				offsetof(struct fault_info, cpu.regs));

/* __SRCVERSION("cpu_asmoff.h $Rev: 153052 $"); */
