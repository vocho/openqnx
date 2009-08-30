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
# Trampoline code--this function is actually 'returned' to when the
# main thread function falls off the end of its code.  We snatch the 
# return value r0 of the main()-ish function into an argument register r4
# before invoking pthread_exit().
# 

	.extern pthread_exit

	.global	__my_thread_exit
	
	.section ".text"
__my_thread_exit:

.ifdef __PIC__
	mov	r0,r4
	mov.l	__got,r1
	mova	__got,r0
	add	r1,r0
	mov.l	__my_thread_exit_1,r1
	mov.l	@(r0,r1),r2
	jmp	@r2
	nop

	.align 2
__my_thread_exit_1:	
	.long	pthread_exit@GOT
__got:
	.long	_GLOBAL_OFFSET_TABLE_

.else
	mov.l	__my_thread_exit_1,r1
	jmp		@r1
	mov		r0,r4

	.align 2
__my_thread_exit_1:	
	.long	pthread_exit

.endif
	.type __my_thread_exit,@function
	.size __my_thread_exit,.-__my_thread_exit
	
