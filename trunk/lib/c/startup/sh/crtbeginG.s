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
# crtbegin

	.data
	.align 2
	.type	 p.3,@object
	.size	 p.3,4
p.3:
	.long	4+__DTOR_LIST__
	.type	 __dso_handle,@object
	.size	 __dso_handle,4
__dso_handle:
	.long	__dso_handle
	.global	__dso_handle
	.hidden	__dso_handle

	.section .fini
	.align 5
	.type	 __do_global_dtors_aux,@function
__do_global_dtors_aux:

	# save registers
	mov.l	r8,@-r15
	mov.l	r12,@-r15
	sts.l	pr,@-r15

	# setup GOT
	mova	.L7,r0
	mov.l	.L7,r12
	add	r0,r12

	# load pointer to functions
	mov.l	.L8,r0
	mov.l	@(r0,r12),r1
	mov.l	@r1,r1

	# test p
	tst	r1,r1
	bt/s	.Lbyebye

	# save away p
	mov	r0,r8
	mov	r8,r0

.Lloop:

	# increment p
	mov.l	@(r0,r12),r1
	add	#4,r1

	# call p-1
	mov.l	r1,@(r0,r12)
	add	#-4,r1
	mov.l	@r1,r1
	jsr	@r1
	nop

	mov	r8,r0
	mov.l	@(r0,r12),r1
	mov.l	@r1,r1
	tst	r1,r1
	bf	.Lloop

	# restore registers
.Lbyebye:
	lds.l	@r15+,pr
	mov.l	@r15+,r12
	mov.l	@r15+,r8

	.section .text
	.align 5
	.type	 __1do_global_dtors_aux,@function
__1do_global_dtors_aux:
	mov.l	r8,@-r15
	mov.l	r12,@-r15
	mova	.L7,r0
	mov.l	.L7,r12
	sts.l	pr,@-r15
	add	r0,r12
	mov.l	.L8,r0
	mov.l	@(r0,r12),r1
	mov.l	@r1,r1
	tst	r1,r1
	bt/s	.L4
	mov	r0,r8
	mov	r8,r0
.L10:
	mov.l	@(r0,r12),r1
	add	#4,r1
	mov.l	r1,@(r0,r12)
	add	#-4,r1
	mov.l	@r1,r1
	jsr	@r1
	nop
	mov	r8,r0
	mov.l	@(r0,r12),r1
	mov.l	@r1,r1
	tst	r1,r1
	bf	.L10
.L4:
	lds.l	@r15+,pr
	mov.l	@r15+,r12
#	rts	
	mov.l	@r15+,r8

	.section .fini
.L9:
	.align 2
	bra .L66
	nop
.L7:
	.long	_GLOBAL_OFFSET_TABLE_
.L8:
	.long	p.3@GOTOFF
.L66:
	nop

	.section	.ctors,"aw"
	.align 2
	.type	 __CTOR_LIST__,@object
	.size	 __CTOR_LIST__,4
__CTOR_LIST__:
	.long	-1
	.section	.dtors,"aw"
	.align 2
	.type	 __DTOR_LIST__,@object
	.size	 __DTOR_LIST__,4
__DTOR_LIST__:
	.long	-1
