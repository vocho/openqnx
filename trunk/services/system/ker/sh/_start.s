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
	.extern bootstrap
	
	.include "asmoff.def"
	.include "sh/util.ah"

.text
.long	IFS_BOOTSTRAP_SIGNATURE
.long	bootstrap
routine_start _start, 1
_cstart_:
	mov.l	start_main,r1
	# get syspage pointer
	stc	r0_bank,r4
	#
	# init kernel stack
	#
	mov.l	start_stack,r15
.ifdef VARIANT_smp
	#
	# SMP kernel stores address of inkernel variable in r7_bank
	#
	mov.l	start_inkernel,r0
	ldc		r0,r7_bank
.endif
	jmp		@r1		! let's get the show on the road!
	nop
	.align 2
start_stack:
	.long	STARTUP_STACK_SIZE -STACK_INITIAL_CALL_CONVENTION_USAGE + startup_stack	
start_main:
	.long	_main
.ifdef VARIANT_smp
start_inkernel:
	.long	inkernel
.endif
		
routine_end _start
