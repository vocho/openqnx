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
 *  mips/asm.h
 *

 *
 */
#ifndef _MIPS_ASM_H
#define _MIPS_ASM_H
/*
 * asm.h
 *	Support stuff for assembler defintions
 */

/*
 * Define the assembler directive needed when compiling 
 * for PIC.
 */

#if !defined(__PIC__)
	#define PIC_DIRECTIVE
#elif defined(__MIPS_ABICALLS__)
	#define PIC_DIRECTIVE   .abicalls
#else
	#define PIC_DIRECTIVE   .qnxpiccalls
#endif

/*
 * Define the assembler support macros for procedure framing
 * FRAME    : sets proc as global, generates debug info with .ent
 * ENDFRAME : generates debug into with .end
 */
#define FRAME(name, frame_register, offset, return_register)  \
    .globl name;                                              \
    .ent   name;                                              \
    PIC_DIRECTIVE;                                            \
name:                                                         \
    .frame frame_register, offset, return_register
 
#define ENDFRAME(name)                                        \
    .end name

/*
 * Definitions of registers, for assembler
 */
#define zero	$0		/* always zero */
#define AT	$1		/* reserved for assembler */

#define v0	$2		/* return value */
#define v1	$3

#define a0	$4		/* arguments 0..3 */
#define a1	$5
#define a2	$6
#define a3	$7

#define t0	$8		/* temporary registers t0-t9 */
#define t1	$9		/* saved by caller */
#define t2	$10
#define t3	$11
#define t4	$12 
#define t5	$13
#define t6	$14
#define t7	$15
#define t8	$24
#define t9	$25

#define s0	$16		/* saved by callee */
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23
#define s8	$30

#define k0	$26		/* reserved for kernel */
#define k1	$27

#define gp	$28		/* global pointer */
#define sp	$29		/* stack pointer */
#define ra	$31		/* return address */

/*
 * Floating point register names
 */
#define FPU_R0	$f0
#define FPU_R1	$f1
#define FPU_R2	$f2
#define FPU_R3	$f3
#define FPU_R4	$f4
#define FPU_R5	$f5
#define FPU_R6	$f6
#define FPU_R7	$f7
#define FPU_R8	$f8
#define FPU_R9	$f9
#define FPU_R10 $f10
#define FPU_R11 $f11
#define FPU_R12 $f12
#define FPU_R13 $f13
#define FPU_R14 $f14
#define FPU_R15 $f15
#define FPU_R16 $f16
#define FPU_R17 $f17
#define FPU_R18 $f18
#define FPU_R19 $f19
#define FPU_R20 $f20
#define FPU_R21 $f21
#define FPU_R22 $f22
#define FPU_R23 $f23
#define FPU_R24 $f24
#define FPU_R25 $f25
#define FPU_R26 $f26
#define FPU_R27 $f27
#define FPU_R28 $f28
#define FPU_R29 $f29
#define FPU_R30 $f30
#define FPU_R31 $f31

/*
 * CP0 Register definitions
 */
#define CP0_INDEX      $0	      /* TLB Index */
#define CP0_RANDOM     $1	      /* TLB Random */
#define CP0_TLB_LO_0   $2	      /* TLB Entry Lo0 */
#define CP0_TLB_LO_1   $3	      /* TLB Entry Lo1 */
#define CP0_CONTEXT    $4	      /* Kernel PTE pointer */
#define CP0_PAGEMASK   $5	      /* TLB Page Mask */
#define CP0_WIRED      $6	      /* TLB Wired */
#define CP0_BADVADDR   $8	      /* Bad Virtual Address */
#define CP0_COUNT      $9	      /* Count */
#define CP0_TLB_HI     $10	      /* TLB Entry Hi */
#define CP0_COMPARE    $11	      /* Timer Compare */
#define CP0_SREG       $12	      /* Status */
#define CP0_CAUSE      $13	      /* Cause */
#define CP0_EPC        $14	      /* Exception PC */
#define CP0_PRID       $15	      /* Proc Rev ID */
#define CP0_CONFIG     $16	      /* Configuration */
#define CP0_LLADDR     $17	      /* Load/Link address */
#define CP0_WATCHLO    $18	      /* Low Watch address */
#define CP0_WATCHHI    $19	      /* High Watch address */
#define CP0_XCONTEXT   $20	      /* Extended context */
#define CP0_ECC        $26	      /* ECC and parity */
#define CP0_CACHERR    $27	      /* Cache Err/Status */
#define CP0_TAGLO      $28	      /* Cache Tag Lo */
#define CP0_TAGHI      $29	      /* Cache Tag Hi */
#define CP0_ERR_EPC    $30	      /* Error exception PC */

/*
 * For the r7k there are two sets of CP0 registers.
 * Set 0 is the usual set of COP0 registers, defined above.
 * The following are the registers defined for set 1, which
 * are accessed through the cfc0 and ctc0 instructions.
 */
#define CP0_IPL_LO	$18	/* interrupt prio for [0..7] interrupts */
#define CP0_IPL_HI	$19	/* interrupt prio for [8..15] interrupts */
#define CP0_INT_CTL	$20	/* interrupt control register */
#define CP0_ERR_ADDR0	$26	/* imprecise error address */
#define CP0_ERR_ADDR1	$27	/* imprecise error address */

/*
 * Macros that enable/disable interrupts and know about the __shadow_imask
 * kludge.
 */

#define DISABLEINTERRUPTS(x,y)		\
	mfc0	x,CP0_SREG ;			\
	 nop ;							\
	ori	y,x,MIPS_SREG_IE ;			\
	xori	y,y,MIPS_SREG_IE ;		\
	mtc0	y,CP0_SREG ;			\
	 nop; nop; nop; nop; nop; nop; nop; nop;

#define ENABLEINTERRUPTS(x,y)		\
	.extern __shadow_imask ;		\
	mfc0	x,CP0_SREG ;			\
	 li	y,~0xff00 ;					\
	ori	x,x,MIPS_SREG_IE ;			\
	and	x,y ;						\
	lw	y,__shadow_imask ;			\
	lw	y,0(y) ;					\
	or	x,y ;						\
	mtc0	x,CP0_SREG ;			\
	 nop; nop; nop; nop; nop; nop; nop; nop;

#define RESTOREINTERRUPTS(x,y)		\
	.extern __shadow_imask ;		\
	mfc0	y,CP0_SREG ;			\
	 andi	x,MIPS_SREG_IE ;		\
	or	x,y ;						\
	li	y,~0xff00 ;					\
	and	x,y ;						\
	lw	y,__shadow_imask ;			\
	lw	y,0(y) ;					\
	or	x,y ;						\
	mtc0	x,CP0_SREG ;			\
	 nop; nop; nop; nop; nop; nop; nop; nop;

#define MFC0_SEL_OPCODE(dst, src, sel)\
	  	.word (0x40000000 | ((dst)<<16) | ((src)<<11) | (sel))

#define MTC0_SEL_OPCODE(dst, src, sel)\
	  	.word (0x40800000 | ((dst)<<16) | ((src)<<11) | (sel))

#define DMFC0_SEL_OPCODE(dst, src, sel) \
		.word (0x40200000 | ((dst)<<16) | ((src)<<11) | (sel))

#define DMTC0_SEL_OPCODE(dst, src, sel) \
		.word (0x40a00000 | ((dst)<<16) | ((src)<<11) | (sel))

#define ssnop	sll	zero,zero,1

/*
 * R7K CPU Errata #18 workaround.
 */
#define MIPS_CP0_REG_STORE_DELAY(src,dst) \
	mtc0 src,dst; nop; nop; nop; nop; nop; nop; nop; nop

#endif /* _MIPS_ASM_H */

/* __SRCVERSION("asm.h $Rev: 153052 $"); */
