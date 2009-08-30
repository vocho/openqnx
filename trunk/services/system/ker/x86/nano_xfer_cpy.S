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

	.globl	xfer_memcpy
	.globl	xfer_cpy_diov

	.data

	_handler_xfer_memcpy:
	.long	_fault_xfer_memcpy
	.long	0

	_handler_xfer_cpy_diov_src:
	.long	_fault_xfer_cpy_diov_src
	.long	0

	_handler_xfer_cpy_diov_dst:
	.long	_fault_xfer_cpy_diov_dst
	.long	0
	
	.text

/
/	int xfer_memcpy(void *dst, const void *src, size_t len)
/	Copy of a short msg (buffer to buffer)
/	
/
/	parameter list: 
#define XFER_MEMCPY_SAVE_NARGS	2		// ebx,ret
#define	XFER_MEMCPY_ARGS		(4+XFER_MEMCPY_SAVE_NARGS*4)
#define	XFER_MEMCPY_DST			(XFER_MEMCPY_ARGS+0x0)
#define XFER_MEMCPY_SRC			(XFER_MEMCPY_ARGS+0x4)
#define	XFER_MEMCPY_LEN			(XFER_MEMCPY_ARGS+0x8)

xfer_memcpy:
	push	%esi
	push	%edi
	movl	XFER_MEMCPY_DST(%esp),%edi 
	movl	XFER_MEMCPY_SRC(%esp),%esi
	movl	XFER_MEMCPY_LEN(%esp),%ecx
	
	SETXFERHANDLER	_handler_xfer_memcpy,%eax,0
	mov	%ecx,%eax
	shr	$2,%ecx
	repne;	movsl
	movb	%al,%cl
	andb	$3,%cl
	repne;	movsb
	
	SETXFERHANDLER	0,%eax,0
	
	pop		%edi
	pop		%esi
	
	xor		%eax,%eax
	ret

/
/	Fault handler for xfer_memcpy	
/	
/	parameter list:
/	(from sp + 4 + saved args * 4)
/ 	0 thp
/ 	4 *regs
/ 	8 fault
_fault_xfer_memcpy:
	/ see what fault it is
	movl	$XFER_DST_FAULT,%eax
	testl	$X86_FAULT_WRITE,12(%esp)	/ 4+8
	jne		1f
	movl	$XFER_SRC_FAULT,%eax
1:	
	/ restore regs
	movl	8(%esp),%esi	/ *regs
	movl	REG_EBP(%esi),%ebp
	movl	REG_EBX(%esi),%ebx
	movl	REG_EBP+4(%esi),%esp
	addl	$12,%esp
	
	pop		%edi
	pop		%esi
	ret

/
/	Fault handler for xfer_cpy_diov	
/
/	parameter list:
/	(from sp + 4 + saved args * 4)
/ 	0 thp
/ 	4 *regs
/ 	8 fault
_fault_xfer_cpy_diov_dst:
	/ see what fault it is
	movl	$XFER_DST_FAULT,%eax
	testl	$X86_FAULT_WRITE,12(%esp)	/ 4+8
	je		1f
	movl	$XFER_SRC_FAULT,%eax
	jmp		1f
		
_fault_xfer_cpy_diov_src:
	/ see what fault it is
	movl	$XFER_DST_FAULT,%eax
	testl	$X86_FAULT_WRITE,12(%esp)	/ 4+8
	jne		1f
	movl	$XFER_SRC_FAULT,%eax
1:	
	/ restore regs
	movl	8(%esp),%esi	/ *regs
	movl	REG_EBP(%esi),%ebp
	movl	REG_EBP+4(%esi),%esp
	addl	$12,%esp
	
	pop		%edi
	pop		%esi
	pop		%ebx
	ret

/
/	int xfer_cpy_diov(THREAD* thpd, IOV *dst, uint8_t *saddr, int dparts, unsigned slen)
/	Copy short msg (bufer to iov)
/
/	parameter list:
#define XFER_CPY_DIOV_SAVE_NARGS	3		// ebx,ret
#define	XFER_CPY_DIOV_ARGS			(4+XFER_CPY_DIOV_SAVE_NARGS*4)
#define	XFER_CPY_DIOV_THPD			(XFER_CPY_DIOV_ARGS+0x0)
#define	XFER_CPY_DIOV_DST			(XFER_CPY_DIOV_ARGS+0x4)
#define XFER_CPY_DIOV_SADDR			(XFER_CPY_DIOV_ARGS+0x8)
#define	XFER_CPY_DIOV_DPARTS		(XFER_CPY_DIOV_ARGS+0xc)
#define	XFER_CPY_DIOV_SLEN			(XFER_CPY_DIOV_ARGS+0x10)

xfer_cpy_diov:
   	pushl  %ebx
	pushl  %esi
	pushl  %edi

	movl	XFER_CPY_DIOV_THPD(%esp),%ebx	/ thpd
 	movl	XFER_CPY_DIOV_DST(%esp),%eax	/ dst
 	movl	XFER_CPY_DIOV_SADDR(%esp),%esi 	/ saddr
	movl	XFER_CPY_DIOV_DPARTS(%esp),%ecx	/ dparts
	movl	PROCESS(%ebx),%ebx	/ thpd->process
 
/ dparts < 0?
	cmpl	$0x0,%ecx
	jnl		1f
	movl	%eax,%edi
	
/ Invert dparts	
	negl	%ecx

 	movl	XFER_CPY_DIOV_SLEN(%esp),%edx 	/ slen
	cmpl	%edx,%ecx
	jbe		2f
	movl	%edx,%ecx
2:	
	SETXFERHANDLER	_handler_xfer_cpy_diov_src,%eax,0
	mov		%ecx,%eax
	shr		$2,%ecx
	repne;	movsl
	movb	%al,%cl
	andb	$3,%cl
	repne;	movsb

	jmp		100f	/ OK	

1:	
	je		100f	/ zero dparts
	 
/ multi parts IOV 
/ ebx porcess; esi saddr; edi daddr; eax dst; ecx dlen/t; edx temp;
10:
	SETXFERHANDLER	_handler_xfer_cpy_diov_dst,%ecx,0
	movl	4(%eax),%ecx
	movl	(%eax),%edi

	cmpl	XFER_CPY_DIOV_SLEN(%esp),%ecx	/ slen
	jb		2f
	movl	XFER_CPY_DIOV_SLEN(%esp),%edx	/ slen
	SETXFERHANDLER	_handler_xfer_cpy_diov_src,%ecx,0
	movl	%edx,%ecx
	shr		$2,%ecx
	repne;	movsl
	movb	%dl,%cl
	andb	$3,%cl
	repne;	movsb
	jmp		100f	/ OK	

2:
	SETXFERHANDLER	_handler_xfer_cpy_diov_src,%edx,0
	movl	%ecx,%edx
	shr		$2,%ecx
	repne;	movsl
	movb	%dl,%cl
	andb	$3,%cl
	repne;	movsb

	decl	28(%esp)	/ dparts
	je		100f		/ OK	
	
	subl	%edx,XFER_CPY_DIOV_SLEN(%esp)	/ slen
	addl	$8,%eax
	
	jmp		10b

99:
	movl	$XFER_DST_FAULT,%eax
	jmp		101f
100:
	xor		%eax,%eax
101:	
	SETXFERHANDLER	0,%ebx,0
	popl	%edi
	popl	%esi
   	popl	%ebx
	ret	
	
