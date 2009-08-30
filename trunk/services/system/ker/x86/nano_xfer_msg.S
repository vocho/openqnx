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

	.globl	xferiov_pos
	.globl	xferiov

#include "asmoff.def"
#include "util.ah"

	.text
	
/
/	int xfer_cpy(THREAD* thp, void* src, void* dst, unsigned bytes)
/	entry:
/		edx		Thread pointer
/		ecx		number of bytes
/		esi		source address
/		edi		dest address
/	retrun:
/		eax		0 -- success; others -- error
/	destroy: (SMP_MSGOPT) ebx
/
xfer_cpy:
#ifdef	VARIANT_smp
#ifdef	SMP_MSGOPT
	mov		$0,%ebx

/ big msg?
	cmp		$512,%ecx
	jb		1f

/release inkernel flag
	andb	$~(INKERNEL_NOW>>8),(inkernel+1)	
	mov		$1,%ebx

1:
#endif
#endif
	mov	%ecx,%eax
	shr	$2,%ecx
xmovsd:	
	repne;	movsl
	movb	%al,%cl
	andb	$3,%cl
	repne;	movsb
/ There is a small window from after the "rep movsd" to the "add" that
/ a restart after premption will recopy bytes already copied.
	add	%eax,ARGS_MSGLEN(%edx)
	
#ifdef	VARIANT_smp
#ifdef	SMP_MSGOPT
	and		%ebx,%ebx
	jz		1f

// reacquire inkern.  we also check if need_to_run is set and if
// so we do not spin on inkernel (to reduce contention for it)
4:
	GETCPU	%cx,%ecx
	cmpl   $0,need_to_run
	je     44f
	movl   need_to_run_cpu,%eax
	cmpl   %eax,%ecx
	je		44f
	pause
	jmp    4b
44:
	mov		inkernel,%eax
	test	$INKERNEL_NOW+INKERNEL_LOCK,%eax
	jz		45f
	pause
	jmp		4b
45:	
	GETCPU	%cx,%ecx
	mov		%eax,%edx
	andl	$0x00ffffff,%edx
	shl		$24,%ecx
	orl		%edx,%ecx		/ Set cpunum
	orl		$INKERNEL_NOW,%ecx
	lock; cmpxchg	%ecx,inkernel
	jz		1f
	pause
	jmp		4b
	
1:
#endif	
#endif	

	sub		%eax,%eax
	ret


/
/ called when restarting a message pass
/
xferiov_pos:
	mov	4(%esp),%edx			/ Argument is pointer to registers
	xor	%eax,%eax
	mov	REG_EIP(%edx),%ecx
	cmp	$xmovsd,%ecx
	jne	notmovsd
	mov	REG_EAX(%edx),%eax	/ eax is original BYTE count to xfer
	mov	REG_ECX(%edx),%edx	/ ecx is whats left of dword count on "rep movsd"
	inc	%edx					/ since it has not yet executed the opcode!
	shl	$2,%edx				/ convert to bytes
	sub	%edx,%eax				/ adjust to correct for downcount
	jae	notmovsd
	xor	%eax,%eax
notmovsd:
	and	$~3,%eax				/ Always trucate to dwords
	ret

#undef ARGS

/
/	int xferiov(THREAD *sthp, IOV *dst, IOV *src, int dparts, int sparts, int doff, int soff)
/
#define	DADDR		0x0
#define	SADDR		0x4
#define	DLEN		0x8
#define	FRAME_SIZE	0xc
#define	SAVE_NARGS	5		// ebx,esi,edi,ebp,ret
#define	ARGS		(FRAME_SIZE+SAVE_NARGS*4)
#define STHP		(ARGS+0x0)
#define DST			(ARGS+0x4)
#define SRC			(ARGS+0x8)
#define	DPARTS		(ARGS+0x0c)
#define	SPARTS		(ARGS+0x10)
#define	DOFF		(ARGS+0x14)
#define	SOFF		(ARGS+0x18)

xferiov:
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp				/ used for SLEN
	sub	$FRAME_SIZE,%esp

/ Extract first dst iov
	mov	DST(%esp),%eax
	mov	DOFF(%esp),%edx
	mov	IOV_ADDR(%eax),%eax
	add	%eax,%edx
	mov	DST(%esp),%eax
	mov	IOV_LEN(%eax),%eax
	sub	DOFF(%esp),%eax
	mov	%edx,DADDR(%esp)
	mov	%eax,DLEN(%esp)
/ Extract first src iov
	mov	SRC(%esp),%eax
	mov	SRC(%esp),%ebp
	mov	SOFF(%esp),%edx
	mov	IOV_ADDR(%eax),%eax
	mov	IOV_LEN(%ebp),%ebp
	add	%eax,%edx
	sub	SOFF(%esp),%ebp
	mov	%edx,SADDR(%esp)
/ next through parts
next:
	cmp	DLEN(%esp),%ebp
	jae	dst_smaller
/ src is smaller than dst
	mov	SADDR(%esp),%esi
	mov	DADDR(%esp),%edi
	mov	STHP(%esp),%edx
	mov	%ebp,%ecx
#ifdef	VARIANT_smp
#ifdef	SMP_MSGOPT
	push	%ebx	
#endif
#endif	
	call	xfer_cpy
#ifdef	VARIANT_smp
#ifdef	SMP_MSGOPT
	pop		%ebx	
#endif
#endif	
	cmpl	$0,%eax
	jne		exit
	decl	SPARTS(%esp)
	je	done
	addl	$SIZEOF_IOV,SRC(%esp)
	add	%ebp,DADDR(%esp)
	mov	SRC(%esp),%eax
	sub	%ebp,DLEN(%esp)
	mov	SRC(%esp),%ebp
	mov	IOV_ADDR(%eax),%eax
	mov	IOV_LEN(%ebp),%ebp
	mov	%eax,SADDR(%esp)
	jmp	next

dst_smaller:
	ja	len_equal
/ dst is smaller than src
	mov	SADDR(%esp),%esi
	mov	DADDR(%esp),%edi
	mov	STHP(%esp),%edx
	mov	%ebp,%ecx
#ifdef	VARIANT_smp
#ifdef	SMP_MSGOPT
	push	%ebx	
#endif
#endif	
	call	xfer_cpy
#ifdef	VARIANT_smp
#ifdef	SMP_MSGOPT
	pop		%ebx	
#endif
#endif	
	cmpl	$0,%eax
	jne		exit
	decl	DPARTS(%esp)
	je	done
	decl	SPARTS(%esp)
	je	done
	addl	$SIZEOF_IOV,SRC(%esp)
	mov	SRC(%esp),%eax
	mov	SRC(%esp),%ebp
	addl	$SIZEOF_IOV,DST(%esp)
	mov	IOV_ADDR(%eax),%eax
	mov	IOV_LEN(%ebp),%ebp
	mov	%eax,SADDR(%esp)
	mov	DST(%esp),%eax
	mov	IOV_ADDR(%eax),%eax
	mov	%eax,DADDR(%esp)
	mov	DST(%esp),%eax
	mov	IOV_LEN(%eax),%eax
	mov	%eax,DLEN(%esp)
	jmp	next

len_equal:
/ src and dst are same size
	mov	DLEN(%esp),%ecx
	mov	SADDR(%esp),%esi
	mov	DADDR(%esp),%edi
	mov	STHP(%esp),%edx
#ifdef	VARIANT_smp
#ifdef	SMP_MSGOPT
	push	%ebx	
#endif
#endif	
	call	xfer_cpy
#ifdef	VARIANT_smp
#ifdef	SMP_MSGOPT
	pop		%ebx	
#endif
#endif	
	cmpl	$0,%eax
	jne		exit
	decl	DPARTS(%esp)
	je	done
	mov	DLEN(%esp),%eax
	addl	$SIZEOF_IOV,DST(%esp)
	add	%eax,SADDR(%esp)
	sub	%eax,%ebp
	mov	DST(%esp),%eax
	mov	IOV_ADDR(%eax),%eax
	mov	%eax,DADDR(%esp)
	mov	DST(%esp),%eax
	mov	IOV_LEN(%eax),%eax
	mov	%eax,DLEN(%esp)
	jmp	next
done:
	xor	%eax,%eax
exit:
	add	$FRAME_SIZE,%esp
	pop	%ebp
	pop	%edi
	pop	%esi
	pop	%ebx
	ret

