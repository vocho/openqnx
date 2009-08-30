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
#include "getcpu.h"

	.set noreorder
	.set mips3


/*
 * _smpstart
 *	The very first kernel code executed by an application processor on an
 *  SMP system.
 *  A1 contains the CPU number, we stick in in the CP0_XCONTEXT register
 *  to insulate ourselves from differences in SMP chip implementations
 */

	.extern	_gp
	.extern init_smp
	.extern startup_stack
	
FRAME(_smpstart,sp,0,ra)
	/*
	 * Set GP to the location assigned by "ld".  Our short data
	 * lives there.
	 */
	la	gp,_gp

	/*
	 * Time for our own stack
	 */
	la	sp,startup_stack+STARTUP_STACK_SIZE- STACK_INITIAL_CALL_CONVENTION_USAGE	
	
	SETCPU(a1)

	/*
	 * Call init_smp; never returns
	 */
	j	init_smp	
	 move	ra,zero
ENDFRAME(_smpstart)
