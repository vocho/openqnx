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
# nano_xfer_check.s
#	Routines for check iov boundry and check read
#

	.include "asmoff.def"
	
	.global xfer_memchk
	.global xfer_memprobe
#
# int xfer_memchk(uintptr_t bound, const IOV *iov, size_t parts) 
#
#	Check the boundry of an iov array
#		r4	bound
#		r5	iov
#		r6	parts
#		r1	ptr
#		r2	len
#		r0	tmp/return
#
	.align	5
xfer_memchk:
	mov.l	@r5,r1
	mov.l	@(4,r5),r2
	add		#8,r5
	
	mov		r1,r0
	add		r2,r0	
	add		#-1,r0
	cmp/hi	r0,r1
	bt		11f
	cmp/hs	r4,r0
	bt		11f
9:	
	dt		r6
	bf		xfer_memchk
	
	# succeed
	rts
	sub		r0,r0

11: # is len zero?
	tst		r2,r2
	bt		9b	

	# boundry error
	rts
	mov		#-1,r0
	
	.type xfer_memchk,@function
	.size xfer_memchk,.-xfer_memchk

#
# int xfer_memprobe(uintptr_t addr) 
#
#	Check the boundry of an iov array
#		r4	addr
#		r0	tmp/return
#
	.align	5
xfer_memprobe:

	mov.b	@r4,r0
	
	rts
	sub		r0,r0
	

