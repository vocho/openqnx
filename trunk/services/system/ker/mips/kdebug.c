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

#include "externs.h"
#include <mips/opcode.h>

/*
 * mips_cause2sig_map:
 *
 * Maps an MIPS Cause code to a signal number.
 * The first 32 define the actual EXCODE of the
 * cause registers. Values >= 32 are extended
 * cause code to cover exceptions other than
 * general exceptions.
 */

#define MK(signo,code,fault)		MAKE_SIGCODE(SIG##signo,signo##_##code,FLT##fault)

#define BAD(cause)	MAKE_SIGCODE(SIGKILL,0,64+cause)

MIPS_CAUSE2SIGMAP_CONST unsigned long __mips_cause2sig_map[] = {
    BAD(0),						/* 00 - CAUSE_INTERRUPT                 */
    MK(SEGV,ACCERR,ACCESS)|SIGCODE_STORE,/* 01 - CAUSE_TLB_MOD           */
    MK(SEGV,MAPERR,BOUNDS),		/* 02 - CAUSE_TLB_LOAD                  */
    MK(SEGV,MAPERR,BOUNDS)|SIGCODE_STORE,	/* 03 - CAUSE_TLB_SAVE      */
    MK(BUS,ADRALN,ACCESS),		/* 04 - CAUSE_ADDR_LOAD                 */
    MK(BUS,ADRALN,ACCESS)|SIGCODE_STORE,/* 05 - CAUSE_ADDR_SAVE         */
    MK(BUS,ADRERR,BOUNDS),		/* 06 - CAUSE_BUS_INSTR                 */
    MK(BUS,ADRERR,BOUNDS),		/* 07 - CAUSE_BUS_DATA                  */
    BAD(8),						/* 08 - CAUSE_SYSCALL                   */
    MK(TRAP,BRKPT,BPT),			/* 09 - CAUSE_BP                        */
    MK(ILL,ILLOPC,ILL),			/* 10 - CAUSE_ILLOP                     */
    MK(ILL,PRVOPC,PRIV),		/* 11 - CAUSE_CP_UNUSABLE               */
    MK(FPE,INTOVF,IOVF),		/* 12 - CAUSE_OVFLW                     */
    MK(FPE,INTOVF,IOVF), 		/* 13 - CAUSE_TRAP                      */
    BAD(14),    				/* 14 - CAUSE_VIRT_COHERENCY_INSTR      */
    MK(FPE,NOFPU,FPE), 			/* 15 - CAUSE_FPE                       */
    BAD(16),    				/* 16 - Reserved, should never happen   */
    BAD(17),    				/* 17 - Reserved, should never happen   */
    BAD(18),    				/* 18 - Reserved, should never happen   */
    BAD(19),    				/* 19 - Reserved, should never happen   */
    BAD(20),    				/* 20 - Reserved, should never happen   */
    BAD(21),    				/* 21 - Reserved, should never happen   */
    BAD(22),    				/* 22 - Reserved, should never happen   */
    MK(TRAP,BRKPT,WATCH),		/* 23 - Watchpoint */
    BAD(24),    				/* 24 - Machine chk, should never happen   */
    BAD(25),    				/* 25 - Reserved, should never happen   */
    BAD(26),    				/* 26 - Reserved, should never happen   */
    BAD(27),    				/* 27 - Reserved, should never happen   */
    BAD(28),    				/* 28 - Reserved, should never happen   */
    BAD(29),   					/* 29 - Reserved, should never happen   */
    BAD(30),    				/* 30 - Reserved, should never happen   */
    BAD(31),    				/* 31 - Reserved, should never happen   */
    BAD(32),    				/* 32 - MIPS extension, Soft reset      */
    BAD(33),    				/* 33 - MIPS extension, NMI interrupt   */
    MK(BUS,ADRERR,CACHERR),		/* 34 - MIPS extension, Cache error     */
    MK(SEGV,MAPERR,UTLBREFILL),	/* 35 - MIPS extension, 32B TLB refill  */
    MK(SEGV,MAPERR,XTLBREFILL),	/* 36 - MIPS extension, 64B TLB refill  */
    };

/*
 * kdebug_callout
 *
 * It's a wrapper for the kdebug_enter call. It's called
 * when an exception occurred in kernel context.
 *
 * Check for the special break points and invoke kernel debugger as
 * appropriate.
 */
ulong_t
kdebug_callout(unsigned long sigcode, MIPS_CPU_REGISTERS *ctx, unsigned loc) {
    PROCESS 	*prp;

	sigcode |= loc;

	prp = (sigcode & SIGCODE_INTR) ? aspaces_prp[KERNCPU] : 0;

	if(SIGCODE_SIGNO(sigcode) == SIGTRAP) {
		struct kdebug_callback	*call;
		uintptr_t				ip = REGIP(ctx);

		switch(*(ulong_t *)ip) {
		case OPCODE_BREAKX(8):	//DebugBreak()
			SETREGIP(ctx, ip + 4);
			break;
		case OPCODE_BREAKX(9):	//DebugKDBreak()
			sigcode |= SIGCODE_KERNEL;
			sigcode &= ~SIGCODE_FATAL;
			SETREGIP(ctx, ip + 4);
			(void) kdebug_enter(prp, sigcode, ctx);
			return(0);
		case OPCODE_BREAKX(10):	//DebugKDOutput()
			call = privateptr->kdebug_call;
			if(call != NULL && call->msg_entry != NULL) {
				call->msg_entry(
					(char *)ctx->regs[MIPS_CREG(MIPS_REG_A0)],
					ctx->regs[MIPS_CREG(MIPS_REG_A1)]);
			}
			SETREGIP(ctx, ip + 4);
			return(0);
		default:
			break;
		}
	}
	/*
	 * Call kernel debugger callout if it's present.
	 */
	return(kdebug_enter(prp, sigcode, ctx));
}

__SRCVERSION("kdebug.c $Rev: 161879 $");
