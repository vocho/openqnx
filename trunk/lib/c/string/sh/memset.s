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
*	memset.s
*
*	r4 - dest
*	r5 - value
*	r6 - len
*
*****************************************************************************/

! Fast memset
! by Toshiyasu Morita (toshiyasu.morita@hsa.hitachi.com)
! 6/20/2002

	.little
	.global	memset
	.type	memset,@function
	.balignw 32,0x0009
memset:
	mov.l	r14,@-r15
	mov		r15,r14
	mov		#12,	r0
	cmp/gt	r6,		r0
	bt.s	.L_byte_loop_init
	mov		r4,		r7

	swap.b	r5,		r1
	or		r1,		r5
	swap.w	r5,		r1
	or		r1,		r5

	mov		r4,		r0
	tst		#1,		r0
	bt		.L_wordalign

	mov.b	r5,		@r4
	add		#-1,	r6	
	add		#1,		r4
	mov	r4,r0

.L_wordalign:
	tst		#2,		r0
	bt		.L_word_loop_init

	mov.w	r5,		@r4
	add		#-2,	r6
	add		#2,		r4
	mov r4,r0

.L_word_loop_init:
	mov		r6,		r3
	shlr2	r3
	mov		#7,		r0
	shlr	r3
	and		r0,		r6

	.balignw 8,0x0009
.L_2word_loop:
	mov.l	r5,		@r4
	dt		r3
	mov.l	r5,		@(4,r4)
	bf.s	.L_2word_loop
	add		#8,		r4

.L_byte_loop_init:
	tst		r6,		r6
	bt		.L_byte_exit

	.balignw 8,0x0009
.L_byte_loop:
	mov.b	r5,		@r4
	dt		r6
	bf.s	.L_byte_loop
	add		#1,		r4

.L_byte_exit:
	mov.l	@r15+,r14
	rts
	mov		r7,		r0
	


.L_function_end:
	.size	memset,.L_function_end-memset
