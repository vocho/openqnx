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
 * WARNING WARNING WARNING - gdb uses a bytecode signature to sniff for signal handler
 * frames.
 *
 * DON'T CHANGE THIS FILE without consulting with the gdb guys...
 *
 */


/*
 * sigstub.S
 *	Routine for delivering a signal to a thread
 */
#include <mips/asm.h>
#include <mips/context.h>
#include <asmoff.def>

	.extern SignalReturn
	.extern	__cpu_flags
 
#define UC_REG(reg)	(MIPS_AREG(reg)+UCONTEXT_CPU)
		
#define SAVE_ALL \
		SAVE_REG(zero, ZERO);	\
		SAVE_REG(v1, V1);		\
		SAVE_REG(a1, A1);		\
		SAVE_REG(a2, A2);		\
		SAVE_REG(a3, A3);		\
		SAVE_REG(t0, T0);		\
		SAVE_REG(t1, T1);		\
		SAVE_REG(t2, T2);		\
		SAVE_REG(t3, T3);		\
		SAVE_REG(t4, T4);		\
		SAVE_REG(t5, T5);		\
		SAVE_REG(t6, T6);		\
		SAVE_REG(t7, T7);		\
		SAVE_REG(s0, S0);		\
		SAVE_REG(s1, S1);		\
		SAVE_REG(s2, S2);		\
		SAVE_REG(s3, S3);		\
		SAVE_REG(s4, S4);		\
		SAVE_REG(s5, S5);		\
		SAVE_REG(s6, S6);		\
		SAVE_REG(s7, S7);		\
		SAVE_REG(s8, S8);		\
		SAVE_REG(t8, T8);		\
		SAVE_REG(t9, T9);		\
		SAVE_REG(zero, K0);		\
		SAVE_REG(zero, K1);		\
		mflo	t0		;		\
		mfhi	t1		;		\
		SAVE_REG(gp, GP);		\
		SAVE_REG(ra, RA);		\
		SAVE_REG(t0, LO);		\
		SAVE_REG(t1, HI)		
		
#define RESTORE_ALL				\
		RESTORE_REG(t0,LO);		\
		RESTORE_REG(t1,HI);		\
		RESTORE_REG(v1,V1);		\
		mtlo	t0		;		\
		mthi	t1		;		\
		RESTORE_REG(a1,A1);		\
		RESTORE_REG(a2,A2);		\
		RESTORE_REG(a3,A3);		\
		RESTORE_REG(t0,T0);		\
		RESTORE_REG(t1,T1);		\
		RESTORE_REG(t2,T2);		\
		RESTORE_REG(t3,T3);		\
		RESTORE_REG(t4,T4);		\
		RESTORE_REG(t5,T5);		\
		RESTORE_REG(t6,T6);		\
		RESTORE_REG(t7,T7);		\
		RESTORE_REG(s0,S0);		\
		RESTORE_REG(s1,S1);		\
		RESTORE_REG(s2,S2);		\
		RESTORE_REG(s3,S3);		\
		RESTORE_REG(s4,S4);		\
		RESTORE_REG(s5,S5);		\
		RESTORE_REG(s6,S6);		\
		RESTORE_REG(s7,S7);		\
		RESTORE_REG(s8,S8);		\
		RESTORE_REG(t8,T8);		\
		RESTORE_REG(t9,T9);		\
		RESTORE_REG(ra,RA);		\
		RESTORE_REG(gp,GP)
				
#
# int __signalstub (struct _sighandler_info *ptr) // pointer in V0
#
# The kernel has already saved A0,V0,AT,SP,PC for us. It also ensures
# that the stack is 8-byte aligned.
#
FRAME(__signalstub,sp,0,ra)
 		.set	noreorder
		.set	noat
		
		move	a0,s7
#ifdef __PIC__
		move	AT,ra
		.cpload	ra				# We need to temporarily load $s7
#endif
		lw		s7,__cpu_flags
#ifdef __PIC__
		move	ra,AT
#endif
		andi	s7,MIPS_CPU_FLAG_64BIT
		bnez	s7,__signalstub64
		 move	s7,a0
		 
		#
		# Fall into __signalstub32
		#
		 
		.globl __signalstub32
__signalstub32:
		addiu	sp,sp,-16		# allocate new stack frame
		
		lw		a0,SIGSTACK_CONTEXT(v0)	# get context save area ptr
		
#define SAVE_REG(src, dst)	\
	sw src,UC_REG(MIPS_REG_##dst)+(MIPS_REGS_LOW_WORD*4)(a0);	\
	sra AT,src,31;	\
	sw AT,UC_REG(MIPS_REG_##dst)+((MIPS_REGS_LOW_WORD^1)*4)(a0)	
	
		SAVE_ALL
		
		move	s0,v0			# save _sighandler *
		move	s1,a0			# save context *
		
		move	a1,v0			# get _sighandler * into right register
		lw		t9,SIGSTACK_HANDLER(v0) # get handler routine (must be in t9)
		lw		a0,SIGSTACK_SIGNO(v0) # get signal number
		jalr	t9
		 move	a2,s1				# load signal context
		 
		move	v0,s1			# restore pointers to proper registers
		move	a0,s0

#ifdef __PIC__
		.cpload	ra				# We need to temporarily load $s7
#ifdef __MIPS_ABICALLS__
      addi    gp,8            /* abicalls change */
#endif
#endif
		la		AT,SignalReturn	
		
#define RESTORE_REG(dst, src)	\
	lw dst,UC_REG(MIPS_REG_##src)+(MIPS_REGS_LOW_WORD*4)(v0)	
	
		RESTORE_ALL
		
		jr		AT
		 nop
		
#undef SAVE_REG
#undef RESTORE_REG
		
		.globl __signalstub64
__signalstub64:
		.set	mips3
		
		addiu	sp,sp,-16		# allocate new stack frame
		
		
		lw		a0,SIGSTACK_CONTEXT(v0)	# get context save area ptr
		
#define SAVE_REG(src, dst)	\
	sd src,UC_REG(MIPS_REG_##dst)(a0)	
	
		SAVE_ALL
		
		move	s0,v0			# save _sighandler *
		move	s1,a0			# save context *
		
		move	a1,v0			# get _sighandler * into right register
		lw		t9,SIGSTACK_HANDLER(v0) # get handler routine (must be in t9)
		lw		a0,SIGSTACK_SIGNO(v0) # get signal number
		jalr	t9
		 move	a2,s1				# load signal context
		 
		move	v0,s1			# restore pointers to proper registers
		move	a0,s0

#ifdef __PIC__
		.cpload	ra				# We need to temporarily load $s7
#ifdef __MIPS_ABICALLS__
		addi	gp,8			/* abicalls change */
#endif
#endif
		la		AT,SignalReturn	
		
#define RESTORE_REG(dst, src)	\
	ld dst,UC_REG(MIPS_REG_##src)(v0)	
	
		RESTORE_ALL
		
		jr		AT
		 nop
ENDFRAME(__signalstub)
