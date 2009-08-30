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

/*
 * Entry point for secondary processors in an SMP system
 */

	.text

	.globl _smpstart

_smpstart:
	/*
	 * Set up stack
	 *
	 * The code in idle() ensures that only one cpu at a time comes through
	 * this code, so it's safe to use the global startup_stack[] storage.
	 */
	ldr		sp, Lstartup_stack
	add		sp, sp, #STARTUP_STACK_SIZE- STACK_INITIAL_CALL_CONVENTION_USAGE

	/*
	 * init_smp never returns
	 */
	b		init_smp

Lstartup_stack:	.word	startup_stack

	.type _smpstart,function
	.size _smpstart,.-_smpstart
