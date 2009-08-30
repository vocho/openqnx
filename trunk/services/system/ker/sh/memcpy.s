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

.global memcpy_line
.global _memcpy_line
.global _memcpy_line_ret
.global _memcpy_line_noalign
.global _memcpy_line_noalign_ret
# extern void memcpy_line(void*dst,void*src,unsigned len);
# for big block 
# r4 -- dst, r5 -- src, r6 -- len, r0,r1,r2,r3,r7, r8,r9,r10
# r11 -- original dst addr; r4 - r11 # of bytes xfered
memcpy_line:
	#save r8,r9,r10,r11
	mov.l	r8,@-r15
	mov		#3,r0
	mov		#3,r1
	mov.l	r9,@-r15
	and		r4,r0
	mov		#32,r2
	mov.l	r10,@-r15
	and		r5,r1
	mov.l	r11,@-r15
	cmp/eq	r0,r1
	sts.l	pr,@-r15
	
	bf		1f
	cmp/hs	r2,r6
	bf		1f

	bsr		_memcpy_line
	mov		r4,r11

2:
	#restore r8,r9,r10,r11 and return
	lds.l	@r15+,pr
	mov.l	@r15+,r11
	mov.l	@r15+,r10
	mov.l	@r15+,r9
	rts
	mov.l	@r15+,r8

1:
	bsr		_memcpy_line_noalign
	mov		r4,r11
	bra		2b
	nop

# nbytes >= 32	
_memcpy_line:
mov		r5,r0
# make long aligment
tst		#1,r0
bt		1f
# byte
mov.b	@r0+,r1
add		#-1,r6
mov.b	r1,@r4
add		#1,r4

1:
# word
tst		#2,r0
bt		1f
mov.w	@r0+,r1
add		#-2,r6
mov.w	r1,@r4
add		#2,r4

1:
# long copy until src 32 bytes bound is reached
tst		#4,r0
bt		1f
# four bytes
mov.l	@r0+,r1
add		#-4,r6
mov.l	r1,@r4
add		#4,r4

1:
tst		#8,r0
bt		1f
# 8 bytes
mov.l	@(0,r0),r1
mov.l	@(4,r0),r2
add		#8,r0
add		#-8,r6
mov.l	r1,@(0,r4)
mov.l	r2,@(4,r4)
add		#8,r4

1:
tst		#16,r0
bt		1f
# 16 bytes
mov.l	@(0,r0),r1
mov.l	@(4,r0),r2
mov.l	@(8,r0),r3
mov.l	@(12,r0),r7
add		#16,r0
add		#-16,r6
mov.l	r1,@(0,r4)
mov.l	r2,@(4,r4)
mov.l	r3,@(8,r4)
mov.l	r7,@(12,r4)
add		#16,r4

1:
mov		#32,r1
mov		r0,r5
cmp/hs	r1,r6
bf.s	3f
mov		r6,r0
2:
# cache line xfer (32 bytes)
mov.l	@(0,r5),r1
add		#-32,r0
mov.l	@(4,r5),r2
mov.l	@(8,r5),r3
mov.l	@(12,r5),r6
mov.l	@(16,r5),r7
mov.l	@(20,r5),r8
mov.l	@(24,r5),r9
mov.l	@(28,r5),r10
mov.l	r1,@(0,r4)
add		#32,r5
mov		#32,r1
mov.l	r2,@(4,r4)
cmp/hs	r1,r0
mov.l	r3,@(8,r4)
mov.l	r6,@(12,r4)
mov.l	r7,@(16,r4)
mov.l	r8,@(20,r4)
mov.l	r9,@(24,r4)
mov.l	r10,@(28,r4)
bt.s	2b	
add		#32,r4

3:
tst		#16,r0
bt		1f
# 16 bytes
mov.l	@(0,r5),r1
mov.l	@(4,r5),r2
mov.l	@(8,r5),r3
mov.l	@(12,r5),r7
add		#16,r5
mov.l	r1,@(0,r4)
mov.l	r2,@(4,r4)
mov.l	r3,@(8,r4)
mov.l	r7,@(12,r4)
add		#16,r4
1:

tst		#8,r0
bt		1f
# 8 bytes
mov.l	@(0,r5),r1
mov.l	@(4,r5),r2
add		#8,r5
mov.l	r1,@(0,r4)
mov.l	r2,@(4,r4)
add		#8,r4

1:
tst		#4,r0
bt		1f
# four bytes
mov.l	@r5+,r1
mov.l	r1,@r4
add		#4,r4
1:

tst		#2,r0
bt		1f
# word
mov.w	@r5+,r1
mov.w	r1,@r4
add		#2,r4
1:

tst		#1,r0
bt		1f
# byte
mov.b	@r5,r1
mov.b	r1,@r4
add		#1,r4
1:

_memcpy_line_ret:
rts
sub		r0,r0


_memcpy_line_noalign:
	mov		#8,r1
	mov		r6,r0
	cmp/hs	r1,r6
	bf		2f
	
1:
	mov.b	@r5+,r1	
	add		#-8,r0
	mov.b	@r5+,r2	
	mov.b	@r5+,r3	
	mov.b	@r5+,r6
	mov.b	@r5+,r7	
	mov.b	@r5+,r8	
	mov.b	@r5+,r9	
	mov.b	@r5+,r10
	mov.b	r1,@r4
	add		#1,r4
	mov		#8,r1
	mov.b	r2,@r4
	add		#1,r4
	cmp/hs	r1,r0
	mov.b	r3,@r4
	add		#1,r4
	mov.b	r6,@r4
	add		#1,r4
	mov.b	r7,@r4
	add		#1,r4
	mov.b	r8,@r4
	add		#1,r4
	mov.b	r9,@r4
	add		#1,r4
	mov.b	r10,@r4
	bt.s	1b
	add		#1,r4

2:
	tst		#4,r0
	bt		2f
	mov.b	@r5+,r1	
	mov.b	@r5+,r2	
	mov.b	@r5+,r3	
	mov.b	@r5+,r6
	mov.b	r1,@r4
	add		#1,r4
	mov.b	r2,@r4
	add		#1,r4
	mov.b	r3,@r4
	add		#1,r4
	mov.b	r6,@r4
	add		#1,r4

2:
	tst		#2,r0
	bt		2f
	mov.b	@r5+,r1	
	mov.b	@r5+,r2	
	mov.b	r1,@r4
	add		#1,r4
	mov.b	r2,@r4
	add		#1,r4

2:
	tst		#1,r0
	bt		2f
	mov.b	@r5+,r1	
	mov.b	r1,@r4
	add		#1,r4

2:
_memcpy_line_noalign_ret:
	rts
	sub		r0,r0
