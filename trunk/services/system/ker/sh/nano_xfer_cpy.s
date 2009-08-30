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
	
	.extern xfer_src_handlers
	.extern xfer_dst_handlers
	.extern xfer_handlers 

# use registers dst, src, nbytes, t1, r0
.macro _MEMCPY4,dst,src,nbytes,t1	
# check if same aligment at 4
mov		&dst,r0
mov		#4,&t1
or		&src,r0
tst		#3,r0
bf		50f
cmp/hs	&t1,&nbytes
bf		50f

# long word move
51:
mov.l	@&src+,r0
add	#-4,&nbytes
mov.l	r0,@&dst
cmp/hs	&t1,&nbytes	
bt.s	51b
add	#4,&dst	

# move by bytes
50:
tst		&nbytes,&nbytes
bt		52f
53:
mov.b	@&src+,r0
dt	&nbytes
mov.b	r0,@&dst
bf.s	53b
add	#1,&dst
52:
.endm


	.global xfer_memcpy
#
# int xfer_memcpy(void *dst, const void *src, size_t len) 
#
#	cpy between buffers for short msg xfer
#
	.align	5
xfer_memcpy:
	mov.l	xfer_memcpy_01,r1
	mov.l	xfer_memcpy_02,r2
	mov.l	r2,@r1
	
	_MEMCPY4	r4, r5, r6, r3
	
	sub		r0,r0
	rts
	mov.l	r0,@r1	

	.align 2
xfer_memcpy_01:
	.long	xfer_handlers 
xfer_memcpy_02:
	.long	xfer_src_handlers
	.type xfer_memcpy,@function
	.size xfer_memcpy,.-xfer_memcpy


	.global xfer_cpy_diov
#
# int xfer_cpy_diov(THREAD* thpd, IOV *dst, uint8_t *saddr, int dparts, unsigned slen) 
#
# cpy from a buffer to an iov (or buffer) for short msg xfer
#
# REG r1:xfer_handlers r4: boundry r5:dst r6:saddr r7:dparts @r15:slen
	.align	5
xfer_cpy_diov:
#	mov.l	xfer_cpy_diov_05,r0
	mov.l	xfer_cpy_diov_01,r1
#	mov.l	@(r0,r4),r4
#	mov.l	xfer_cpy_diov_03,r0

# check dparts
	cmp/pz	r7
# get boundry
#mov.l	@(r0,r4),r4
	bt		4f
	
# negative, direct buffer
# check dst address range
# the opt only works for short msg
#	add		r7,r4
	mov.l	@r15,r3
#	cmp/hi	r4,r5
	neg		r7,r7		
#	bt		102f

# copy msg
mov.l	xfer_cpy_diov_02,r2
	cmp/hs	r7,r3
mov 	#16,r0
	bt		2f
	mov		r3,r7
2:

# check if it is a big msg
cmp/hs	r0,r7
bt.s	200f
# set src handler
	mov.l	r2,@r1

	_MEMCPY4	r5, r6, r7, r2

99:
# return success	
	sub		r0,r0
	rts
	mov.l	r0,@r1	

102:
	bra	100f
	nop

200:
# use memcpy_line for big msg

mov.l	xfer_cpy_diov_06,r0
sts.l	pr,@-r15

mov	r5,r4
mov	r6,r5
jsr	@r0
mov	r7,r6

lds.l	@r15+,pr
mov.l	xfer_cpy_diov_01,r1
sub	r2,r2
rts
mov.l	r2,@r1	

4:
# no dparts, return
mov.l	xfer_cpy_diov_04,r2
	tst		r7,r7
	bt		99b

# set dst handler	
	mov.l	r2,@r1
	
# get dst iov	
mov.l	xfer_cpy_diov_02,r0
	mov.l	@r5,r2
	mov.l	@(4,r5),r3
	
# set src handler	
	mov.l	r0,@r1
	
# check range
 #	mov		r2,r0
 #	add		r3,r0	
 #	add		#-1,r0
 #	cmp/hi	r0,r2
 #	bt		101f
 #	cmp/hs	r4,r0
mov.l	@r15,r0
 #	bt		100f
	
	cmp/hs	r0,r3
	bf		1f
mov 	#16,r3
	mov		r0,r7

# check if it is a big msg
cmp/hs	r3,r0
bt	200f

	_MEMCPY4	r2, r6, r7, r3

	sub		r0,r0
	rts
	mov.l	r0,@r1	

200:
# use memcpy_line for big msg

mov.l	xfer_cpy_diov_06,r0
sts.l	pr,@-r15

mov	r2,r4
mov	r6,r5
jsr	@r0
mov	r7,r6

lds.l	@r15+,pr
mov.l	xfer_cpy_diov_01,r1
sub	r2,r2
rts
mov.l	r2,@r1


1:
	# not within one iov. Rare case.
	sub		r3,r0
mov 	#16,r1
	mov.l	r0,@r15

# check if it is a big msg
cmp/hs	r1,r3
bt	200f

	_MEMCPY4	r2, r6, r3, r1
	mov.l	xfer_cpy_diov_01,r1

98:	
	add		#-1,r7
	bra		4b
	add		#8,r5

101:
	tst		r3,r3
	bt		98b
	
# error
100:
	sub		r0,r0
	mov.l	r0,@r1
	rts
	mov		#XFER_DST_FAULT,r0

200:
# use memcpy_line for big msg

# save tmp registers:r1
sts.l	pr,@-r15
mov	r6,r1
add	#-16,r15
add	r3,r1
mov.l	xfer_cpy_diov_06,r0
mov.l	r4,@(0,r15)
mov.l	r5,@(4,r15)
mov.l	r7,@(8,r15)
mov.l	r1,@(12,r15)


mov	r2,r4
mov	r6,r5
jsr	@r0
mov	r3,r6

mov.l	@(0,r15),r4
mov.l	@(4,r15),r5
mov.l	@(8,r15),r7
mov.l	@(12,r15),r6
add	#16,r15

mov.l	xfer_cpy_diov_01,r1
tst	r0,r0
bt.s	98b
lds.l	@r15+,pr

#error
sub	r2,r2
rts
mov.l	r2,@r1


	
	.align 2
xfer_cpy_diov_05:
	.long	PROCESS	
xfer_cpy_diov_01:
	.long	xfer_handlers 
xfer_cpy_diov_03:
	.long	BOUNDRY_ADDR   
xfer_cpy_diov_02:
	.long	xfer_src_handlers
xfer_cpy_diov_04:
	.long	xfer_dst_handlers
xfer_cpy_diov_06:
	.long	memcpy_line
	.type xfer_cpy_diov,@function
	.size xfer_cpy_diov,.-xfer_cpy_diov
