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

.include "asmoff.def"
.include "sh/util.ah"

#
# Entry point for secondary processors in an SMP system
#

	.text

routine_start	_smpstart, 1
	#
	# Set up stack.
	#
	# The code in idle() ensures that only one cpu at a time comes through
	# this code, so it's safe to use the global startup_stack[] storage.
	#
	mov.l	.Lstartup_stack, r15

	#
	# r7_bank holds address of inkernel variable
	#
	mov.l	.Linkernel, r0
	ldc		r0, r7_bank

	#
	# init_smp never returns
	#
	mov.l	.Linit_smp, r0
	jmp		@r0
	nop

	.align 2
.Lstartup_stack:	.long	startup_stack + STARTUP_STACK_SIZE - STACK_INITIAL_CALL_CONVENTION_USAGE
.Linit_smp:			.long	init_smp
.Linkernel:			.long	inkernel

routine_end		_smpstart
