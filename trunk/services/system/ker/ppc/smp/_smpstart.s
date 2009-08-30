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

	.global _smpstart
	.extern init_smp
	.extern startup_stack
	.extern _SDA_BASE_
	.extern _SDA2_BASE_
	
	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "context.ah"
	
	.text

routine_start _smpstart, 1
	#
	# init kernel stack
	#
	lis		%r1,STARTUP_STACK_SIZE -STACK_INITIAL_CALL_CONVENTION_USAGE + startup_stack@ha	
	la  	%r1,STARTUP_STACK_SIZE -STACK_INITIAL_CALL_CONVENTION_USAGE + startup_stack@l(%r1)
	subi	%r1,%r1,16
	
	#
	# init small data area ptrs
	#
	loada	%r13,_SDA_BASE_			
	loada	%r2,_SDA2_BASE_

	#
	# Set inkernel bit on CPU
	#
	mfmsr	%r3
	ori		%r3,%r3,PPC_MSR_INKERNEL_BIT
	mtmsr	%r3

	b		init_smp
routine_end _smpstart
