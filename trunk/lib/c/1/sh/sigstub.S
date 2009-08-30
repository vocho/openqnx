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
 * sigstub.s
 *	Routine for delivering a signal to a thread
 */
	.include "asmoff.def"

	.extern SignalReturn

#
# int _signalstub (_sighandler_info *ptr)
#
# The kernel has already saved GR0-GR4, SR for us, so we
# can trash them if necessary.
#
# The kernel placed the *ptr parameter in r4 (and in r0, for older
# __signalstub implementations, but we do not care about that here).
#
# ptr points to a _sighandler_info structure.  ptr->context contains a pointer
# to a ucontext_t structure (not typed in the definition of _sighandler_info).
#
 	.global 	__signalstub
	.section 	".text"
	.type 		__signalstub,@function
__signalstub:
	mov r4, r0  ! r4 will be our context ptr, r0 will be our sighandler ptr
	# save context
	# get pointer to context save area
	mov.l	@(SIGSTACK_CONTEXT,r0),r4
	add	#(UCONTEXT_CPU + SIZE_CONTEXTCPU),r4
	sts.l	pr,@-r4
	sts.l	macl,@-r4
	sts.l	mach,@-r4
	stc.l	gbr,@-r4
	add	#-(3*4),r4	! skip sr, pc and r15
	mov.l	r14,@-r4	
	mov.l	r13,@-r4	
	mov.l	r12,@-r4	
	mov.l	r11,@-r4	
	mov.l	r10,@-r4	
	mov.l	r9,@-r4
	mov.l	r8,@-r4	
	mov.l	r7,@-r4	
	mov.l	r6,@-r4	
	mov.l	r5,@-r4	
	add	#(-5*4),r4  ! skip r4 through r0
	# r4 is left pointing at ptr->context.uc_mcontext.cpu

	# save context pointer
	mov	r4,r8
	# save sighandler pointer
	mov	r0,r9

	# save the FPU context on the stack
	sts.l	fpscr,@-r15
	sts.l	fpul,@-r15
	mov.l	L_double_mode_fpscr,r1
	lds	r1,fpscr
	fmov	fr0,@-r15
	fmov	fr1,@-r15
	fmov	fr2,@-r15
	fmov	fr3,@-r15
	fmov	fr4,@-r15
	fmov	fr5,@-r15
	fmov	fr6,@-r15
	fmov	fr7,@-r15
	fmov	fr8,@-r15
	fmov	fr9,@-r15
	fmov	fr10,@-r15
	fmov	fr11,@-r15
	fmov	fr12,@-r15
	fmov	fr13,@-r15
	fmov	fr14,@-r15
	fmov	fr15,@-r15

	mov.l	@(SIGSTACK_HANDLER,r0),r1
	add	#-UCONTEXT_CPU,r4		
	mov	r4,r6				! third param = ptr->context
	mov.l	@(SIGSTACK_SIGNO,r0),r4		! first param = signo
	jsr	@r1
	mov	r0,r5				! second parm = &ptr->siginfo

	# restore context pointer
	mov	r8,r0
	# restore sighandler pointer
	mov	r9,r4

	fmov	@r15+,fr15
	fmov	@r15+,fr14
	fmov	@r15+,fr13
	fmov	@r15+,fr12
	fmov	@r15+,fr11
	fmov	@r15+,fr10
	fmov	@r15+,fr9
	fmov	@r15+,fr8
	fmov	@r15+,fr7
	fmov	@r15+,fr6
	fmov	@r15+,fr5
	fmov	@r15+,fr4
	fmov	@r15+,fr3
	fmov	@r15+,fr2
	fmov	@r15+,fr1
	fmov	@r15+,fr0

	lds.l	@r15+,fpul
	lds.l	@r15+,fpscr

	#restore context
	add	#(5*4),r0	! we skipped r0-r4
	mov.l	@r0+,r5
	mov.l	@r0+,r6
	mov.l	@r0+,r7
	mov.l	@r0+,r8	
	mov.l	@r0+,r9	
	mov.l	@r0+,r10	
	mov.l	@r0+,r11
	mov.l	@r0+,r12	
	mov.l	@r0+,r13	
	mov.l	@r0+,r14	
	add	#(3*4),r0	! we skipped sr, pc r15
	ldc.l	@r0+,gbr
	lds.l	@r0+,mach
# tell the kernel the signal is done
#ifndef __PIC__	
	lds.l	@r0+,macl
	lds.l	@r0+,pr
	mov.l	__signalstub_sigret,r0
	jmp	@r0
	nop
	.align 2
__signalstub_sigret:	
	.long	SignalReturn	
#else
	lds.l	@r0+,macl
	lds.l	@r0+,pr
	mov.l	r4,@-r15
	mov.l	__got,r4
	mova	__got,r0
	add	r0,r4
	mov.l	__signalstub_ret,r0
	mov.l	@(r0,r4),r0
	mov.l	@r15+,r4
	jmp	@r0
	nop
	.align 2
__signalstub_ret:	
	.long	SignalReturn@GOT
__got:
	.long	_GLOBAL_OFFSET_TABLE_
#endif
	.size 	__signalstub,.-__signalstub

L_double_mode_fpscr:
	.long	0xc0404		! double precision, div-by-zero exceptions enabled
