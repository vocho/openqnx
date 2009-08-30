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

	.text

	.globl send_ipi

send_ipi:
	ldr		r3, =_syspage_ptr
	mov		r2, r1
	mov		r1, r0
	ldr		r0, [r3]
	ldr		r3, =ipicmds
	ldr		ip, =send_ipi_rtn
	add		r3, r3, r1, lsl #2
	ldr		pc, [ip]

	.type	send_ipi, function
	.size	send_ipi, . - send_ipi
