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
# nano_xfer_pulse.s
#	Routines for pulse xfer
#
#include "asmoff.def"
#include <mips/asm.h>
#include <asm_variant.h>

	.set	noreorder
	
	.extern xfer_handlers,4

#
# our xfer handler structure
#
	.data
	.align 2
	.extern	_xfer_dst_fault_jmp
xfer_pulse_handlers:
	.long _xfer_dst_fault_jmp
	.long 0

	.text
	.align	4

#
# xferpulse(dthp, IOV *dst, int parts, uint32 code, uint32 value, int32 scoid)
#
#	Parameters: 
#		a0			dthp
#		a1			dst
#		a2			parts
#		a3			code
#		stack+16	value
#		stack+20	scoid
#		t0			daddr
#		t1			dlen	
#		t2,t3		temp
#
FRAME(xferpulse,sp,0,ra)
	# set fault handler address
	la 		v0,xfer_pulse_handlers
	bltz	a2,1f
	 sw	 	v0,xfer_handlers

	# IOV
	# get dst iov	
	lw		t0,IOV_ADDR(a1)
	j		2f
	 lw		t1,IOV_LEN(a1)
1:
	addu	t0,zero,a1
	subu	t1,zero,a2	
2:
	# make sure dst has enough room to hold a pulse
	addiu	v0,t1,-PULSE_SIZE
	bltz	v0,100f
	 lw		v0,PROCESS(a0)
	lw		t2,BOUNDRY_ADDR(v0)
	
	# check boundry
	addu	v0,t0,t1
	addiu	v0,v0,-1
	sltu	v1,v0,t0
	bne		v1,zero,100f	
	 nop
	sltu	v1,t2,v0
	bne		v1,zero,100f		
	 nop

	# xfer pulse
	#type and sub type, all zero according to neutrino.h
	sh		zero,0(t0)
	lw		t2,16(sp)
	sh		zero,2(t0)
	lw		t3,20(sp)
	sb		a3,4(t0)
	sw		t2,8(t0)
	sw		t3,12(t0)
	
	# ret
	sw	 	zero,xfer_handlers
	jr		ra
	 li		v0,0

100:
	sw	 	zero,xfer_handlers
	jr		ra
	 li		v0,XFER_DST_FAULT
	
ENDFRAME(xferpulse)

