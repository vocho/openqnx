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

#include "asmoff.def"
#include <mips/asm.h>
#include <mips/cpu.h>
#include "loadstore.h"

	.set	noreorder
	.set	MIPSARCH
	
	.global xfer_dst_handlers
	.global xfer_src_handlers
	.global xfer_async_handlers
	.extern xfer_restart	
	.extern xfer_async_restart	

	.data
	.align 2
xfer_dst_handlers:
	.long	_xfer_dst_fault_jmp
	.long	xfer_restart	
	
xfer_src_handlers:
	.long	_xfer_src_fault_jmp
	.long	xfer_restart	
	
#xfer_fault_handlers:
#	.long	_xfer_dst_fault_jmp
#	.long	0
	
xfer_async_handlers:
	.long	_xfer_src_fault_jmp
	.long	xfer_async_restart	
	
	.text
	.align	2
	.global _xfer_dst_fault_jmp
	
 #
 # int _xfer_dst_fault_jmp(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) 
 #
 #	handler for dst fault in msg xfer
 #
 _xfer_dst_fault_jmp:
	not	a2,a2
	
 #
 # int _xfer_src_fault_jmp(THREAD *thp, CPU_REGISTERS *regs, unsigned flags) 
 #
 # handler for src fault in msg xfer
 #
 _xfer_src_fault_jmp:
# restore saved regs according to ABI
	RESTORE_ONE_REG(ra,RA,0,a1)
	RESTORE_ONE_REG(s0,S0,0,a1)
	RESTORE_ONE_REG(s1,S1,0,a1)
	RESTORE_ONE_REG(s2,S2,0,a1)
	RESTORE_ONE_REG(s3,S3,0,a1)
	RESTORE_ONE_REG(s4,S4,0,a1)
	RESTORE_ONE_REG(s5,S5,0,a1)
	RESTORE_ONE_REG(s6,S6,0,a1)
	RESTORE_ONE_REG(s7,S7,0,a1)
	RESTORE_ONE_REG(s8,S8,0,a1)
# restore sp last		
	RESTORE_ONE_REG(sp,SP,0,a1)
	
	li		v1,SIGCODE_STORE
	and		v0,v1,a2
	beq		v0,zero,1f
	 nop
	
	jr		ra
	 li		v0,XFER_DST_FAULT
1:	 
	jr		ra
	 li		v0,XFER_SRC_FAULT

	.type _xfer_dst_fault_jmp,@function
	.size _xfer_dst_fault_jmp,.-_xfer_dst_fault_jmp
	.type _xfer_src_fault_jmp,@function
	.size _xfer_src_fault_jmp,.-_xfer_src_fault_jmp
