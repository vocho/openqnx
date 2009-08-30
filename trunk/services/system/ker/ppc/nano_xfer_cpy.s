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

.include "asmoff.def"
.include "ppc/util.ah"

	.extern xfer_src_handlers
	.extern xfer_dst_handlers
	.extern xfer_handlers 

# only ppc400 and ppcle could not do unaligned copy by H/W
.ifdef VARIANT_400 
.set	XFER_CPY_CHECK_ALIGNMENT,1
.endif

.ifdef VARIANT_le
.set	XFER_CPY_CHECK_ALIGNMENT,1
.endif
 
.macro _XFER_MEMCPY,dst,src,len,tmp
.ifdef	XFER_CPY_CHECK_ALIGNMENT 
# check alignment
	or		&tmp,&dst,&src
	andi.	&tmp,&tmp,3
	bne-	1002f
.endif

# move by words
	cmplwi	&len,4
	blt-	1002f
1001:	
	lwz		&tmp,0(&src)
	addi	&src,&src,4
	addi	&len,&len,-4
	stw		&tmp,0(&dst)	
	cmplwi	&len,4
	addi	&dst,&dst,4
	bge+	1001b
	
1002:		
	cmplwi	&len,0
	beq-	1004f
	
# move by bytes
1003:
	lbz		&tmp,0(&src)
	addi	&src,&src,1
	addic.	&len,&len,-1
	stb		&tmp,0(&dst)	
	addi	&dst,&dst,1
	bne+	1003b

1004:	
.endm

	.global xfer_memcpy
#
# int xfer_memcpy(void *dst, const void *src, size_t len) 
#
#	cpy between buffers for short msg xfer
#
#	Parameters: 
#		r3		dst
#		r4		src
#		r5		len
#
routine_start xfer_memcpy,0

# set up fault handlers
	la 		%r0,xfer_src_handlers@sdarel(%r13)
	stw 	%r0,xfer_handlers@sdarel(%r13)
	
	_XFER_MEMCPY %r3,%r4,%r5,%r0
	
	sub		%r3,%r3,%r3
	stw 	%r3,xfer_handlers@sdarel(%r13)
	blr
routine_end xfer_memcpy

	.global xfer_cpy_diov
#
# int xfer_cpy_diov(THREAD* thpd, IOV *dst, uint8_t *saddr, int dparts, unsigned slen) 
#
# cpy from a buffer to an iov (or buffer) for short msg xfer
#
#	Parameters: 
#		r3		thpd
#		r4		dst
#		r5		saddr
#		r6		dparts
#		r7		slen
#	Reg usage:
#		r8		BOUNDRY
#	(for buffer to iov)
#		r9		dst addr
#		r10		dst len
#
xfer_cpy_diov:
# check dparts
	cmpwi	%r6,0
	bge		4f
	
# negative, direct buffer

	neg		%r6,%r6

# copy msg
	la 		%r0,xfer_src_handlers@sdarel(%r13)
	cmplw	%r6,%r7
	blt		2f
	mr		%r6,%r7
2:	
# set src handler
	stw 	%r0,xfer_handlers@sdarel(%r13)

	_XFER_MEMCPY %r4,%r5,%r6,%r0
99:
# return success	
	sub		%r3,%r3,%r3
	stw 	%r3,xfer_handlers@sdarel(%r13)
	blr

4:
# no dparts, return
	la 		%r0,xfer_dst_handlers@sdarel(%r13)
	beq-	99b

# set dst handler	
	stw 	%r0,xfer_handlers@sdarel(%r13)
	
# get dst iov	
	lwz		%r9,0(%r4)
	lwz		%r10,4(%r4)
	
	# set src handler	
	la 		%r0,xfer_src_handlers@sdarel(%r13)

	cmplw	%r7,%r10
	stw 	%r0,xfer_handlers@sdarel(%r13)
	bgt-	1f
	
	# within this iov
	_XFER_MEMCPY %r9,%r5,%r7,%r0

	# return success 
	sub		%r3,%r3,%r3
	stw 	%r3,xfer_handlers@sdarel(%r13)
	blr

1:
	# not within one iov. Rare case.
	sub		%r7,%r7,%r10
	
	_XFER_MEMCPY %r9,%r5,%r10,%r0

	addic.	%r6,%r6,-1
	addi	%r4,%r4,8
	b		4b
	
	.type xfer_cpy_diov,@function
	.size xfer_cpy_diov,.-xfer_cpy_diov
	
	.global _mem_cpy
#
# int _mem_cpy(void *dst, const void *src, size_t len) 
#
#	cpy between buffers for short msg xfer
#
#	Parameters: 
#		r3		dst
#		r4		src
#		r5		len
#
routine_start _mem_cpy,0

	_XFER_MEMCPY %r3,%r4,%r5,%r0

	sub		%r3,%r3,%r3	
	blr
routine_end _mem_cpy
