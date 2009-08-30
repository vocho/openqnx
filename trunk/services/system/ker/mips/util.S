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

/*
 * util.S
 *	Utility functions best implemented in assembly, usually for speed
 */
#include "asmoff.def"
#include <mips/asm.h>
#include "getcpu.h"

	.set noreorder

/*
 * _start
 *	The very first code executed as Neutrino starts on a R4000
 *  A0 contains the kernel syspage pointer, it's passed to _main.
 */
 
	.extern	_gp
	.extern _main
	.extern startup_stack
	.extern bootstrap
	
.long IFS_BOOTSTRAP_SIGNATURE
.long bootstrap
FRAME(_start,sp,0,ra)
	/*
	 * Set GP to the location assigned by "ld".  Our short data
	 * lives there.
	 */
	la	gp,_gp

	/*
	 * Time for our own stack
	 */
	la	sp,startup_stack+STARTUP_STACK_SIZE- STACK_INITIAL_CALL_CONVENTION_USAGE	
	
   	SETCPU(zero)

	/*
	 * Call _main; never returns
	 */
	j	_main
	 move	ra,zero
ENDFRAME(_start)
