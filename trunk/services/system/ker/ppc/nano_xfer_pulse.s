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

.include "ppc/util.ah"
.include "asmoff.def"

	.extern xfer_src_handlers

#
# Assembly-optimized xferpulse. The main reason we do this is because
# we can handle the faults more cleanly and efficiently.
#
# xferpulse(dthp, IOV *dst, int parts, uint32 code, uint32 value, int32 scoid)
#

routine_start xferpulse,1
	la 		%r0,xfer_src_handlers@sdarel(%r13)
	stw 	%r0,xfer_handlers@sdarel(%r13)
	mr. 	%r5,%r5
	bge		2f
	mr 		%r11,%r4
	neg 	%r4,%r5
	b 		3f
2:
	lwz 	%r11,IOV_ADDR(%r4)
	lwz 	%r4,IOV_LEN(%r4)
3:
	cmplwi 	cr1,%r4,PULSE_SIZE-1
	ble-	cr1,8f
	add 	%r0,%r11,%r4
	addic 	%r0,%r0,-1
	cmplw 	cr1,%r11,%r0
	bgt-	cr1,8f
	lwz 	%r9,PROCESS(%r3)
	lwz 	%r0,BOUNDRY_ADDR(%r9)
	cmplw 	cr1,%r11,%r0
	bge+	cr1,9f
8:
	li 		%r0,0
	stw 	%r0,xfer_handlers@sdarel(13)
	li 		%r3,XFER_DST_FAULT
	b 		10f
9:
	li 		%r0,0
	sth 	%r0,0(%r11)
	sth 	%r0,2(%r11)
	stb 	%r6,4(%r11)
	stw 	%r7,8(%r11)
	stw 	%r8,12(%r11)
	stw 	%r0,xfer_handlers@sdarel(%r13)
	mr 		%r3,%r0
10:
	blr
routine_end xferpulse
