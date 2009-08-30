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

#
#
# setjmp.s
#	Routines for saving and restoring a context
#

#
# jmp_buf:
#
#      ----------------
#      |       PR      |    jmp_buf[0]
#      -----------------
#      |       SP(R15) |           [1]
#      -----------------
#      |       R8      |           [2]
#      -----------------
#      |       R9      |           [3]
#      -----------------
#      |       R10     |           [4]
#      -----------------
#      |       ...     |           ...
#      -----------------
#      |       R14     |           
#      -----------------
#      |       MACL    |           
#      -----------------
#      |       MACH    |           [10]
#      -----------------
#      |       FPSCR   |           [11]
#      -----------------
#      |       FR12    |           [12]
#      -----------------
#      |       FR13    |           [13]
#      -----------------
#      |       FR14    |           [14]
#      -----------------
#      |       FR15    |           [15]
#      -----------------
#
 
	.global _setjmp
	.global _longjmp
	.section ".text"
	
 #
 # int _setjmp (jmp_buf *ptr)
 #
_setjmp:
	sts		pr,r0
	mov.l	r0,@(0,r4)
	mov.l	r15,@(4,r4)
	mov.l	r8,@(8,r4)
	mov.l	r9,@(12,r4)
	mov.l	r10,@(16,r4)
	mov.l	r11,@(20,r4)
	mov.l	r12,@(24,r4)
	mov.l	r13,@(28,r4)
	mov.l	r14,@(32,r4)
	sts		macl,r0
	mov.l	r0,@(36,r4)
	sts		mach,r0
	mov.l	r0,@(40,r4)
	add		#64,r4
	fmov	fr15,@-r4
	fmov	fr14,@-r4
	fmov	fr13,@-r4
	fmov	fr12,@-r4
	sts.l	fpscr,@-r4
	add		#-44,r4
	rts
	mov		#0,r0
	
	.type _setjmp,@function
	.size _setjmp,.-_setjmp
 
 #
 # void _longjmp (jmp_buf *ptr, int ret_val)
 #
_longjmp:
	mov.l	@(0,r4),r0
	lds		r0,pr
	mov.l	@(4,r4),r15
	mov.l	@(8,r4),r8
	mov.l	@(12,r4),r9
	mov.l	@(16,r4),r10
	mov.l	@(20,r4),r11
	mov.l	@(24,r4),r12
	mov.l	@(28,r4),r13
	mov.l	@(32,r4),r14
	mov.l	@(36,r4),r0
	lds		r0,macl
	mov.l	@(40,r4),r0
	lds		r0,mach
	add		#44,r4
	lds.l	@r4+,fpscr
	fmov	@r4+,fr12
	fmov	@r4+,fr13
	fmov	@r4+,fr14
	fmov	@r4+,fr15
	add		#-64, r4
	mov		r5,r0
	cmp/eq	#0,r0
	bt		1f
	rts
	nop
1:	
	rts
	mov		#1,r0

	.type _longjmp,@function
	.size _longjmp,.-_longjmp
