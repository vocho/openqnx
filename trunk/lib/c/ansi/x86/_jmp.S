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




#define REG_EAX  0x1c
#define REG_EBX  0x10
#define REG_ECX  0x18
#define REG_EDX  0x14
#define REG_EDI  0x00
#define REG_ESI  0x04
#define REG_EBP  0x08
#define REG_EIP  0x20
#define REG_CS   0x24
#define REG_EFL  0x28
#define REG_ESP  0x2c
#define REG_SS   0x30

		.text

		.globl	setjmp
setjmp:
#if 0
#if defined(__PIC__)
		pushl	%ebx
		call	1f
1:		popl	%ebx
		addl	$_GLOBAL_OFFSET_TABLE_+[.-1b],%ebx
		pushl	$1
		pushl	12(%esp)
		call	__sigjmp_prolog@PLT
		addl	$8,%esp
		popl	%ebx
#else
		pushl	$1
		pushl	8(%esp)
		call	__sigjmp_prolog
		addl	$8,%esp
#endif
#endif

		.globl	_setjmp
_setjmp:
		movl	4(%esp),%eax		/* Get arg from stack */
		movl	%edi,REG_EDI(%eax)
		movl	%esi,REG_ESI(%eax)
		movl	%ebp,REG_EBP(%eax)
		movl	%ebx,REG_EBX(%eax)
		movl	%edx,REG_EDX(%eax)
		movl	%ecx,REG_ECX(%eax)
		popl	REG_EIP(%eax)
		movl	%esp,REG_ESP(%eax)
		pushl	REG_EIP(%eax)
		subl	%eax,%eax
		ret		
		.type	_setjmp,@function
		.size   _setjmp,.-_setjmp
		.type	setjmp,@function
		.size   setjmp,.-setjmp

		.globl	_longjmp
_longjmp:
		popl	%eax		/* Remove return address */
		popl	%eax
		popl	%edx
		movl	REG_ESP(%eax),%esp
		pushl	REG_EIP(%eax)
		orl		%edx,%edx
		jne		1f
		incl	%edx
1:
		pushl	%edx
		movl	REG_EDI(%eax),%edi
		movl	REG_ESI(%eax),%esi
		movl	REG_EBP(%eax),%ebp
		movl	REG_EBX(%eax),%ebx
		movl	REG_EDX(%eax),%edx
		movl	REG_ECX(%eax),%ecx
		popl	%eax
		ret		
		.type	 _longjmp,@function
		.size	 _longjmp,.-_longjmp
