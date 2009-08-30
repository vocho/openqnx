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

	.extern	startup_stack
	.extern	_main
	.extern	bootstrap
	
	.globl	_cstart_
	.globl	_start

#include	"asmoff.def"

	.text
	
.long	IFS_BOOTSTRAP_SIGNATURE
.long	bootstrap
_cstart_:
_start:
	movw	%ds,%ax
	movw	%ax,%ss
	mov		$startup_stack+STARTUP_STACK_SIZE-STACK_INITIAL_CALL_CONVENTION_USAGE,%esp
	cld
	push	%edi	/ syspage is passed in EDI
	call	_main	/ never returns
