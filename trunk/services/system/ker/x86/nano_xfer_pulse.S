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

#include "asmoff.def"
#include "util.ah"

	.globl	xferpulse

	.data
	_handler_xferpulse:
	.long	_fault_xferpulse
	.long	0
	
/
/	int xferpulse(THREAD *dthp, IOV *dst, int parts, uint32_t code, uint32_t value, int32_t scoid) 
/	Xfer a pulse
/
	
#undef ARGS // defined in asmoff.def
#define SAVE_NARGS	2		// ebx,ret
#define	ARGS		(+SAVE_NARGS*4)
#define	DTHP		(ARGS+0x0)
#define DST			(ARGS+0x4)
#define	PARTS		(ARGS+0x8)
#define	CODE		(ARGS+0xc)
#define	VALUE		(ARGS+0x10)
#define	SCOID		(ARGS+0x14)

	.text
xferpulse:
	push	%ebx
	mov	DST(%esp),%ebx
	SETXFERHANDLER	_handler_xferpulse,%eax,0
	cmp	$0,PARTS(%esp)
	jl	X$4
	mov	IOV_ADDR(%ebx),%eax
	mov	IOV_LEN(%ebx),%edx
X$1:
	cmp	$PULSE_SIZE,%edx
	jb	dst_flt
	add	%eax,%edx
	dec	%edx
	cmp	%edx,%eax
	ja	dst_flt
	mov	DTHP(%esp),%ecx
	mov	PROCESS(%ecx),%ecx
	cmp	BOUNDRY_ADDR(%ecx),%edx
	jbe	X$5
dst_flt:
	SETXFERHANDLER	0,%eax,0
	mov	$XFER_DST_FAULT,%eax
	jmp	exit
X$4:
	mov	PARTS(%esp),%edx
	mov	%ebx,%eax
	neg	%edx
	jmp	X$1
X$5:
	movw	$0,0x0(%eax)
	movb	CODE(%esp),%dl
	movw	$0,0x2(%eax)
	movb	%dl,0x4(%eax)
	mov	VALUE(%esp),%edx
	mov	%edx,0x8(%eax)
	mov	SCOID(%esp),%edx
	mov	%edx,0xc(%eax)
	SETXFERHANDLER	0,%eax,0
	xor	%eax,%eax
exit:
	pop	%ebx
	ret
	
/
/	Fault handler for xferpulse
/
/	parameter list:
/	(from sp + 4 + saved args * 4)
/ 	0 thp
/ 	4 *regs
/ 	8 fault
_fault_xferpulse:
	movl	$XFER_SRC_FAULT,%eax
	/ restore regs
	movl	8(%esp),%ebx	/ *regs
	movl	REG_EBP(%ebx),%ebp
	movl	REG_ESI(%ebx),%esi
	movl	REG_EDI(%ebx),%edi
	movl	REG_EBP+4(%ebx),%esp
	addl	$12,%esp
	pop	%ebx
	ret
	
