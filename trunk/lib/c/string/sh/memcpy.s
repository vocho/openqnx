#
# $QNXtpLicenseA:
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



/*****************************************************************************
* Tabs 5 9
* The carets in the following lines will match up if tabs are set properly
*	^	^	^	^	^	^	<== tabs
*   ^   ^   ^   ^   ^   ^   <== spaces
******************************************************************************
*
*	HSA Software License, Version 1.0
*
*	Copyright (C) 2000,2001,2002 Hitachi Semiconductor (America), Inc.
*	All rights reserved.
*
*	Redistribution and use in source and binary forms, with or without
*	modification, are permitted provided that the following conditions are
*	met:
*
*	1.	Redistributions of source code must retain the above copyright notice,
*		this list of conditions and the following disclaimer.
*
*	2.	Redistributions in binary form must reproduce the above copyright
*		notice, this list of conditions and the following disclaimer in the
*		documentation and/or other materials provided with the distribution.
*
*	3.	The end-user documentation included with the redistribution, if any,
*		must include the following acknowledgment: "This product includes
*		software developed by Hitachi Semiconductor (America), Inc."
*		Alternately, this acknowledgment may appear in the software itself,
*		if and wherever such third-party acknowledgments normally appear.
*
*
*	THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES,
*	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
*	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
*	HITACHI SEMICONDUCTOR (AMERICA), INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
*	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
*	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
*	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
******************************************************************************
*
*	memcpy2.s
*
*	r4 - dest
*	r5 - src
*	r6 - len
*
*****************************************************************************/

! Fast memcpy v2.0
! by Toshiyasu Morita (toshiyasu.morita@hsa.hitachi.com)
! 6/20/2002

	.little
	.global		memcpy
	.type		memcpy,@function
	.balignw	32,0x0009
memcpy:
.ifdef VARIANT_so
	mov.l   r12,@-r15
	mova	.L_GOT,	r0
	mov.l	.L_GOT,	r12
	add		r0,		r12
.endif
	mov.l	r14,@-r15
	mov		r15,r14
	mov		r4,		r3		! Save dest

	mov		r6,		r0		! If less than 11 bytes, just do a byte copy
	mov		#11,	r0
	cmp/gt	r6,		r0
	bt		.L_byteloop_init

	mov		r5,		r0		! Check if we need to word-align source
	tst		#1,		r0
	bt		.L_wordalign

	mov.b	@r0+,	r1		! Copy one byte
	add		#-1,	r6
	mov.b	r1,		@r4
	add		#1,		r4

.L_wordalign:
	tst		#2,		r0		! Check if we need to longword-align source
	bt		.L_copy

	mov.w	@r0+,	r1		! Copy one word
	add		#-2,	r6
	mov.b	r1,		@r4
	shlr8	r1
	add		#1,		r4
	mov.b	r1,		@r4
	add		#1,		r4

.L_copy:
	mov		r0,		r5		! Move r0 back to r5

.ifdef VARIANT_so
	mov.l	.L_GOT_jumptable,r0	! Calculate the correct routine to handle the
	add		r12,	r0
.else
	mova	.L_jumptable,r0	! Calculate the correct routine to handle the
.endif
	mov		r0,		r1		!   destination alignment and simultaneously
	mov		r4,		r0		!   calculate the loop counts for both the
	mov		r6,		r7		!   2 word copy loop and byte copy loop
	and		#3,		r0
	shlr2	r7
	shll2	r0
	shlr	r7
	mov.l	@(r0,r1),r2
	mov		#7,		r0
	jmp		@r2
	and		r0,		r6

.ifdef VARIANT_so
	.data
.endif
	.balign	4
.L_jumptable:
	.long	.L_copydest0
	.long	.L_copydest1_or_3
	.long	.L_copydest2
	.long	.L_copydest1_or_3

	.text
	.balign	4
.L_copydest1_or_3:			! Copy routine for (dest mod 4) == 1 or == 3
	add		#-3,	r4

	.balignw	8,0x0009
.L_copydest1_or_3_loop:
	mov.l	@r5+,	r0		! Read first longword
	add		#3,		r4
	mov.l	@r5+,	r1		! Read second longword
	dt		r7
	mov.b	r0,		@r4		! Write first longword as byte, word, byte
	add		#1,		r4
	shlr8	r0
	mov.w	r0,		@r4
	shlr16	r0
	mov.b	r0,		@(2,r4)
	mov		r1,		r0
	mov.b	r0,		@(3,r4)	! Write second longword as byte, word, byte
	add		#4,		r4
	shlr8	r0
	mov.w	r0,		@r4
	shlr16	r0
	mov.b	r0,		@(2,r4)
	bf		.L_copydest1_or_3_loop

	bra		.L_byteloop_init
	add		#3,		r4

.L_copydest2:			! Copy routine for (dest mod 4) == 2
	add		#-8,	r4

	.balignw	8,0x0009
.L_copydest2_loop:
	mov.l	@r5+,	r0		! Read first longword
	add		#8,		r4
	mov.l	@r5+,	r1		! Read second longword
	dt		r7
	mov.w	r0,		@r4		! Write first longword as two words
	shlr16	r0
	mov.w	r0,		@(2,r4)
	mov		r1,		r0
	mov.w	r0,		@(4,r4)	! Write second longword as two words
	shlr16	r0
	mov.w	r0,		@(6,r4)
	bf		.L_copydest2_loop

	bra		.L_byteloop_init
	add		#8,		r4

.L_copydest0:			! Copy routine for (dest mod 4) == 0
	add		#-8,	r4
	.balignw	8,0x0009
.L_copydest0_loop:
	mov.l	@r5+,	r0		! Read first longword
	add		#8,		r4
	mov.l	@r5+,	r1		! Read second longword
	dt		r7
	mov.l	r0,		@r4		! Write first longword
	bf/s	.L_copydest0_loop
	mov.l	r1,		@(4,r4)	! Write second longword

	add		#8,		r4		! Fall through


.L_byteloop_init:
	tst		r6, 	r6
	bt		.L_exit

	.balignw	8,0x0009
.L_byteloop:			! Copy remaining bytes
	mov.b	@r5+,	r0
	dt		r6
	mov.b	r0,		@r4
	bf/s	.L_byteloop
	add		#1,		r4

.L_exit:
	mov.l   @r15+,r14
.ifdef VARIANT_so
	mov.l   @r15+,r12
.endif
	rts
	mov		r3,		r0		! Return dest

.ifdef VARIANT_so
	.align	2
.L_GOT:
	.long	_GLOBAL_OFFSET_TABLE_
.L_GOT_jumptable:
	.long	.L_jumptable@GOTOFF
.endif
.L_function_end:
	.size	memcpy,.L_function_end-memcpy
