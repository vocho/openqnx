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

#
# _xfer_fault_handler.s
#	Routines for error handling for msg xfer 
#

	.include "asmoff.def"
	.include "ppc/util.ah"
	
	.global xfer_src_handlers
	.global xfer_dst_handlers
	.global xfer_fault_handlers
	.global xfer_async_handlers

	.extern	xfer_restart
	.extern	xfer_async_restart
	
	.section ".sdata"
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
	
	.section ".text"
	.global _xfer_dst_fault_jmp
	
 #
 # int _xfer_dst_fault_jmp(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) 
 #
 #	handler for dst fault in msg xfer
 #
 _xfer_dst_fault_jmp:
	not		%r5,%r5
	
 #
 # int _xfer_src_fault_jmp(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) 
 #
 # handler for src fault in msg xfer
 #
 _xfer_src_fault_jmp:

.ifdef VARIANT_600
.set NEED_VMX_RESTORE,	1
.endif

.ifdef VARIANT_900
.set NEED_VMX_RESTORE,	1
.endif

.ifdef NEED_VMX_RESTORE
# need to restore vmx register
	mr		%r23,%r3
	mr		%r24,%r4
	mr		%r25,%r5

	mr		%r3,%r4
	bl		xfer_fault_restore_vmx
	
	mr		%r3,%r23
	mr		%r4,%r24
	mr		%r5,%r25
.endif

# restore saved regs according to ABI
	lwz		%r0,REG_LR(%r4)
	lmw		%r14,REG_GPR+14*4(%r4)
	mtlr	%r0
	lwz		%r0,REG_CTR(%r4)
	mtctr	%r0
	lwz		%r0,REG_XER(%r4)
	mtxer	%r0
		
	bittst	%r0,%r5,SIGCODE_STORE
	beq		1f
	li		%r3,XFER_DST_FAULT
    b		2f
1:	 
	li		%r3,XFER_SRC_FAULT
2:
	lwz		%r0,REG_CR(%r4)
	mtcr	%r0
	lwz		%r1,REG_GPR+4(%r4)
	blr

	.type _xfer_dst_fault_jmp,@function
	.size _xfer_dst_fault_jmp,.-_xfer_dst_fault_jmp
	.type _xfer_src_fault_jmp,@function
	.size _xfer_src_fault_jmp,.-_xfer_dst_fault_jmp
	
	
