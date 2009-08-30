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

#include "asmoff.def"
#include <mips/asm.h>
#include "loadstore.h"

	.set noreorder
	.set MIPSARCH
	.text
	.align	4
	
	.extern _gp

	# Handle calling a hook routine from in the kernel

	##
	## struct sigevent *hook_trace(void *, INTERRUPT *)
	##
	
   	.set noreorder
	.set MIPSARCH
	.globl	hook_trace
	
FRAME(hook_trace,sp,0,ra)
	addiu	sp,sp,-32
	sw		ra,28(sp)
	lw		t9,INTR_HANDLER(a1)		# get hook routine address 
	jal		t9						# and invoke it (use t9 for PIC)
	 LS_INSTR(l) gp,INTR_CPU+REG_POS_BASE(a1) # load thread short data ptr
	la		gp,_gp
	lw		ra,28(sp)
	j		ra
	 addiu	sp,sp,32
ENDFRAME(hook_trace)
