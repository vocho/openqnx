#
# $QNXLicenseA:
# Copyright 2007, QNX Software Systems. All Rights Reserved.
# 
# You must obtain a written license from and pay applicable license fees to QNX 
# Software Systems before you may reproduce, modify or distribute this software, 
# or any work that includes all or part of this software.   Free development 
# licenses are available for evaluation and non-commercial purposes.  For more 
# information visit http://licensing.qnx.com or email licensing@qnx.com.
#  
# This file may contain contributions from others.  Please review this entire 
# file for other proprietary rights or license notices, as well as the QNX 
# Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
# for other information.
# $
#

	.globl	xfer_setjmp
	.globl	xfer_longjmp

#include "asmoff.def"

	.text

xfer_setjmp:
	mov	4(%esp),%eax		/ Get arg from stack
	mov	%edi,REG_EDI(%eax)
	mov	%esi,REG_ESI(%eax)
	mov	%ebp,REG_EBP(%eax)
	mov	%ebx,REG_EBX(%eax)
	pop	REG_EIP(%eax)
	mov	%esp,REG_ESP(%eax)
	push	REG_EIP(%eax)
	sub	%eax,%eax
	ret

xfer_longjmp:
	pop	%edx
	pop	%edx
	pop	%eax
	pop	%ecx
	mov	REG_ESP(%edx),%esp
	push	REG_EIP(%edx)
	or	%eax,%eax
	jne	ret_ok
	inc	%eax
ret_ok:
	mov	REG_EDI(%edx),%edi
	mov	REG_ESI(%edx),%esi
	mov	REG_EBP(%edx),%ebp
	mov	REG_EBX(%edx),%ebx
	ret
