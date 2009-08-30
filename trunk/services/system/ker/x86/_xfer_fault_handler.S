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

/
/ _xfer_fault_handler.S
/	Routines for error handling for msg xfer 
/

/ handlers defined in this module are used to handle errors when the called function 
/ does not reserve any registers in stack

	#include "asmoff.def"
	#include "util.ah"
	
	.global xfer_dst_handlers
	.global xfer_src_handlers
	
	.global xfer_fault_handlers
	.global xfer_async_handlers
	
	.extern	xfer_restart
	.extern	xfer_async_restart
	
	.data
xfer_dst_handlers:
	.long	_xfer_dst_fault_jmp
	.long	xfer_restart
	
xfer_src_handlers:
	.long	_xfer_src_fault_jmp
	.long	xfer_restart
	
xfer_fault_handlers:
	.long	_xfer_dst_fault_jmp
	.long	0	
	
xfer_async_handlers:
	.long	_xfer_src_fault_jmp
	.long	xfer_async_restart
	
	.text
	.global _xfer_dst_fault_jmp
	
 /
 / int _xfer_dst_fault_jmp(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) 
 /
 /	handler for dst fault in msg xfer
 /
 _xfer_dst_fault_jmp:
	/ see what fault it is
	movl	$XFER_DST_FAULT,%eax
	testl	$X86_FAULT_WRITE,12(%esp)	/ 4+8
	je		1f
	movl	$XFER_SRC_FAULT,%eax
	jmp		1f
		
	
 /
 / int _xfer_src_fault_jmp(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) 
 /
 / handler for src fault in msg xfer
 /
 _xfer_src_fault_jmp:
	/ see what fault it is
	movl	$XFER_DST_FAULT,%eax
	testl	$X86_FAULT_WRITE,12(%esp)	/ 4+8
	jne		1f
	movl	$XFER_SRC_FAULT,%eax
1:	
	/ restore regs
	movl	8(%esp),%edx	/ *regs
	movl	REG_EBP(%edx),%ebp
	movl	REG_EBX(%edx),%ebx
	movl	REG_ESI(%edx),%esi
	movl	REG_EDI(%edx),%edi
	movl	REG_EBP+4(%edx),%esp
	addl	$12,%esp
	ret

	.type _xfer_dst_fault_jmp,@function
	.size _xfer_dst_fault_jmp,.-_xfer_dst_fault_jmp
	.type _xfer_src_fault_jmp,@function
	.size _xfer_src_fault_jmp,.-_xfer_dst_fault_jmp
	
	
