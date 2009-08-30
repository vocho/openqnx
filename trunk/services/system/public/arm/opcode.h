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
 *  arm/opcode.h
 *

 */

#ifndef __ARM_OPCODE_H_INCLUDED
#define __ARM_OPCODE_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __ARM_CONTEXT_H_INCLUDED
#include _NTO_HDR_(arm/context.h)
#endif

/*
 * ARM condition codes
 */
#define	ARM_COND(x)		((unsigned)(x) >> 28)
#define	ARM_COND_EQ		0
#define	ARM_COND_NE		1
#define	ARM_COND_CS		2
#define	ARM_COND_HS		2
#define	ARM_COND_CC		3
#define	ARM_COND_LO		3
#define	ARM_COND_MI		4
#define	ARM_COND_PL		5
#define	ARM_COND_VS		6
#define	ARM_COND_VC		7
#define	ARM_COND_HI		8
#define	ARM_COND_LS		9
#define	ARM_COND_GE		10
#define	ARM_COND_LT		11
#define	ARM_COND_GT		12
#define	ARM_COND_LE		13
#define	ARM_COND_AL		14
#define	ARM_COND_NV		15

/*
 * Instruction masks.
 * WARNING: these may change for ARMv5
 */
#define	ARM_OPC(x)			(((x) >> 24) & 15)	/* coarse opcode mask */
#define	ARM_IS_BRANCH(x)	(((x) & 0x0e000000) == 0x0a000000)

/*
 * register operands
 */
#define	ARM_RM(x)		((x) & 15)
#define	ARM_RS(x)		(((x) >> 8) & 15)
#define	ARM_RD(x)		(((x) >> 12) & 15)
#define	ARM_RN(x)		(((x) >> 16) & 15)

/*
 * shifter operands
 */
#define	ARM_SHIFT_IMM(x)	(((x) >> 7) & 31)
#define	ARM_SHIFT(x)		(((x) >> 5) & 3)
#define	ARM_SHIFT_LSL		0
#define	ARM_SHIFT_LSR		1
#define	ARM_SHIFT_ASR		2
#define	ARM_SHIFT_ROR		3

/*
 * Fields relevant to load/store instructions
 */
#define	ARM_LS_L		(1 << 20)	/* L bit for all load/store instructions */
#define	ARM_LS_W		(1 << 21)	/* W bit for all load/store instructions */
#define	ARM_LS_U		(1 << 23)	/* U bit for all load/store instructions */
#define	ARM_LS_P		(1 << 24)	/* P bit for all load/store instructions */

#define	ARM_LSR_B		(1 << 22)		/* B bit for ldr/str */
#define	ARM_LSR_IMM(x)	((x) & 0xfff)	/* ldr/str immediate offset */

#define	ARM_LSM_S		(1 << 22)		/* S bit for ldm/stm */
#define	ARM_LSM_RL(x)	((x) & 0xffff)	/* ldm/stm register list */

#define	ARM_LSH_I		(1 << 22)			/* I bit for ldrh etc. */
#define	ARM_LSH_IMMH(x)	(((x) >> 8) & 15)	/* ldrh etc. hi offset */
#define	ARM_LSH_IMML(x)	((x) & 15)			/* ldrh etc. lo offset */

#define	ARM_LSC_N		(1 << 22)		/* N bit for ldc/stc */
#define	ARM_LSC_IMM(x)	((x) & 255)		/* ldc/stc immediate offset */

/*
 * Fields relevant to data processing instructions
 */
#define	ARM_DP_S		(1 << 20)			/* S bit */
#define	ARM_DP_A		(1 << 21)			/* A bit for multiply */
#define	ARM_DP_ROT(x)	(((x) >> 8) & 15)	/* rotate amount */
#define	ARM_DP_IMM(x)	((x) & 255)			/* immediate value */

/*
 * Fields relevant to status register instructions
 */
#define	ARM_MSR_R		(1 << 22)	/* R bit for msr/mrs */

/*
 * Fields relevant to branch instructions
 */
#define	ARM_B_L			(1 << 24)			/* L bit for b/bl */
#define	ARM_B_OFFSET(x)	((x) & 0x00ffffff)	/* b/bl offset */

/*
 * Fields relevant to coprocessor instructions
 */
#define	ARM_CPNUM(x)	(((x) >> 8) & 15)
#define	ARM_CRM(x)		ARM_RM(x)
#define	ARM_CRD(x)		ARM_RD(x)
#define	ARM_CRN(x)		ARM_RN(x)

/*
 * Special instruction opcodes
 */
#define	OPCODE_BREAK	0xe7ffdefe
#define	OPCODE_KDBREAK	0xe7ffdeff
#define	OPCODE_KDOUTPUT	0xe7ffffff
#define	OPCODE_DIV0		0xe7fffffe

#endif

/* __SRCVERSION("opcode.h $Rev: 160127 $"); */
