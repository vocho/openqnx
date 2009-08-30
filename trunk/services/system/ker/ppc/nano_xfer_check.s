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
#		r3	bound/return
#		r4	iov
#		r5	parts
#	Registers used
#		r6	ptr
#		r7	len
#		r8	tmp
#
	.align	5
xfer_memchk:
1:
	lwz		%r6,0(%r4)
	lwz		%r7,4(%r4)
	addi	%r4,%r4,8

	add		%r8,%r6,%r7
	addi	%r8,%r8,-1
	
	cmplw	%r8,%r6
	blt-	11f
	cmplw	%r6,%r3
	blt-	11f
9:	
	addic.	%r5,%r5,-1
	bne+	1b
	
	# succeed
	li		%r3,0
	blr

11: 
	# is len zero?
	cmplwi	%r7,0
	beq+	9b	

	# boundry error
	li		%r3,-1
	blr
	
	.type xfer_memchk,@function
	.size xfer_memchk,.-xfer_memchk

#
# int xfer_memprobe(uintptr_t addr) 
#
#	Check the boundry of an iov array
#		r3	addr/return
#
	.align	5
xfer_memprobe:
	lbz		%r3,0(%r3)
	li		%r3,0
	blr
	
	

