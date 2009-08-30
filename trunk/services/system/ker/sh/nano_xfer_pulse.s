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

.include "asmoff.def"
.include "sh/util.ah"

#
# our xfer handler structure
#
	.section ".text"
	.align 2
	.extern	xfer_fault_handlers


#
# xferpulse(dthp, IOV *dst, int parts, uint32 code, uint32 value, int32 scoid)
#
# REGS: r1-@xfer_fault r4-dthp r5-dst>daddr r6-parts>len r7-code @r15-value @(4+r15)-scoid 
.align	5
routine_start xferpulse,1
	# set fault handler address
	mov.l	xferpulse_01,r1
	mov.l	xferpulse_02,r2
	
	cmp/pz	r6
	bf.s	2f
	mov.l	r2,@r1

	# IOV
	mov.l	@(4,r5),r6
	bra		4f
	mov.l	@r5,r5

2:
	neg		r6,r6
4:	
	# check buffer size
	mov		#PULSE_SIZE,r2
	cmp/hs	r2,r6
	bf		99f

	# get boundry
	mov.l	xferpulse_03,r0
	mov.l	@(r0,r4),r2
	mov.l	xferpulse_04,r0
	mov.l	@(r0,r2),r4
	
	# check boundry
	add		r5,r6
	add		#-1,r6
	cmp/hs	r5,r6
	bf		99f
	cmp/hs	r6,r4
	bf		99f

	# xfer pulse
	#type and sub type, all zero according to neutrino.h
	sub		r0,r0
	mov.l	@r15,r2
	mov.l	@(4,r15),r3
	mov.w	r0,@r5 
	mov.w	r0,@(2,r5)
	mov		r7,r0
	mov.b	r0,@(4,r5) 
sub		r0,r0
	mov.l	r2,@(8,r5)
	mov.l	r3,@(12,r5)
	
	# ret
	rts
	mov.l	r0,@r1

99:
	sub		r0,r0
	mov.l	r0,@r1
	rts
	mov		#XFER_DST_FAULT,r0
	
.align	2
xferpulse_01:
	.long	xfer_handlers 
xferpulse_02:
	.long	xfer_fault_handlers
xferpulse_03:
	.long	PROCESS	
xferpulse_04:
	.long	BOUNDRY_ADDR   

routine_end xferpulse
