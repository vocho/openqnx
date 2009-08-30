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


#include	"asmoff.def"

/*
 *
 * int __signalstub (struct _sighandler_info *ptr)
 *
 * The kernel has already saved EAX,EFL,CS,SS,SP,PC for us. 
 *
 * On entry
 *  esp -> signo,code,value,handler,...stuff...
 */
.globl		__signalstub
__signalstub:
	movl	SIGSTACK_CONTEXT(%esp),%eax	// Get context ptr
	//
	// context order: edi, esi, ebp, exx, ebx, edx, ecx, eax
	//
	mov		%edi,0x00+UCONTEXT_CPU(%eax)				// save registers
	mov		%esi,0x04+UCONTEXT_CPU(%eax)		
	mov		%ebp,0x08+UCONTEXT_CPU(%eax)
	mov		%ebx,0x10+UCONTEXT_CPU(%eax)
	mov		%edx,0x14+UCONTEXT_CPU(%eax)
	mov		%ecx,0x18+UCONTEXT_CPU(%eax)
	
	mov		%esp,%esi					// save _sighandler *
	mov		%eax,%edi					// save context *
	
	push	%eax						// pass context *
	push	%esi						// pass siginfo_t *
	pushl	SIGSTACK_SIGNO(%esi)		// pass signo 
	call	*SIGSTACK_HANDLER(%esi)		// Call user's handler 
	
	push	%esi						// pass _sighandler * to SignalReturn
	
	mov		%edi,%eax					// get context back into good reg
	
	mov		0x00+UCONTEXT_CPU(%eax),%edi	// restore registers
	mov		0x04+UCONTEXT_CPU(%eax),%esi		
	mov		0x08+UCONTEXT_CPU(%eax),%ebp
	mov		0x10+UCONTEXT_CPU(%eax),%ebx
	mov		0x14+UCONTEXT_CPU(%eax),%edx
	mov		0x18+UCONTEXT_CPU(%eax),%ecx
	
#ifdef KERNEL_SAVED_EBX
.extern		SignalReturn
#ifdef __PIC__
	call	1f
1:	popl	%ebx						/* Load EBX for GOT indexing */
	addl	$_GLOBAL_OFFSET_TABLE_+[.-1b],%ebx
	call	SignalReturn@PLT			/* This will NOT return */
#else
	call	SignalReturn				/* This will NOT return */
#endif
#else
	subl	$4,%esp
	movl	$ KER_SIGNAL_RETURN,%eax
	int		$0x28
	ret
	ret
#endif
	.type	 __signalstub,@function
	.size	 __signalstub,.-__signalstub
