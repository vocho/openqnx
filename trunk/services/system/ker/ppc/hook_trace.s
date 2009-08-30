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

.include "ppc/util.ah"
.include "asmoff.def"

	.text

	# Handle calling a hook routine from in the kernel

	##
	## struct sigevent *hook_trace(void *, INTERRUPT *)
	##

routine_start hook_trace, 1
	stwu 	%r1,-32(%r1)
	mflr 	0
	stw 	%r0,36(%r1)
	
	lwz		%r0,INTR_HANDLER(%r4)	# get hook routine address
	mtctr	%r0
	
	# load up thread short data ptrs
	lwz		%r2, 0*PPCINT+INTR_CPU(%r4)	
	lwz		%r13,1*PPCINT+INTR_CPU(%r4)	
	
	bctrl								# invoke user routine
	
	# reload up kernel small data area ptr
	lis		%r13,_SDA_BASE_@ha			
	la		%r13,_SDA_BASE_@l(%r13)
	lis		%r2,_SDA2_BASE_@ha			
	la		%r2,_SDA2_BASE_@l(%r2)
	
	lwz 	%r11,0(%r1)
	lwz 	%r0,4(%r11)
	mtlr 	%r0
	mr 		%r1,%r11
	blr
routine_end hook_trace
