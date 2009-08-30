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
# out_*.S
#	Routines for calling into the kernel from "outside"
#
#
	.text

	.extern trace_event
	.extern	_SDA_BASE_
	.extern	_SDA2_BASE_

#
# outside_trace_event()
#
.global outside_trace_event
outside_trace_event:
	stwu 	%r1,-32(%r1)
	mflr 	0
	stw 	%r2,12(%r1)
	stw 	%r13,16(%r1)
	stw 	%r0,36(%r1)
	lis		%r13,_SDA_BASE_@ha				# load up small data area ptr
	la		%r13,_SDA_BASE_@l(%r13)
	lis		%r2,_SDA2_BASE_@ha				# load up small data area ptr
	la		%r2,_SDA2_BASE_@l(%r2)
	bl 		trace_event
	lwz 	%r11,0(%r1)
	lwz 	%r0,4(%r11)
	mtlr 	%r0
	lwz 	%r2,-20(%r11)
	lwz 	%r13,-16(%r11)
	mr 		%r1,%r11
	blr
