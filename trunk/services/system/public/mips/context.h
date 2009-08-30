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
 *  mips/context.h
 *

 */
#ifndef __MIPS_CONTEXT_H_INCLUDED
#define __MIPS_CONTEXT_H_INCLUDED
 
/*
 * Names for register positions in the exception frame
 */
#define MIPS_REG_ZERO		0
#define MIPS_REG_AT			1
#define MIPS_REG_V0			2
#define MIPS_REG_V1			3
#define MIPS_REG_A0			4
#define MIPS_REG_A1			5
#define MIPS_REG_A2			6
#define MIPS_REG_A3			7
#define MIPS_REG_T0			8
#define MIPS_REG_T1			9
#define MIPS_REG_T2			10
#define MIPS_REG_T3			11
#define MIPS_REG_T4			12
#define MIPS_REG_T5			13
#define MIPS_REG_T6			14
#define MIPS_REG_T7			15
#define MIPS_REG_S0			16
#define MIPS_REG_S1			17
#define MIPS_REG_S2			18
#define MIPS_REG_S3			19
#define MIPS_REG_S4			20
#define MIPS_REG_S5			21
#define MIPS_REG_S6			22
#define MIPS_REG_S7			23
#define MIPS_REG_T8			24
#define MIPS_REG_T9			25
#define MIPS_REG_K0			26
#define MIPS_REG_K1			27
#define MIPS_REG_GP			28
#define MIPS_REG_SP			29
#define MIPS_REG_S8			30
#define MIPS_REG_RA			31
#define MIPS_REG_SREG		32
#define MIPS_REG_LO			33
#define MIPS_REG_HI			34
#define MIPS_REG_BADVADDR	35
#define MIPS_REG_CAUSE		36
#define MIPS_REG_EPC		37
/*#define MIPS_REG_ERR_EPC	38*/
#define MIPS_REG_NUM		(MIPS_REG_EPC+1)

/*
 * Base Address of register in save context.
 */
#define MIPS_BAREG(idx)	((idx)*2)

/*
 * Addressing the slot from assembly (scales for the 2 longwords)
 */
#define MIPS_AREG(idx) (MIPS_BAREG(idx)*4)

#if defined(__BIGENDIAN__)
	#define MIPS_REGS_LOW_WORD	1
#elif defined(__LITTLEENDIAN__)
	#define MIPS_REGS_LOW_WORD	0
#else
	#error ENDIAN Not defined for system
#endif


#ifndef __ASM__

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

typedef union {
	_Uint64t	u64;
	double		f;
} mipsfloat;

typedef struct mips_cpu_registers {
	_Uint32t	regs[(MIPS_REG_NUM-1)*2];
	_Uint64t	regs_alignment;
} MIPS_CPU_REGISTERS;

typedef struct mips_fpu_registers {
	mipsfloat	fpr[32];
	_Uint32t	fpcr31;
} MIPS_FPU_REGISTERS;

typedef struct mips_alt_registers {
	union {
		struct mips_tx79 {
			_Uint64t	gpr_hi[32];
			_Uint64t	lo1;
			_Uint64t	hi1;
			_Uint32t	sa;
		} tx79;
	} un;
} MIPS_ALT_REGISTERS;

typedef struct mips_sb1_perfregs {
	_Uint32t	id;
	_Uint32t	EventControl[4];
	_Uint32t	pad;
	_Uint64t	EventCounter[4];
	_Uint64t	PTR;
} MIPS_SB1_PERFREGS;

typedef union {
	_Uint32t			id;
	MIPS_SB1_PERFREGS	sb1;
} MIPS_PERFREGS;

/*
 * Addressing the lower and upper 32-bits from C
 */
#define MIPS_CREG(idx)	(MIPS_BAREG(idx) + MIPS_REGS_LOW_WORD)
#define MIPS_HCREG(idx)	(MIPS_BAREG(idx) + (MIPS_REGS_LOW_WORD^1))

/*
 * MIPS_REG_SETx64()
 *	Set a 64-bit register to its 32-bit pointer value
 *
 */
#define MIPS_REG_SETx64(rs, field, v) \
	(((rs)->regs[MIPS_CREG(field)] = (unsigned long)(v)), \
	 ((rs)->regs[MIPS_HCREG(field)] = \
	  (((unsigned long)(v) & 0x80000000) ? 0xFFFFFFFF : 0)))

#define MIPS_GET_REGIP(reg)			((reg)->regs[MIPS_CREG(MIPS_REG_EPC)])
#define MIPS_GET_REGSP(reg)			((reg)->regs[MIPS_CREG(MIPS_REG_SP)])
#define MIPS_SET_REGIP(reg,v)		(MIPS_REG_SETx64((reg),MIPS_REG_EPC,v)
#define MIPS_SET_REGSP(reg,v)		(MIPS_REG_SETx64((reg),MIPS_REG_SP,v)

#endif 

#endif /* __MIPS_CONTEXT_H_INCLUDED */

/* __SRCVERSION("context.h $Rev: 153052 $"); */
