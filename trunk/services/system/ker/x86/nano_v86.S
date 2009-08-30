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

	
	.text

	.globl	__v86

/
/ V86 code always runs on processor zero in an SMP system (see v86.c
/ in proc)
/

#include "asmoff.def"

/
/ Offsets of esp after we have pushed some registers.
/
#define	OPBYTES		0
#define	SEGOVER		4
#define SIZOVER		8
#define TEMP_SIZE	12
#define	VEBP		(TEMP_SIZE+0)
#define	VEDI		(TEMP_SIZE+4)
#define	VESI		(TEMP_SIZE+8)
#define	VEBX		(TEMP_SIZE+12)
#define	VEIP		(TEMP_SIZE+16)
#define	VCS			(TEMP_SIZE+20)
#define	VEFL		(TEMP_SIZE+24)
#define	VESP		(TEMP_SIZE+28)
#define	VSS			(TEMP_SIZE+32)
#define	VES			(TEMP_SIZE+36)
#define	VDS			(TEMP_SIZE+40)


#define ADSIZ_OVER	0x01
#define OPSIZ_OVER	0x02


/
/ v86 fault handler is here to catch "int nn", in/out opcodes which will cause
/ a GP fault in the V86 machine. In the case of an "int nn" we have to fake up
/ what it should have done (vector through the real mode interrupt vectors).
/ We use an iret at a specific stubcode offset to terminate v86 mode.
/
__v86:
	mov		%ss:usr_ds,%ds
	mov		%ss:usr_ds,%es
	push	%ebx
	push	%esi
	push	%edi
	push	%ebp
	push	$0						/ No size overrides
	push	$0						/ Default no segment override
	push	$1						/ Default to 1 byte opbyte
	mov		%esp,%ebp				/ Switch to kernel stack
	mov		ker_stack,%esp
	mov		$V86_EXECCODE,%edi		/ point to execute buffer

/
/ Calculate linear address of v86  cs:ip and get opcode which faulted
/
	movzwl	VCS(%ebp),%ebx
	shl		$4,%ebx
	add		VEIP(%ebp),%ebx			/ ebx is linear address of v86 cs:ip
	mov		0(%ebx),%ebx			/ Fetch opcode + 3 bytes following
	jmp		prefix

/
/ Process all prefix bytes
/
keep_prefix_op:
	xorb	$OPSIZ_OVER,SIZOVER(%ebp)
	jmp		keep_prefix
keep_prefix_ad:
	xorb	$ADSIZ_OVER,SIZOVER(%ebp)
	/ Fall through
keep_prefix:
	movb	%bl,(%edi)
	inc		%edi
	jmp		nextprefix
segds:
	inc		%esi
seges:
	inc		%esi
segss:
	add		$3,%esi
segcs:
	lea		VCS(%ebp,%esi,4),%esi
	mov		%esi,SEGOVER(%ebp)		/ Save address of segment to override
nextprefix:
	shr		$8,%ebx
prefix:
	xor		%esi,%esi
	cmpb	$0x066,%bl					/ Check for a 32 bit opsize
	je		keep_prefix_op
	cmpb	$0x067,%bl					/ Check for a 32 bit addrsize
	je		keep_prefix_ad
	cmpb	$0x0f3,%bl					/ Check for "rep"
	je		keep_prefix
	cmpb	$0x026,%bl					/ Check for "es:"
	je		seges
	cmpb	$0x036,%bl					/ Check for "ss:"
	je		segss
	cmpb	$0x02e,%bl					/ Check for "cs:"
	je		segcs
	cmpb	$0x03e,%bl					/ Check for "ds:"
	je		segds
	
	mov		%ebx,(%edi)				/ Stash instr for possible emulation
	
   	sub		$V86_EXECCODE-1,%edi	/ remember prefix size and guess 
	mov		%edi,OPBYTES(%ebp)		/ one byte opcode

/
/ Calculate linear address of v86  ss:sp
/
	movzwl	VSS(%ebp),%esi
	movzwl	VESP(%ebp),%edi
	shl		$4,%esi
	add		%edi,%esi				/ Point to v86 stack

/
/ Process all opcodes
/
	cmpb	$0x0fb,%bl					/ Check for "sti"
	je		_sti
	cmpb	$0x0fa,%bl					/ Check for "cli"
	je		_cli

	cmpb	$0x0ef,%bl					/ Check for "in/out dx"
	ja		l1
	cmpb	$0x0ec,%bl
	jae		_inout1

	cmpb	$0x0e7,%bl					/ Check for "in/out port"
	ja		l1
	cmpb	$0x0e4,%bl
	jae		_inout2

l1:
	cmpb	$0x06c,%bl					/ Check for "insb"
	je		_ins
	cmpb	$0x06d,%bl					/ Check for "insw"
	je		_ins

	cmpb	$0x06e,%bl					/ Check for "outsb"
	je		_outs
	cmpb	$0x06f,%bl					/ Check for "outsw"
	je		_outs

	cmpb	$0x09c,%bl					/ Check for "pushf"
	je		_pushf
	cmpb	$0x09d,%bl					/ Check for "popf"
	je		_popf

	cmpb	$0x0cd,%bl					/ Check for "int nn"
	je		_int

	cmpb	$0x0cf,%bl					/ Check for "iret"
	je		_iret

/
/ If we end up here we hit an unsupported opcode. Terminate the kercall
/ with a fault indication
/
	mov		$ERRNO_EFAULT,%edx
	mov		actives,%eax
	push	$v86leave
	jmp		kererr

/
/ Opcode emulations
/

_sti:
#ifdef TRUST_BIOS
	orl		$0x0200,VEFL(%ebp)		/ Set interrupt enable (IF bit)
#endif
	jmp		v86ret

_cli:
#ifdef TRUST_BIOS
	andl	$~0x200,VEFL(%ebp)	/ Clear interrupt enable (IF bit)
#endif
	jmp		v86ret

_pushf:
	testb	$OPSIZ_OVER,SIZOVER(%ebp)
	jne		_pushfd
	sub		$2,VESP(%ebp)		/ Adjust v86 stack
	mov		VEFL(%ebp),%ebx		/ Get flags
	mov		%bx,-2(%esi)		/ Push them on v86 stack
	jmp		v86ret
	
_pushfd:
	sub		$4,VESP(%ebp)		/ Adjust v86 stack
	mov		VEFL(%ebp),%ebx		/ Get flags
	mov		%ebx,-4(%esi)		/ Push them on v86 stack
	jmp		v86ret

_popf:
	testb	$OPSIZ_OVER,SIZOVER(%ebp)
	jne		_popfd
	add		$2,VESP(%ebp)			/ Adjust v86 stack
	mov 	(%esi),%bx				/ Get popf flags 
#ifndef TRUST_BIOS
	or		$0x200,%bx			/ Dont let v86 code disable interrupts
#endif
	mov 	%bx,VEFL(%ebp)			/ Load flags with them
	jmp		v86ret

_popfd:
	add		$4,VESP(%ebp)			/ Adjust v86 stack
	mov		(%esi),%ebx				/ Get popf flags 
#ifndef TRUST_BIOS
	or		$0x200,%bx			/ Dont let v86 code disable interrupts
#endif
	mov		%ebx,VEFL(%ebp)			/ Load flags with them
	jmp		v86ret

_inout2:
	incl	OPBYTES(%ebp)	/ A 2 byte opcode
_inout1:
	call	exec
	jmp		v86ret

_ins:
	movzwl	VES(%ebp),%esi	/ Convert es:di into 32 bit 0:edi
	cmpl	$0,SEGOVER(%ebp)
	jz		ins1
	mov		SEGOVER(%ebp),%esi
	movzwl	(%esi),%esi				/ Convert seg:di into 32 bit 0:edi
ins1:	
	shl		$4,%esi
//NYI: doesn't deal with a piece of 16-bit code wanting to do an ADDRSIZ
//override
	movzwl	VEDI(%ebp),%edi
	add		%esi,%edi					/ edi now 32 bit
	call	exec_addr
	sub		%esi,%edi					/ edi back to 16 bit
	mov		%edi,VEDI(%ebp)
	jmp		v86ret

_outs:
	movzwl	VDS(%ebp),%edi	/ Convert ds:si into 32 bit 0:esi
	cmpl	$0,SEGOVER(%ebp)
	jz		outs1
	mov		SEGOVER(%ebp),%edi
	movzwl	(%edi),%edi				/ Convert seg:di into 32 bit 0:edi
outs1:	
	shl		$4,%edi
//NYI: doesn't deal with a piece of 16-bit code wanting to do an ADDRSIZ
//override
	movzwl	VESI(%ebp),%esi
	add		%edi,%esi					/ esi now 32 bit
	call	exec_addr
	sub		%edi,%esi					/ esi back to 16 bit
	mov		%esi,VESI(%ebp)
	jmp		v86ret

v86ret:
	mov		OPBYTES(%ebp),%ebx
	cmpl	$0,SEGOVER(%ebp)
	jz		1f
	inc		%ebx			/ If override, real instr is 1 byte longer
1:
	add		%ebx,VEIP(%ebp)			/ Bump instruction over emulated opcode
v86ret2:	
	lea		TEMP_SIZE(%ebp),%esp	/ restore old stack pointer & toss temps
	pop		%ebp
	pop		%edi
	pop		%esi
	pop		%ebx
	iret

_iret:
	testb	$OPSIZ_OVER,SIZOVER(%ebp)
	jne		_iretd
	mov		VEIP(%ebp),%edi			/ Get addr of opcode which faulted
	cmp		$V86_STUBCODE+2,%edi	/ We plant an iret here to regain control
	je		v86leave				/ Yes, we are done.
	add		$6,VESP(%ebp)			/ Remove ip,cs,fl for iret
	movw	4(%esi),%di				/ Load new flags
#ifndef TRUST_BIOS
	or		$0x200,%di				/ Dont let v86 code disable interrupts
#endif
	movw	%di,VEFL(%ebp)
	mov		(%esi),%edi				/ Load ip,cs
	cmp		$V86_STUBCODE+2,%edi	/ Are we returning to stubcode+2
	je		v86leave				/ Yes, we are done.
	and		$0x0ffff,%edi			/ Load new ip
	mov		%edi,VEIP(%ebp)
	movzwl	2(%esi),%edi			/ Load new cs
	mov		%edi,VCS(%ebp)
	jmp		v86ret2
	
_iretd:
	mov		(%esi),%edi				/ Load eip
	mov		%edi,VEIP(%ebp)
	movzwl	4(%esi),%edi			/ Load new cs
	mov		%edi,VCS(%ebp)
	mov		8(%esi),%edi			/ Load new flags
#ifndef TRUST_BIOS
	or		$0x200,%di			/ Dont let v86 code disable interrupts
#endif
	mov		%edi,VEFL(%ebp)
	add		$12,VESP(%ebp)			/ Remove eip,cs,efl for iretd
	jmp		v86ret2

_int:
	mov		VEIP(%ebp),%edi
	add		$2,%edi					/ Step over int nn
	movw	%di,-6(%esi)
	mov		VCS(%ebp),%edi
	movw	%di,-4(%esi)
	mov		VEFL(%ebp),%edi
	movw	%di,-2(%esi)
	sub		$6,VESP(%ebp)	/ Adust stack down for iret addr
/
/ Patch priv 0 stack to return to where int nn vector points in v86 space
	movzb	%bh,%ebx
	shl		$2,%ebx			/ ebx is linear address of int nn vec
	movzwl	0(%ebx),%esi	/ Get ip from real mode idt
	mov		%esi,VEIP(%ebp)
	movzwl	2(%ebx),%esi	/ Get cs from real mode idt
	mov		%esi,VCS(%ebp)
	andl	$~0x200,VEFL(%ebp)	/ Mask off IF flag
	jmp		v86ret2

v86leave:
	lea		TEMP_SIZE(%ebp),%esp	/ restore old stack pointer & toss temps
	pop		%ebp
	pop		%edi
	pop		%esi
	pop		%ebx
	pusha						/ Save register set for caller
	mov		ker_stack,%esp
	mov		actives,%eax
	andl	$~_NTO_TF_V86,TFLAGS(%eax)
	jmp		__ker_exit_v86


/
/ We move the following opcodes to a 16 bit code seg and execute directly.
/
/ ec:in al,dx    ed:in ax,dx    ee:out dx,al    ef:out dx,ax    (1 byte)
/ e4:in al,port  e5:in ax,port  e6:out port,al  e7:out port,ax  (2 byte)
/ 6c:insb        6d:insw
/
/ V86_EXECCODE buffer is of form:
/     
/     [prefixes] opcode  066 retf
/     [prefixes] opcode  port 066 retf

exec_addr: // add an addrsize override to the mix
	push	%eax
	mov		OPBYTES(%ebp),%ebx			
	mov		$0x67,%al
	xchg	%al,V86_EXECCODE-1(%ebx)	// Assume 1 byte opcode
	movb	%al,V86_EXECCODE(%ebx)	
	inc		%ebx
	
	movw	$0xcb66,V86_EXECCODE(%ebx)	/ End opcode with a retf 32bit.
	
	movl	%cr0,%eax	// make sure the prefetcher gets newly written
	movl	%eax,%cr0	// instructions by using serializing instruction 
	
	pop		%eax
	
   	// "call far X86_V86_GPF_CS:V86_EXECCODE" sequence 
	// (X86_V86_GPF_CS is 16-bit segment)
	.byte	0x9a
	.long	V86_EXECCODE
	.short	X86_V86_GPF_CS
	
	ret
	
exec:
	mov		OPBYTES(%ebp),%ebx			/ Get size of opcode
	movw	$0xcb66,V86_EXECCODE(%ebx)	/ End opcode with a retf 32bit.
	
   	push	%ebx
	push	%ebp
	
	movl	%cr0,%ebx	// make sure the prefetcher gets newly written
	movl	%ebx,%cr0	// instructions by using serializing instruction 

//                            CF     PF     AF     ZF     SF     DF      OF			
#define FLAG_RESTORE_BITS	((1<<0)+(1<<2)+(1<<4)+(1<<6)+(1<<7)+(1<<10)+(1<<11))
	
	// Restore these register values in case the in/out is triggering
	// system management mode and SMM wants to look at stuff in them.
	// (Can't do all of the PSW bits, restrict it to a safe subset)
	pushfl
	movl	VEFL(%ebp),%ebx
	andl	$FLAG_RESTORE_BITS,%ebx	
	andl	$~FLAG_RESTORE_BITS,0(%esp)
	orl		%ebx,0(%esp)
	movl	VEBX(%ebp),%ebx
	movl	VESI(%ebp),%esi
	movl	VEDI(%ebp),%edi
	movl	VEBP(%ebp),%ebp
	popfl

   	// "call far X86_V86_GPF_CS:V86_EXECCODE" sequence 
	// (X86_V86_GPF_CS is 16-bit segment)
	.byte	0x9a
	.long	V86_EXECCODE
	.short	X86_V86_GPF_CS
	
	// Update the saved register values in case SMM twiddled them. Ugh.
	// (Can't update all the flag bits, only do a safe subset)
	pushfl
   	xchg	%ebp,4(%esp)
	movl	%edi,VEDI(%ebp)
	movl	%esi,VESI(%ebp)
	movl	%ebx,VEBX(%ebp)
	andl	$FLAG_RESTORE_BITS,0(%esp)
	movl	VEFL(%ebp),%ebx
	andl	$~FLAG_RESTORE_BITS,%ebx
	orl		%ebx,0(%esp)
	popl	VEFL(%ebp)
	popl	VEBP(%ebp)

	pop		%ebx
	
	ret
