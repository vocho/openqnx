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

#include <mips/context.h>
/*
 * Deal with load and store issues for MIPS32/MIPS64/TX79/R3K architectures
 */
 
#if defined(VARIANT_tx79)

	#define MIPSARCH		mips3

	#define REG_POS_BASE	0

	#define MV_INSTR(instr)	instr
	#define LS_INSTR(ls)	ls##d

#elif defined(VARIANT_32) || defined(VARIANT_r3k)

	#define MIPSARCH		mips2

	#define REG_POS_BASE	(MIPS_REGS_LOW_WORD*4)

	#define MV_INSTR(instr)	instr
	#define LS_INSTR(ls)	ls##w

#else

	#define MIPSARCH		mips3

	#define REG_POS_BASE	0

	#define MV_INSTR(instr)	d##instr
	#define LS_INSTR(ls)	ls##d

#endif
	
#define RESTORE_ONE_REG(dst, src, adj, base)	\
	LS_INSTR(l)	dst, MIPS_AREG(MIPS_REG_##src)+REG_POS_BASE+(adj)(base)

#define SAVE_ONE_REG(src, dst, adj, base)	\
	LS_INSTR(s)	src,MIPS_AREG(MIPS_REG_##dst)+REG_POS_BASE+(adj)(base)

//To fix some funkiness with C macro expansion & token concatenation	
#define MIPS_REG_MYAT	MIPS_REG_AT

/* __SRCVERSION("loadstore.h $Rev: 153052 $"); */
