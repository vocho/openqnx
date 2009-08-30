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
# nano_xfer_cpy.s
#	Routines for short msg xfer cpy
#

#include "asmoff.def"
#include <mips/asm.h>
#include <asm_variant.h>

	.set	noreorder
	.text
	.align	4
	
#
#
# int	(xfer_memchk)(uintptr_t bound, const IOV *iov, size_t iov_len) 
#
#	Parameters: 
#		a0		bound	
#		a1		iov
#		a2		iov_len
#		t0		base
#		t1		size
#
FRAME(xfer_memchk,sp,0,ra)
	lw		t0,0(a1)
	lw		t1,4(a1)
	
	addu	t2,t0,t1
	addiu	t2,t2,-1
	sltu	v0,t2,t0
	bne		v0,zero,9f
	 nop
	sltu	v0,a0,t2
	bne		v0,zero,9f	
	 nop

8:	
	addiu	a2,a2,-1
	bne		a2,zero,xfer_memchk
	 addiu	a1,a1,8

	jr		ra
	 add	v0,zero,zero
	
9:
	beq		t1,zero,8b
	 nop
	
	jr		ra
	 li		v0,-1	
ENDFRAME(xfer_memchk)

#
# int	(xfer_memprobe)(void *ptr) {
#
# check the readability of the byte pointed by ptr 
#
#	Parameters: 
#		a0			ptr	
#
FRAME(xfer_memprobe,sp,0,ra)
	lb		v0,0(a0)
	jr		ra
	 li		v0,0
ENDFRAME(xfer_memprobe)
