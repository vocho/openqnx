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
	
	.extern xfer_handlers 
	.extern xfer_fault_handlers
	.global xferlen
#
# int xferlen(THREAD* thp, const IOV *iov, size_t parts) 
#
#	Calculate the length of the iov 
#	return -1 when address pointed by iov is not accessable 	
#		r4	thp
#		r5	iov
#		r6	parts
#		r2	len
#		r0	return
#
	.align	5
xferlen:
	cmp/pz	r6
	mov.l	xferlen_01,r1
	bf		3f
	mov.l	xferlen_02,r2
	tst		r6,r6
	xor		r0,r0
	bt.s	2f
	mov.l	r2,@r1
1:
	mov.l	@(4,r5),r2
	dt		r6
	add		#8,r5
	bf.s	1b	
	add		r2,r0
	
2:
	xor		r2,r2
	rts
	mov.l	r2,@r1
3:
	rts
	neg		r6,r0	
xferlen_01:
	.long	xfer_handlers 
xferlen_02:
	.long	xfer_fault_handlers 
	
	.type xferlen,@function
	.size xferlen,.-xferlen


