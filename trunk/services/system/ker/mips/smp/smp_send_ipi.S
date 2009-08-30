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

	.set noreorder
	.set mips3
	.extern	send_ipi_rtn,4
	.extern	_syspage_ptr,4
	.extern ipicmds,4*PROCESSORS_MAX

/*
 * smp_send_ipi.S
 *  Send an IPI to another processor. Pointer to actual routine kept
 *  in 'send_ipi_rtn' variable.
 */
 
FRAME(send_ipi,sp,0,ra)
	lw		t9,send_ipi_rtn
	move	a2,a1
	move	a1,a0
	lw		a0,_syspage_ptr
	la		a3,ipicmds
	sll		t0,a1,2
	j		t9
	 addu	a3,t0
ENDFRAME(send_ipi)
