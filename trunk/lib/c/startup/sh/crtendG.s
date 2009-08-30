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
# crtend

	.data
	.align 2
	.type	 p.3,@object
	.size	 p.3,4
p.3:
	.long	-4+__CTOR_END__

	.section .init
	.type	 __do_global_ctors_aux,@function
__do_global_ctors_aux:
	mov.l	r8,@-r15
	mov.l	r12,@-r15
	mova	.L7,r0
	mov.l	.L7,r12
	mov.l	.L8,r2
	sts.l	pr,@-r15
	add	r0,r12
	mov	r2,r0
	mov.l	@(r0,r12),r1
	mov.l	@r1,r0
	cmp/eq	#-1,r0
	bt/s	.L4
	mov	r2,r8
	mov	r8,r0
.L10:
	mov.l	@(r0,r12),r1
	mov.l	@r1,r1
	jsr	@r1
	nop
	mov	r8,r0
	mov.l	@(r0,r12),r1
	add	#-4,r1
	mov.l	r1,@(r0,r12)
	mov.l	@r1,r0
	cmp/eq	#-1,r0
	bf/s	.L10
	mov	r8,r0
.L4:
	lds.l	@r15+,pr
	mov.l	@r15+,r12
#	rts	
	mov.l	@r15+,r8
	bra		.L11
.L9:
	.align 2
.L7:
	.long	_GLOBAL_OFFSET_TABLE_
.L8:
	.long	p.3@GOTOFF
.L11:

	.section	.ctors,"aw"
	.align 2
	.type	 __CTOR_END__,@object
	.size	 __CTOR_END__,4
__CTOR_END__:
	.long	0
	.section	.dtors,"aw"
	.align 2
	.type	 __DTOR_END__,@object
	.size	 __DTOR_END__,4
__DTOR_END__:
	.long	0
