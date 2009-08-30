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
	
	.extern xfer_src_handlers,4
	.extern xfer_dst_handlers,4
	.extern xfer_handlers,4 


.macro _XFER_MEMCPY,dst,src,len,tmp,tflag
	or		&tmp,&dst,&src
	andi	&tmp,3 
	bne		&tmp,zero,1002f
	 sltiu	&tflag,&len,4
	bne		&tflag,zero,1002f
	 nop 
1001: 
	lw	&tmp,0(&src)
	addiu	&len,&len,-4 
	addiu	&src,&src,4 
	sltiu	&tflag,&len,4 
	sw		&tmp,0(&dst) 
	beq		&tflag,zero,1001b
	 addiu	&dst,&dst,4 
1002: 
	beq		&len,zero,1004f
	 nop
1003: 
	lbu		&tmp,0(&src)
	addiu	&len,&len,-1 
	addiu	&src,&src,1 
	sb		&tmp,0(&dst)	
	bne		&len,zero,1003b
	 addiu	&dst,&dst,1 
1004: 
.endm

#
# int xfer_memcpy(void *dst, const void *src, size_t len) 
#
#	cpy between buffers for short msg xfer
#
#	Parameters: 
#		a0		dst
#		a1		src
#		a2		len
#
FRAME(xfer_memcpy,sp,0,ra)
	# set up fault handlers
	lw		t0,xfer_handlers
	bne		t0,zero,1f
	 la 	v0,xfer_src_handlers
	sw 		v0,xfer_handlers

1:
	_XFER_MEMCPY a0,a1,a2,v0,v1
	
	sw 		t0,xfer_handlers
	jr		ra
	 add	v0,zero,zero
ENDFRAME(xfer_memcpy)

#
# int xfer_cpy_diov(THREAD* thpd, IOV *dst, uint8_t *saddr, int dparts, unsigned slen) 
#
# cpy from a buffer to an iov (or buffer) for short msg xfer
#
#	Parameters: 
#		a0			thpd
#		a1			dst
#		a2			saddr
#		a3			dparts
#		stack+16	slen
#	Reg usage:
#		t0		BOUNDRY
#		t1		slen
#	(for buffer to iov)
#		t2		dst addr
#		t3		dst len
#
FRAME(xfer_cpy_diov,sp,0,ra)
	lw		t0,PROCESS(a0)
	lw		t1,16(sp)		# load slen

# check dparts
	bgez	a3,4f
# get boundry
	 lw		t0,BOUNDRY_ADDR(t0)
	
# negative, direct buffer
	negu	a3,a3
	sltu	v1,a3,t1

# copy msg
	bne		v1,zero,2f
	 la		v0,xfer_src_handlers
	add		a3,t1,zero
2:	
# set src handler
	sw	 	v0,xfer_handlers

	_XFER_MEMCPY a1,a2,a3,v0,v1
99:
# return success	
	sw 		zero,xfer_handlers
	jr		ra
	 add	v0,zero,zero

4:
# no dparts, return
	la 		v0,xfer_dst_handlers
	beq		a3,zero,99b
# set dst handler	
	 sw 	v0,xfer_handlers
	
# get dst iov	
	lw		t2,0(a1)
	lw		t3,4(a1)
	
	# set src handler	
	 la 	v0,xfer_src_handlers

	sltu	v1,t3,t1
	bne		v1,zero,1f
	 sw		v0,xfer_handlers
	
	# within this iov
	_XFER_MEMCPY t2,a2,t1,v0,v1

	# return success 
	sw 		zero,xfer_handlers
	jr		ra
	 add	v0,zero,zero

1:
	# not within one iov. Rare case.
	subu	t1,t1,t3
	
	_XFER_MEMCPY t2,a2,t3,v0,v1

102:
	addiu	a3,a3,-1
	j		4b
	 addi	a1,a1,8
ENDFRAME(xfer_cpy_diov)
