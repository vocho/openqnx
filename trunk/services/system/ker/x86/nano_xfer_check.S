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

/
/ nano_xfer_check.s
/	Routines for check iov boundry and check read
/

	.include "asmoff.def"
	
	.global xfer_memchk
	.global xfer_memprobe
/
/ int xfer_memchk(uintptr_t bound, const IOV *iov, size_t parts) 
/
/	Check the boundry of an iov array
/		edx	iov
/		ecx len
/		eax	ptr/return
/
/	parameter list: 
#define	XFER_MEMCHK_ARGS		(4)
#define	XFER_MEMCHK_BOUND		(XFER_MEMCHK_ARGS+0x0)
#define XFER_MEMCHK_IOV			(XFER_MEMCHK_ARGS+0x4)
#define	XFER_MEMCHK_PARTS		(XFER_MEMCHK_ARGS+0x8)

xfer_memchk:
	movl	XFER_MEMCHK_IOV(%esp),%edx
1:	
	movl	0(%edx),%eax
	movl	4(%edx),%ecx
	addl	%eax,%ecx
	decl	%ecx
	cmpl	%eax,%ecx
	jb		11f
	cmpl	XFER_MEMCHK_BOUND(%esp),%ecx	
	ja		11f
9:	
	add		$8,%edx
	decl	XFER_MEMCHK_PARTS(%esp)
	jne		1b
	
	# succeed
	sub		%eax,%eax
	ret

11: / is len zero?
	cmpl	$0,4(%edx)	
	je		9b

	/ boundry error
	movl	$-1,%eax
	ret
	
	.type xfer_memchk,@function
	.size xfer_memchk,.-xfer_memchk

/
/ int xfer_memprobe(uintptr_t addr) 
/
/	Check the boundry of an iov array
/		edx	addr/tmp
/		eax	return
/
/	parameter list: 
#define	XFER_MEMPROBE_ARGS		(4)
#define	XFER_MEMPROBE_ADDR		(XFER_MEMPROBE_ARGS+0x0)
xfer_memprobe:
	movl	XFER_MEMPROBE_ADDR(%esp),%edx
	sub		%eax,%eax
	movb	0(%edx),%dl 	
	ret	

