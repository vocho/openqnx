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
 *	sigstub.S
 *
 *	int __signalstub (struct _sighandler_info *ss)
 */

#include <asmoff.def>

	.text
	.align 2

	.extern	SignalReturn

	.global	__signalstub

__signalstub:
	/*
	 * Save r1-r12 in _sighandler_info->context.uc_mcontext.
	 * The kernel saves r0, sp, lr, pc and spsr.
	 */
	ldr		lr, [r0, #SIGSTACK_CONTEXT]
	add		lr, lr, #UCONTEXT_CPU+4
	stmia	lr, {r1-r12}

	/*
	 * Remember the _sighandler_info and uc_mcontext pointers
	 */
	mov		r4, r0
	mov		r5, lr

	/*
	 * Call handler(signo, siginfo_t *, ucontext_t *)
	 */
	ldr		r0, [r4,#SIGSTACK_SIGNO]
	add		r1, r4, #SIGSTACK_SIGINFO
	sub		r2, lr, #UCONTEXT_CPU+4
	mov		lr, pc
	ldr		pc, [r4,#SIGSTACK_HANDLER]

	/*
	 * Restore r1-r11 and call SignalReturn(_sighandler_info *)
	 *
	 * Note that SignalReturn trashes r0 and ip, which means that
	 * the kernel must restore r0, ip, sp, lr, pc and spsr.
	 */
	mov		r0, r4
	ldmia	r5, {r1-r11}
#ifdef	__PIC__
	b		SignalReturn(PLT)
#else
	b		SignalReturn
#endif

	.type	 __signalstub,function
	.size	 __signalstub,.-__signalstub
