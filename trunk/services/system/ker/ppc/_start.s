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

	.global	_cstart_
	.extern _main
	.extern startup_stack
	.extern _SDA_BASE_
	.extern _SDA2_BASE_
	.extern bootstrap
	
	.include "asmoff.def"
	.include "context.ah"
	.include "ppc/util.ah"

.text
.long	IFS_BOOTSTRAP_SIGNATURE
.long	bootstrap
routine_start _start, 1
_cstart_:
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
	
.ifdef VARIANT_smp
	mfmsr	%r0
	SET_CPUINKERNEL	%r0
	mtmsr	%r0
.endif
	
   	# syspage pointer is in R3 - pass it to _main.
	b		_main					# let's get the show on the road!
routine_end _start
