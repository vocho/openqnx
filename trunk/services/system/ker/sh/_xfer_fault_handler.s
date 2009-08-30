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
	
	.global xfer_dst_handlers
	.global xfer_src_handlers
	
	.global xfer_fault_handlers
	.global xfer_async_handlers
	
	.extern	xfer_restart
	.extern	xfer_async_restart

	.section ".data"
xfer_dst_handlers:
	.long	_xfer_dst_fault_jmp
	.long	xfer_restart
	
xfer_src_handlers:
	.long	_xfer_src_fault_jmp
	.long	xfer_restart
	
xfer_fault_handlers:
	.long	_xfer_fault_jmp
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
	not		r6,r6
	
 #
 # int _xfer_src_fault_jmp(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) 
 #
 # handler for src fault in msg xfer
 #
 _xfer_src_fault_jmp:
# restore saved regs according to ABI
	mov.l	_xfer_fault_jmp_01,r0
	tst		r0,r6

.macro RESTORE_REG	
	mov.l	@(4,r5),r1	
	mov.l	@(8,r5),r2	
	mov.l	@(12,r5),r3	
	mov.l	@(16,r5),r4	
	mov.l	@(20,r5),r0	
	mov.l	@(24,r5),r6	
	mov.l	@(28,r5),r7	
	mov.l	@(32,r5),r8	
	mov.l	@(36,r5),r9	
	mov.l	@(40,r5),r10	
	mov.l	@(44,r5),r11	
	mov.l	@(48,r5),r12	
	mov.l	@(52,r5),r13
	mov.l	@(56,r5),r14	
	add		#64+8,r5
	ldc.l	@r5+,gbr
	lds.l	@r5+,mach
	lds.l	@r5+,macl
	lds.l	@r5+,pr

# leave the ker stack pointer to the last to adjust
	add		#-(64+8+16),r5
	mov.l	@(60,r5),r15

	mov		r0,r5
.endm

	RESTORE_REG	

	bt		1f
	rts
	mov		#XFER_DST_FAULT,r0
1:	 
	rts
	mov		#XFER_SRC_FAULT,r0

	.align 2
_xfer_fault_jmp_01:
	.long	SIGCODE_STORE	

	.type _xfer_dst_fault_jmp,@function
	.size _xfer_dst_fault_jmp,.-_xfer_dst_fault_jmp
	.type _xfer_src_fault_jmp,@function
	.size _xfer_src_fault_jmp,.-_xfer_dst_fault_jmp
	
 #
 # int _xfer_fault_jmp(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) 
 #
 #	handler for fault in msg xfer, always return -1
 #
_xfer_fault_jmp:
	RESTORE_REG	
 	rts
	mov		#-1,r0

	
