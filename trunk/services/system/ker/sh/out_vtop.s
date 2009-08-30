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

#
# outside.S
#	Routines for calling into the kernel from "outside"
#
#
	.text

	.extern kdebug_vtop

#
# outside_vaddr_to_paddr()
#
.global outside_vaddr_to_paddr
outside_vaddr_to_paddr:
	sts.l	pr,@-r15
	mov.l	outside_vaddr_to_paddr_1,r0
	jsr		@r0
	nop
	lds.l	@r15+,pr
	rts
	nop
	.align 2
outside_vaddr_to_paddr_1:
	.long	kdebug_vtop
