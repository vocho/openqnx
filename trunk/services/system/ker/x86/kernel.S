/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable license fees to QNX
 * Software Systems before you may reproduce, modify or distribute this software,
 * or any work that includes all or part of this software.   Free development
 * licenses are available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review this entire
 * file for other proprietary rights or license notices, as well as the QNX
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/
 * for other information.
 * $
 */
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA


    Authors: Jerome Stadelmann (JSN), Pietro Descombes (PDB), Daniel Rossier (DRE)
    Emails: <firstname.lastname@heig-vd.ch>
    Copyright (c) 2009 Reconfigurable Embedded Digital Systems (REDS) Institute from HEIG-VD, Switzerland
*/



/
/ This file contains all the entry points to the kernel.
/ There are serveral sections
/    Kernel call entry
/    Special kernel calls
/    Hardware interrupts
/    Exceptions
/    Interprocessor (IPI) interrupts
/
/ When we enter kernel code from user space via one of the above means
/ we switch to privity level 0 with an automatic stack switch pointing
/ at active[cpu]->reg. We stuff esp0 in the TSS to point at the threads
/ register save area before we return to a thread.
/ While kernel calls can only originate from user space, interrupts
/ and exceptions can also occur while in kernel space.
/

	.text

	.globl	ker_start
	.globl	__ker_exit
	.globl	__ker_exit_v86
	.globl	__ker_entry
	.globl	__ker_specialret
	.globl	__ker_intrevent
	.globl	intr_process_queue
	.globl	intr_done_chk_fault
	.globl	intr_done
	.globl	__nmi_lo
	.globl	__nmi_hi
	.globl	__v86_ret
	.globl	__keriret
	.globl	__exc0
	.globl	__exc1
	.globl	__exc3
	.globl	__exc4
	.globl	__exc5
	.globl	__exc6
	.globl	__exc7
	.globl	__exc7emul
	.globl	__exc7emul_end
	.globl	__exc8
	.globl	__exc9
	.globl	__exca
	.globl	__excb
	.globl	__excc
	.globl	__excd
	.globl	__exce
	.globl	__excf
	.globl	__exc10
	.globl	__exc11
	.globl	__exc12
	.globl	__intr_unexpected
	.globl	__hardcrash
	.globl	__fault_error
	.globl	__fault_noerror
	.globl	halt
	.globl	cpu_force_fpu_save
	.globl	__ker_sysenter
	.globl	disabled_perfregs

/	extrn	ker_call_table
/	extrn	actives
/	extrn	actives_fpu
/	extrn	actives_prp
/	extrn	aspaces_prp
/	extrn	ker_stack
/	extrn	xfer_handlers
/	extrn	interrupt_level
/	extrn	interrupt_mask
/	extrn	pulse_souls
/	extrn	inkernel
/	extrn	inspecret
/	extrn	intrevent_pending
/	extrn	tss
/	extrn	intr_fault_event
/	extrn	queued_event_priority
/	extrn	specialret
/	extrn	intrevent_drain
/	extrn	intrevent_add
/	extrn	memmgr
/	extrn	intr_tls
/	extrn	debug_attach_brkpts
/	extrn	debug_detach_brkpts
/	extrn	usr_fault
/	extrn	intrespsave
/	extrn	exc_stack
/	extrn	__v86
/	extrn	usr_ds
/	extrn	fault_code
/	extrn	shutdown
/	extrn	clock_handler
/	extrn	fpusave_alloc
/	extrn	fpuerr2code
/	extrn	cpupageptr
/	extrn	cpupage_segs
/	extrn	realmode_addr
/	extrn	resched
/	extrn	kdebug_callout
/	extrn	kprintf
/	extrn	_syspage_ptr
/	extrn	kererr
/	extrn	kercallptr
/	extrn	cycles
/	extrn	qtimeptr
/	extrn	callout_timer_value
/	extrn	xfer_restorestate
#include "asmoff.def"
#include "util.ah"

	.data
kdbg_fault_code:	.long	0
nmi_pending:		.long	0
nmi_eax:			.long	0
nmi_save:			.long	0
					.long	0



/ MACROS
/;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/
/ Useful macro definitions.
/

.macro GOT_HERE, chr, line
	push	%eax
#ifdef VARIANT_smp
	GETCPU	%ax,%eax
	add	realmode_addr,%eax
#else
	mov	realmode_addr,%eax
#endif
	movb	$&chr,0xb8000+(160*&line)(%eax)
	pop	%eax
.endm

#define	CPUPAGE_SEG		%fs

.macro TRPPOINT
	int	$0x20
.endm

.macro	PUSHREG
	// Force a DS: reference so that if the DS register is corrupted,
	// we'll get a GPF and go to the "__excd" handler to fix it up while
	// we still have stack space. By referencing the top of the stack, we
	// should ensure that we get a cache hit.

	// Instead of just referencing the stack, we OR (which causes a store operation)
	// in order to catch the case where the DS==CS (or other readonly segments) and
	// fix it up via GPF - otherwise a simple reference won't trap, and we'll start
	// doing nasty things in the thread entry area (small stack area we're on) during
	// __ker_exit

	ds
	orb		$0,(%esp)  // Fault?


	pusha
#ifdef	__SEGMENTS__
	push	%ds
	push	%es
	push	%fs
	push	%gs
	movw	%ss,%bp
	movw	%bp,%ds
	movw	%bp,%es
#endif
.endm

.macro POPREG
#ifdef	__SEGMENTS__
	#error NMI code assumes POPREG is atomic - see comment in handler
	pop	%gs
	pop	%fs
	pop	%es
	pop	%ds
#endif
	popa
.endm

.macro SAVE_PERFREGS, abscall
	/* save performance counter registers, if necessary */
	push	%eax
	mov		CPUDATA(%ebx),%eax
	cmpl	$disabled_perfregs,%eax
	je		111f

	push	%ecx
	push	%edx
	/* Pointer to cpu data in %eax. */
.if	\abscall
	mov		$cpu_save_perfregs, %edx
	call	*%edx
.else
	call 	cpu_save_perfregs
.endif
	pop		%edx
	pop		%ecx

111:
	pop		%eax
.endm






#ifdef	__SEGMENTS__
	#define NUM_REGS	12
#else
	#define NUM_REGS	8
#endif
#define	STK_ESI		(((NUM_REGS-8)*4)+4)
#define	STK_EIP		((NUM_REGS*4)+0)
#define	STK_CS		((NUM_REGS*4)+4)
#define	STK_EFL		((NUM_REGS*4)+8)

// These are here until we completely switch to a new assembler
#define FXSAVE_EAX	.byte 0x0f,0xae,0x00
#define FXSAVE_ECX	.byte 0x0f,0xae,0x01
#define FXSAVE_EDX	.byte 0x0f,0xae,0x02
#define FXRSTOR_EAX	.byte 0x0f,0xae,0x08
#define FXRSTOR_ECX	.byte 0x0f,0xae,0x09
#define FXRSTOR_EDX	.byte 0x0f,0xae,0x0a

#define	PAGE_FAULT_CODE	(SIGSEGV+(SEGV_MAPERR*256)+(FLTPAGE*65536))	// fault code if no memmgr handler

#ifdef	VARIANT_smp

/   extrn   intr_slock
/	extrn	ker_slock
/	extrn	cpunum

	.data
	exce_slock:	.long	0

	.global cpunum
	.type	 cpunum,@object
	.size	 cpunum,1

	.global	inkernel
	.type	 inkernel,@object
	.size	 inkernel,4

	inkernel:
		.byte	0
		.byte	0
		.byte	0
	cpunum:
		.byte	0


	.text

	#define	SMPREF(var,reg,scale)	var(,reg,scale)

	.macro LOCKOP
		.byte	0x0f0
	.endm

	.macro	SPINLOCK,	slock,reg,preserve
		.if &preserve <> 0
			push	&reg
		.endif
		99:
		mov	$1,&reg
		98:
		cmp	$0,&slock
		je	97f
		pause
		jmp 98b
		97:
		xchg	&reg,&slock
		test	&reg,&reg
		jne	99b
		.if &preserve <> 0
			pop	&reg
		.endif
	.endm

	.macro SPINUNLOCK,	slock
		movl	$0,&slock
	.endm

#else

	#define	SMPREF(var,reg,scale)	var

	.macro LOCKOP
	.endm

	.macro	SPINLOCK,	slock,reg,preserve
	.endm

	.macro SPINUNLOCK,	slock
	.endm

#endif

.macro CHKSTKOVER, reg, preserve
	.if &preserve <> 0
		push	&reg
	.endif
	mov		SMPREF(ker_stack,%ebp,4),&reg
	subl	$7000,&reg
	cmp 	%esp,&reg
	jb 		1f
	call 	deep
	1:
	.if &preserve <> 0
		pop	&reg
	.endif
.endm

.macro SETTSS, reg1, reg2, reg3
	mov		SMPREF(tss,&reg1,4),&reg1
	testl	$1,&reg1
	je		999f
	// We have a V86 thread running, so we have to make sure the
	// io permission bitmap in the TSS is up to date - allow V86 threads
	// to write to the first 256 ports with no exception, disallow
	// port access for normal user threads.
	xorl	$1,&reg1
	xorl	&reg3,&reg3 	// going to write all zeros into iomap
	testl	$_NTO_TF_V86,TFLAGS(%ebx)
	jne		998f
	notl	&reg3			// going to write all ones into iomap
998:
	movl	&reg3,TSS_IOMAP_DATA+0x00(&reg1)	// Update the iomap
	movl	&reg3,TSS_IOMAP_DATA+0x04(&reg1)
	movl	&reg3,TSS_IOMAP_DATA+0x08(&reg1)
	movl	&reg3,TSS_IOMAP_DATA+0x0c(&reg1)
	movl	&reg3,TSS_IOMAP_DATA+0x10(&reg1)
	movl	&reg3,TSS_IOMAP_DATA+0x14(&reg1)
	movl	&reg3,TSS_IOMAP_DATA+0x18(&reg1)
	movl	&reg3,TSS_IOMAP_DATA+0x1c(&reg1)
999:
	mov 	&reg2,4(&reg1)
.endm

/*
	 EBX has the active thread pointer, don't damage EAX
*/
.macro	FORCED_KERNEL_RESTORE
	mov	ARGS_ASYNC_IP(%ebx),%ecx
	mov	ARGS_ASYNC_TYPE(%ebx),%edx
	mov	%ecx,REG_OFF+REG_EIP(%ebx)
	mov	%edx,REG_OFF+REG_EAX(%ebx)
.endm

/*
	.global showit
	.data
	.align	4
	.space	1024
showit_stk:
msg: .ascii	"(%x)	"

	.text

showit:
	push	%eax
	mov	%esp,%eax
	mov	$showit_stk,%esp
	push	%eax
	push	%edx
	push	%ecx
	push	8(%eax)
	push	$msg
	call	kprintf
	add	$8,%esp
	pop	%ecx
	pop	%edx
	pop	%esp
	pop	%eax
	ret
*/

	.text

#ifdef	VARIANT_smp
__ker_exit_v86:
	//
	// Load EBX with actives[0] in case we use __ker_exit2. Don't have
	// have to check which CPU we're on - we know it's CPU 0 due to
	// code in memmgr/x86/v86.c
	//
	mov			actives,%ebx
1:
	movl		inkernel,%eax
	testl		$INKERNEL_NOW+INKERNEL_LOCK,%eax
	jnz			2f
	movl		%eax,%edx
	orl			$INKERNEL_NOW+INKERNEL_LOCK,%edx
	andl		$0x00ffffff,%edx		/ We know cpunum is zero
	lock; cmpxchg	%edx,inkernel
	jz			__ker_exit			/ We are the kernel
	testl		$INKERNEL_NOW+INKERNEL_LOCK,%eax
	jz			1b					/ Try again
2:
// Check if we have the lock
	cmpb		$0,cpunum
	jne			1b
	jmp			__ker_exit2
#endif

/ KERNEL CALL
/;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/
/ Kernel call entry point. Ker_start is used only once to start the first
/ thread on each processor. We do not have to protect access to the kernel
/ here since the code in idle() makes sure that only one CPU at a time
/ comes through.
/
/
ker_start:					/ Starting the kernel
	GETCPU	%bp,%ebp
#ifdef	VARIANT_smp
	movl	%ebp,%eax
	shl		$24,%eax
	orl		$INKERNEL_NOW+INKERNEL_LOCK,%eax
	movl	%eax,inkernel
#else
	movl	$INKERNEL_NOW+INKERNEL_LOCK,inkernel
#endif
	mov		SMPREF(ker_stack,%ebp,4),%esp
	mov		SMPREF(actives,%ebp,4),%ebx
	jmp		__ker_exit


__ker_entry:
	PUSHREG				/ Save all registers in thread register save area
	GETCPU	%bp, %ebp

	mov	SIZEOF_REG-(2*4)(%esp),%edx	/ Pointer to users stack frame
	add	$4,%edx				/ Step over return address, point at arguments

	lea		-REG_OFF(%esp),%ebx	/ Recover actives

	mov		SMPREF(ker_stack,%ebp,4),%esp      // switch to the kernel stack

	jmp		__common_ker_entry

/
/ Entry point to SYSENTER call, the fast kernel call entry point
/
#define		SYSEXIT_OPCODE 			.byte	0x0f,0x35

__ker_sysenter:
	/ coming in on kernel stack already
	/ Push some regs so we can get to actives
	push	%ebx
	push	%ebp
	GETCPU	%bp, %ebp
	mov		SMPREF(actives,%ebp,4),%ebx

    // NOTE: we save off edi and load it with eflags so that the code down
	//       in debug_exc can do its thing and then jump right after here
	//       without it having to duplicate even more code.
    //
	//       Keep in mind that if this or the preceeding 4 instructions
	//       are changed you will need to update debug_exc where it
	//       handles the case of the TF flag being set.
    //
	movl	%edi,REG_OFF+REG_EDI(%ebx)
	pushf
	popl	%edi

	testl	$2,REG_OFF+REG_CS(%ebx)
	jz		from_proc
	/ Flip the bit in EFLAGS image so we know we can exit through SYSEXIT
	orl		$SYSENTER_EFLAGS_BIT,%edi
from_proc:
	// This is the label that debug_exc will jump to if appropriate.  It
	// will have taken care of making sure the state is identical to what
	// we would see if we got to this point normally.  Single stepping
	// through SYSEXIT is not allowed.
fixup_tf_entry:

	/ Now save the regs
	/ eip is in edx, esp is in ecx, edi has the flags

	orl		$X86_PSW_IF,%edi // mark intrs as enabled in saved flags

	movl	%esi,REG_OFF+REG_ESI(%ebx)
	movl	%edx,REG_OFF+REG_EDX(%ebx)
	movl	%ecx,REG_OFF+REG_ECX(%ebx)
	movl	%eax,REG_OFF+REG_EAX(%ebx)
	movl	%edx,REG_OFF+REG_EIP(%ebx)
	movl	%ecx,REG_OFF+REG_ESP(%ebx)
	lea		4(%ecx),%edx			// Point at ker call arguments
	pop		%ecx
	movl	%ecx,REG_OFF+REG_EBP(%ebx)
	movl	(%esp),%ecx
	movl	%ecx,REG_OFF+REG_EBX(%ebx)
	movl	%edi,REG_OFF+REG_EFL(%ebx)

__common_ker_entry:
#ifdef VARIANT_instr
	SAVE_PERFREGS 0
#endif

#ifdef	VARIANT_smp

	//NOTE: If the register used to save the kernel call number is
	//changed, there's code in 'force_kernel' that needs to change as
	//well. See the "NOTE:" comment in that routine.
	mov		%eax,%esi	// save kernel call number
acquire_kernel_attempt:
	sti
    // Wait for need to run to clear if we're not on the right CPU.
1:
    cmpl    $0,need_to_run
	jz		3f
	cmpl    %ebp,need_to_run_cpu
	je		3f
	pause
	jmp		1b

   	// See if anybody else is in the kernel
3:
	mov		inkernel,%eax
	test	$INKERNEL_NOW,%eax
	jnz		1b

	cli
end_acquire_kernel_attempt:

	mov		%eax,%edi
	andl	$0x00ffffff,%edi
	mov 	%ebp,%ecx
	shl		$24,%ecx
	orl		%edi,%ecx		/ Set cpunum
	orl		$INKERNEL_NOW,%ecx
	lock; cmpxchg	%ecx,inkernel
	jnz		acquire_kernel_attempt
	// We are the kernel
	mov		%esi,%eax	// restore kernel call number
#else
	LOCKOP
	orl		$INKERNEL_NOW,inkernel	/ In the kernel (restartable)
#endif

	sti
	cld

	mov		%eax,SYSCALL(%ebx)
	mov		TFLAGS(%ebx),%ecx
	and		$~(_NTO_TF_KERERR_SET+_NTO_TF_BUFF_MSG),%ecx
	mov		%ecx,TFLAGS(%ebx)
	cmp		$__KER_BAD,%eax
	jae		bad_func
	push	%ebx  / store on the stack for later
	push	%edx
	push	%ebx
#if defined(VARIANT_instr)
	call	*_trace_call_table(,%eax,4)
#else
	call	*ker_call_table(,%eax,4)
#endif
	// Recover the %ebx register.  Don't worry about restoring the
	// stack pointer since we're about to __ker_exit
	mov		0x8(%esp), %ebx

	test	%eax,%eax
	jge		set_err

enoerror:

#ifndef	VARIANT_smp
__ker_exit_v86:
#endif

__ker_exit:
/
/ Check and make sure the cpupage seg is correct and fix it if not.
/
	GETCPU	%bp, %ebp
	movw	CPUPAGE_SEG,%ax
	cmpw	SMPREF(cpupage_segs,%ebp,2),%ax
	jne		fixcpupage

	mov		SMPREF(actives,%ebp,4),%ebx
#ifdef VARIANT_smp
	orb		$((INKERNEL_NOW+INKERNEL_EXIT+INKERNEL_LOCK)>>8),inkernel+1 / In the kernel (locked)
#else
	orl		$((INKERNEL_NOW+INKERNEL_EXIT+INKERNEL_LOCK)),inkernel / In the kernel (locked)
#endif
	sti					/ In case someone called here with intrs off

/
/ Check for any pending events.
/
	cmpl	$0,intrevent_pending
	jne		__ker_intrevent

/
/ Check for a process switch since we may need to remove breakpoints
/
	mov		SMPREF(actives_prp,%ebp,4),%eax
	cmp		PROCESS(%ebx),%eax
	jne		dbg_check
dbgret:

/
/ Check for a aspace switch since we may need to change aspace mappings
/
	leal	SMPREF(aspaces_prp,%ebp,4),%ecx
	mov		ASPACE_PRP(%ebx),%eax
	cmp		(%ecx),%eax
	jne		aspace_switch
aspaceret:

/
/ Check for a process switch since we may need to add breakpoints and change PLS
/
	mov		PROCESS(%ebx),%eax
	cmp 	SMPREF(actives_prp,%ebp,4),%eax
	jne		prp_switch
prpret:

/
/ Check for special actions
/
	testl	$_NTO_TF_SPECRET_MASK,TFLAGS(%ebx)
	jnz		__ker_specialret

/
/ Check FPU owner and possibly set the TS bit
/
	mov 	SMPREF(actives_fpu,%ebp,4),%edx
	cmp		%ebx,%edx
	je		fpuret
	smsww	%ax
	test	$8,%eax		/ Is TS (task switched) flag already set?
	jnz		fpuret
	orl		$8,%eax		/ Set the TS (task switched) flag
	lmsww	%ax

fpuret:

#if defined(VARIANT_instr)
	movl	ker_exit_enable_mask,%edx
	test	%edx,%edx
	jz		__skip_trace_kerexit
	push  %eax
	push  %ebx
	call  _trace_ker_exit
	pop   %ebx
	pop   %eax
__skip_trace_kerexit:
#endif

	cli
	cmpl	$0,intrevent_pending
	jne		__ker_intrevent

	xor		%eax,%eax
	xchg	ATFLAGS(%ebx),%eax
	test	%eax,%eax
	jne		__ker_atflags

__nmi_lo:
	xchg	%al,nmi_pending
	test	%al,%al
	jne		__ker_nmi

#ifdef	VARIANT_smp
//	andb	$~((INKERNEL_NOW+INKERNEL_EXIT+INKERNEL_LOCK)>>8),inkernel+1
	movb	$0,inkernel+1
#else
	movl	$0,inkernel
#endif

__ker_exit2:
#ifdef VARIANT_smp
	GETCPU	%bp, %ebp

// Force ourselves into the kernel if there are any async flags
// set on the thread so that we can deal with them.
//
// Also, if need_to_run is set and we're the target cpu, we force
// whoever is executing to make a null syscall so that we'll
// eventually return through the scheduler and take care of
// whoever it is that needs to run.
//
// note: the code sequence for forcing a null syscall is
//       shamelessly ripped off from force_kernel
//
/ This code slows down the ker exit sequence, should be moved to interrupt handling case

	mov		ATFLAGS(%ebx),%eax
	cmpl	$0,%eax
	jne		1f

    cmpl    $0,need_to_run
	je      3f
    movl    need_to_run_cpu,%edx
    cmpl    %ebp,%edx
    jne     3f

1:
	testl	$0x03,REG_OFF+REG_CS(%ebx)
	jz      3f                         // can't force the kernel if we were in the kernel

    // force this thread to do a null syscall so we'll resched
2:
	mov		$_NTO_ATF_FORCED_KERNEL,%edx
	or		%eax,%edx					/ turn on new bit(s)
	lock; cmpxchg	%edx,ATFLAGS(%ebx)	/ put new bits back atomically
	jnz		2b
	test	$_NTO_ATF_FORCED_KERNEL,%eax
	jnz		3f
	mov		REG_OFF+REG_EAX(%ebx),%eax
	mov		%eax,ARGS_ASYNC_TYPE(%ebx)
	mov		REG_OFF+REG_EIP(%ebx),%eax
	mov		%eax,ARGS_ASYNC_IP(%ebx)
	movl	$__KER_NOP,REG_OFF+REG_EAX(%ebx)
	mov		kercallptr,%eax
	mov		%eax,REG_OFF+REG_EIP(%ebx)
3:


// Sometimes we aren't able to drain the interrupt event queue
// so here we check if we've got anything pending that hasn't
// been processed yet.  If there are some interrupt events, we
// send an IPI to the next cpu numerically and hope that they're
// able to take care of it.
//
// note: there is another check, identical to this one down in
//       case4/5 of intr_done.
//
	cmpl    $0,intrevent_pending
	je      9f

#ifndef NDEBUG
	/* Make sure we are on the kernel stack!!! */
	cmp		run_ker_stack_bot,%esp
	jb		1f
	cmp		run_ker_stack_top,%esp
	jbe		2f
1:	jmp		__hardcrash
2:
#endif

// If interrupt counter is not zero, somebody is either in interrupt handling
// or temporarily increase the counter; he will either deliver the interrupt
// events or go to here or case 4/5  to do the check again
	mov		inkernel,%eax
	testl	$INKERNEL_INTRMASK,%eax
	jne		9f
// If somebody is in kernel but not set EXIT flag yet, he will go through
// ker_exit sequence and deliver the interrupt events.
	testl	$INKERNEL_NOW,%eax
	je		1f
	testl	$INKERNEL_EXIT,%eax
	je		9f

1:
    // tell another cpu to deal with this.  we pick another cpu
    // by incrementing our cpu number and doing a modulo on the
    // number of cpu's present.
    movl    %ebp,%eax
    incl    %eax                  // increment our cpu number
	cmpl	%eax,num_processors
	jne		1f
	xorl	%eax,%eax			// wrap-around on CPU number
1:
	movl	$IPI_CHECK_INTR,%edx   // ipi cmd goes in %edx
	call	send_ipi

9:
#endif
#ifdef VARIANT_instr
	/* Restore performance registers. */
	push	%eax

	/* Zero out actives_pcr to ensure stale pointer
	isn't maintained. */
	movl	$0,SMPREF(actives_pcr,%ebp,4)

	movl	CPUDATA(%ebx),%eax
	cmpl	$disabled_perfregs,%eax
	je		1f

	/* Save active performance counter thread for reference
	in kernel ISR if necessary. */
	mov		%ebx,SMPREF(actives_pcr,%ebp,4)

	push	%ecx
	push	%edx
	call	cpu_restore_perfregs /* pointer to perfregs in %eax */
	pop		%edx
	pop		%ecx

1:
	pop		%eax
#endif
/
/ Set thread local storage
/
	mov		SMPREF(cpupageptr,%ebp,4),%ecx
	mov		TLS(%ebx),%eax
	mov		%eax,CPUPAGE_TLS(%ecx)


/
/ See if we came in through sysenter and can exit through sysexit
/
	movl	REG_OFF+REG_EFL(%ebx),%eax
	testl	$SYSENTER_EFLAGS_BIT,%eax
	jz		not_sysexit

	// Came in through SYSENTER, can go out through sysexit
	// Turn off the I bit in the saved image so the popf just before
	// the SYSEXIT doesn't enable interrupts - we need to use sti
	// to enable them so that the SYSEXIT executes with intrs off.
	andl	$~(SYSENTER_EFLAGS_BIT|X86_PSW_IF),%eax
	movl	%eax,REG_OFF+REG_EFL(%ebx)
	lea		REG_OFF+REG_SS+4(%ebx),%eax	/ Set tss.esp to top of thread reg.
	SETTSS	%ebp,%eax,%edx

	lea		REG_OFF(%ebx),%esp
	POPREG
	//
	// Note that any changes to the stack manipulation in the SYSEXIT code
	// might have ramifications to the short stack handling in case 4 of the
	// "intr_entry_nmi_start" routine down below. Buyer beware....
	//
	/ Has EIP been modified?
	cmp		%edx,(%esp)
	jnz		eip_mod

	add		$8,%esp
	movl	4(%esp),%ecx
	popf
	sti
	SYSEXIT_OPCODE

eip_mod:
	/ EIP modified, might be a kernel call restart.
	/ Make sure edx is correct and interrupts are on
	orl	$X86_PSW_IF,8(%esp)
	iret

not_sysexit:
	testl	$_NTO_TF_V86,TFLAGS(%ebx)
	jnz		__v86_ret
	lea		REG_OFF+REG_SS+4(%ebx),%eax	/ Set tss.esp to top of thread reg.
	SETTSS	%ebp,%eax,%edx

__keriret:
	lea		REG_OFF(%ebx),%esp

	POPREG
__keriret_iret_instr:
	iret

__v86_ret:
	mov		$V86_REG+SIZEOF_V86REG,%eax
	SETTSS	%ebp,%eax,%edx
	lea		-SIZEOF_V86REG(%eax),%esp
	POPREG
	iret
__nmi_hi:

set_err:
	// NOTE: We're setting the error on the initial active thread,
	// not on what's in actives[CURRCPU] after the call.
	jnz		real_err
#ifdef VARIANT_smp
	orb		$((INKERNEL_NOW+INKERNEL_EXIT+INKERNEL_LOCK)>>8),inkernel+1 / In the kernel (locked)
#else
	orl		$((INKERNEL_NOW+INKERNEL_EXIT+INKERNEL_LOCK)),inkernel / In the kernel (locked)
#endif
	mov		%eax,REG_OFF+REG_EAX(%ebx)
	jmp		enoerror

bad_func:
	mov		$ENOSYS,%eax
		// fall through
real_err:
	mov		%eax,%edx
	mov		%ebx,%eax
	push	$__ker_exit
	jmp		kererr

dbg_check:
	test	%eax,%eax
	je		dbgret
	cmp		$0,DEBUGGER(%eax)
	je		dbgret
	lea		SMPREF(aspaces_prp,%ebp,4),%ecx
	cmp		(%ecx),%eax
	je		dbg2
	push	%eax
	push	%ecx
	push	%eax
	call	*MEMMGR_ASPACE+memmgr	/ Switch address spaces (ebx is not modified)
	add		$8,%esp
	pop		%eax
dbg2:
	push	%ebx
	mov		DEBUGGER(%eax),%eax
	call	*debug_detach_brkpts	/ Possibly yes so call to remove soft breakpoints (prp in EAX)
	pop		%ebx
	jmp		dbgret

aspace_switch:
	test	%eax,%eax
	je		aspaceret
	push	%ecx
	push	%eax
	call	*MEMMGR_ASPACE+memmgr	/ Switch address spaces (ebx is not modified)
	add		$8,%esp
	jmp		aspaceret

prp_switch:
	mov		%eax,SMPREF(actives_prp,%ebp,4)
	mov 	SMPREF(cpupageptr,%ebp,4),%ecx
	mov		PLS(%eax),%edx
	mov		%edx,CPUPAGE_PLS(%ecx)
	mov		DEBUGGER(%eax),%eax
	test	%eax,%eax
	je		prpret
	andl	$~SYSENTER_EFLAGS_BIT,REG_OFF+REG_EFL(%ebx)		// Single stepping through the SYSEXIT sequence is not allowed
	push	%ebx
	call	*debug_attach_brkpts	/ Possibly yes so call to remove soft breakpoints (prp in EAX)
	pop		%ebx
	jmp		prpret

/
/ Call a routine to dequeue all pending intrevents.
/
__ker_intrevent:
	sti
	push	$__ker_exit
	jmp		intrevent_drain


/
/ On entry
/	ebx = actives[CURCPU]
/
__ker_specialret:
	mov		SMPREF(ker_stack,%ebp,4),%esp
#ifdef	VARIANT_smp
	cli
	testl	$_NTO_ATF_FORCED_KERNEL,ATFLAGS(%ebx)
	jz		1f
	/
	/ have to restore original registers - specialret looks at them
	/
	lock; andl	$~_NTO_ATF_FORCED_KERNEL,ATFLAGS(%ebx)
	FORCED_KERNEL_RESTORE
1:
	sti
#endif
	mov		%ebx,%eax
	push	$__ker_exit
	jmp		specialret


/
/ deal with async thread flags being set
/
/ ebx == actives[0], eax == ATFLAGS[ebx]
/
__ker_atflags:

#ifdef	VARIANT_smp
	test	$_NTO_ATF_FORCED_KERNEL,%eax
	je		1f
	FORCED_KERNEL_RESTORE
1:
#endif

	sti
	push	$__ker_exit
#ifndef	VARIANT_smp
	testl	$_NTO_ATF_TIMESLICE,%eax
	je		1f
	push	resched
1:
#else

	testl	$_NTO_ATF_SMP_RESCHED+_NTO_ATF_TIMESLICE,%eax
	jnz		2f

    // if need_to_run is set and we're the target cpu, we should
	// call smp_resched
	cmpl  $0,need_to_run
	je    1f
	cmpl  %ebp,need_to_run_cpu
	jne   1f
2:
	push  resched

1:
#endif

/ Important: this has to be done before anything that changes active
/ Fortunately, we should never get an SMP_EXCEPTION and FPUSAVE simultaneously
	testl	$_NTO_ATF_FPUSAVE_ALLOC,%eax
	je		1f
	push	$fpusave_alloc
1:

#ifdef	VARIANT_smp
	/ NOTE: This has to be last one checked
	testl	$_NTO_ATF_SMP_EXCEPTION,%eax
	jz		1f
	mov		ARGS_ASYNC_FAULT_ADDR(%ebx),%edi
	mov		ARGS_ASYNC_CODE(%ebx),%eax
	cmp		$PAGE_FAULT_CODE,%eax
	jne		2f
	mov		ARGS_ASYNC_FAULT_TYPE(%ebx),%esi
	sti
	call	handle_page_fault
	je		1f		/ if zero, page fault has been handled
2:
	mov		%ebx,%edx
	mov		%edi,%ecx
	mov		%ecx,%ebx	/	watcom wants ebx, gcc wants ecx
	push	$usr_fault
1:
#endif

	sti
	ret			/ start the async routines executing

//
//	Process an NMI interrupt, now that we're in a nice state
//
__ker_nmi:
	sti
	int		$2
	jmp		__ker_exit

fixcpupage:
	mov		SMPREF(cpupage_segs,%ebp,2),CPUPAGE_SEG
	jmp		__ker_exit


/ HARDWARE INTERRUPTS
/;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/
/
/ Overall interrupt handler for X86 systems.
/ This code is not used in place. Rather, it is copied elsewhere and
/ intermixed with interrupt controller bursts from the startup module.
/ Make sure that only position independent code transfers are used here
/ (referencing absolute data addresses is OK).
/
	.globl	intr_entry_start
	.globl	intr_entry_end
	.globl	intr_entry_nmi_start
	.globl	intr_entry_nmi_end

//
// Start of prefix for NMI processing. These are a real pain since they can		(PDB: NMI = Non Maskable Interrupt)
// nail you at inopportune times. Before we can process it, let's check
// to see what state we're in.
//
//	1. In user code	=> go ahead and handle it
//	2. In system code with interrupts enabled, => go ahead and handle it
//	3. Interrupts disabled, EIP outside __nmi_lo to __nmi_hi range => defer		(PDB: EIP = Enhanced Instruction Pointer)
//  4. EIP in __nmi_lo to __nmi_hi range => reset to __ker_exit state
//
// NOTE: ESP might be pointing into the 'nmi_overflow' area of a thread_entry	(PDB: ESP = Extended Stack Pointer)
// here, so we can't use any additional stack space in this code :-(.
//

intr_entry_nmi_start:
	movl	%eax,%ss:nmi_eax
	movl	8(%esp),%eax	/ check interrupt flag
	test	$X86_PSW_IF,%eax / if interrupts enabled, cool
	jnz		1f
	testl	$X86_PSW_VM,%eax / if V86, cool
	jnz		1f
	movl	4(%esp),%eax	/ check original CS value
	test	$2,%eax
	jnz		1f				/ if in user code, cool
	cmpl	$X86_V86_GPF_CS,%eax
	je		1f				/ if handling a V86 GPF, cool
	movl	0(%esp),%eax	/ see where we were in the kernel
	cmpl	$__nmi_lo,%eax
	jbe		2f				/ not in special range
	cmpl	$__nmi_hi,%eax
	ja		2f				/ not in special range

	//
	// Case 4 -- really hairy. We want to reset so it looks like we're
	// 	at the __ker_exit location, but we can't just back up the
	//	saved EIP. We might have already popped the register set,
	//	so the NMI will have damaged what's in the thread_entry save
	//	area.
	//
	addl	$4,%esp		/ throw away offset
	popl	nmi_save+4	/ save segment
	popl	nmi_save+0	/ save flags

	GETCPU	%ax, %eax
	mov		SMPREF(actives,%eax,4),%eax
	addl	$REG_OFF + SIZEOF_REG,%eax
	cmpl	%eax,%esp
	ja		10f				/ on normal kernel stack
	sub		%esp,%eax
	cmpl	$SIZEOF_REG,%eax
	jae		10f				/ on normal kernel stack

	// On the short stack, restore the register save area to proper condition.
	// NOTE: We're assuming that POPREG is atomic. If we start
	// supporting saving/restoring of segment registers (yeah, right :-),
	// this code will have to be spiffied up.
	// We're also assuming that if we need to push EIP/CS/EFL, we're doing
	// the SYSEXIT sequence
	cmpl	$0x14,%eax
	jae		17f
	cmpl	$0x0c,%eax
	jae		18f
19:
	pushl	nmi_save+0	//EFL
18:
	pushl	usr_cs		//CS
	pushl	%edx 		//EIP
17:
	movl	nmi_eax,%eax
	pusha

10:
	GETCPU	%ax, %eax
	mov		SMPREF(ker_stack,%eax,4),%esp
	pushl	nmi_save+0
	pushl	nmi_save+4
	pushl	$__ker_exit
	jmp		1f

   	//
	// case 3 -- not too bad, just mark it as pending.
	//
2:
	movb	$1,%ss:nmi_pending	/ indicate we need to process NMI later

	// Return to original code, keep further NMI's disabled by not doing
	// an IRET. Instead we walk the stack frame around so we can pop the
	// flags and then do a RET.
	xchgl	%eax,4(%esp)	/ get segment, stash return offset
	xchgl	%eax,8(%esp)	/ get flags, stash return segment
	movl	%eax,0(%esp)	/ stash flags
	movl	%ss:nmi_eax,%eax
	popfl					/ restore original flags
	lret					/ return to original location

1:
	//
	// Case 1, 2 -- process the interrupt.
	//

	movl	%ss:nmi_eax,%eax
	movb	$0,%ss:nmi_pending

	/
	/ End of prefix for NMI processing. Fall into IRQ handler
	/

intr_entry_start:
	PUSHREG
	LOCKOP
	incl	inkernel
	GETCPU	%bp, %ebp
	testl	$X86_PSW_VM,STK_EFL(%esp)	/ Check for V86
	jnz		from_v86
	mov		STK_CS(%esp),%eax		/ Load pushed CS
	cmpl	$X86_V86_GPF_CS,%eax	/ If handling V86 GPF, pretend user mode
	je		from_v86
	test	$3,%eax				/ If priv 0 then we are nested
	jz		nested
from_v86:
	mov		$3,%eax			/ Fake CS so later tests think we came from user mode
	mov		SMPREF(ker_stack,%ebp,4),%esp

#ifdef VARIANT_instr
	push 	%ebx
	mov		SMPREF(actives,%ebp,4), %ebx
	SAVE_PERFREGS 1
	pop		%ebx
#endif
nested:

	push	%eax			/ save CS value

	SPINLOCK intr_slock,%eax,0
intr_entry_end:
intr_entry_nmi_end:


/
/ Fall into startup/generated interrupt level identification code.
/ This is the end point of the code that gets copied.
/

autoadd:
	mov		INTR_LEVEL(%ebx),%eax
	mov		%ebx,%edx
	call	interrupt_mask
	mov		INTR_AREA(%ebx),%eax
	mov		INTR_THREAD(%ebx),%edx
	mov		%ebx,%ecx
	orl		$AP_INTREVENT_FROM_IO_FLAG,%edx
	call	intrevent_add
#ifdef VARIANT_instr
	movl	int_exit_enable_mask,%edx
	test	%edx,%edx
	jz		__skip_trace_ihexit1
	push	%eax
	mov		INTR_AREA(%ebx),%eax
	push	%eax
	push	%esi
	call	add_ktrace_int_handler_exit // add_ktrace_int_handler_exit(INTRLEVEL* lp, const struct sigevent *ev)
	pop		%esi
	pop		%eax
	pop		%eax
__skip_trace_ihexit1:
#endif

	// Call the irq handler exit marker for monartis
	movl	mt_tracebuf_addr,%edx
	test	%edx,%edx
	jz		__skip_irq_handler_exit1
	push	%eax
	mov		INTR_LEVEL(%ebx),%eax
	test	%eax,%eax
	jz		__skip_irq_handler_exit1		/ Ignore the irq timer
	call	mt_trace_irq_handler_exit
	pop		%eax
__skip_irq_handler_exit1:

	jmp		noeve

intr_process_queue:
	SPINUNLOCK intr_slock
#if defined(VARIANT_instr)
	movl	int_enter_enable_mask,%ebx
	test	%ebx,%ebx
	jz		__skip_trace_ienter
	push	%edx
	push	%eax
	push	%esi
	call	add_ktrace_int_enter
	pop		%esi
	pop		%eax
	pop		%edx
__skip_trace_ienter:
#endif

	// Call the irq entry marker for monartis
	movl	mt_tracebuf_addr,%ebx
	test	%ebx,%ebx
	jz		__skip_irq_enter
	push	%eax
	mov		INTRLEVEL_QUEUE(%esi),%ebx
	test	%ebx,%ebx
	jz		__skip_irq_enter
	mov		INTR_LEVEL(%ebx),%eax
	test	%eax,%eax
	jz		__irq_timer_entry
	call	mt_trace_irq_entry
	pop		%eax
	jmp		__skip_irq_enter
__irq_timer_entry:
	call	mt_trace_irq_timer_entry
	pop		%eax
__skip_irq_enter:

	GETCPU	%bp, %ebp
	mov		SMPREF(cpupageptr,%ebp,4),%ecx
	pushl	CPUPAGE_STATE(%ecx) / preserve old interrupt state
	movl	$1,CPUPAGE_STATE(%ecx) / say we are handling an interrupt
	sti
	push	%eax	/ preserve for EOI code
	cld
	movl	$intr_tls,CPUPAGE_TLS(%ecx)

	mov		INTRLEVEL_QUEUE(%esi),%ebx/ get head of queue
	push	intrespsave		/ Save old fault esp
#if defined(VARIANT_instr)
	pushl	%esi
#endif
	test	%ebx,%ebx
	jz		intrdone
	push	SMPREF(aspaces_prp,%ebp,4)
intrnext:
	mov		INTR_HANDLER(%ebx),%edx
#ifdef VARIANT_instr
	movl	int_enter_enable_mask,%eax
	test	%eax,%eax
	jz		__skip_trace_ihenter
	push	%edx
	push	%ebx
	push	%esi
	call	add_ktrace_int_handler_enter //	add_ktrace_int_handler_enter(ilp, isr);
	pop		%esi
	pop		%ebx
	pop		%edx
__skip_trace_ihenter:
#endif

	// Call the irq handler entry marker for monartis
	movl	mt_tracebuf_addr,%eax
	test	%eax,%eax
	jz		__skip_irq_handler_entry
	mov		INTR_LEVEL(%ebx),%eax
	test	%eax,%eax
	jz		__skip_irq_handler_entry 		/ Ignore the irq timer
	push	%edx
	call	mt_trace_irq_handler_entry
	pop		%edx
__skip_irq_handler_entry:

	or		%edx,%edx
	je		autoadd

	mov		INTR_THREAD(%ebx),%eax
	lea		SMPREF(aspaces_prp,%ebp,4),%ecx
	mov		ASPACE_PRP(%eax),%eax
	test	%eax,%eax
	je		intcall
	cmp		(%ecx),%eax
	je		intcall
	push	%ecx
	push	%eax
	call	*MEMMGR_ASPACE+memmgr
	add		$8,%esp
	mov		INTR_HANDLER(%ebx),%edx

    //
    // Here we have an extra TLB invalidate which is a serializing
	// instruction on the P-II and P-III cpu's (ppro arch).  This
	// is unfortunately necessary because of some unusual behaviour
	// that you get when you have a high rate of interrupts (>10k per
	// second).  Without this serializing instruction the processor
	// will (seemingly) randomly get a GPF when calling the interrupt
	// handler or it will jump to an incorrect address or it will
	// make the call ok but then run into trouble while executing the
	// interrupt handler (the stack pointer appears to be pointing
	// somewhere strange).
	//
	// We do not currently understand why this happens, only that if
	// the address space gets switched an extra serializing instruction
	// is necessary.  Other serializing instructions that work are
	// things like cpuid, mov %eax,%dr3, and mov %eax,%cr0.  See volume
	// 3, section 7.3 (page 7-12) of the Intel architecture manual for
	// more details about serializing instructions.
	//
	// The serializing instructions all have about the same worst case
	// cycle cost (7000 cycles on a P-III 550Mhz).  I chose to use the
	// TLB invalidate since the cost is the same for all of them and
	// if any side effects are going to happen, a TLB invalidate is
	// most likely to have good side effects (from the point of view
	// of keeping the system stable).  Plus since we just invalidated
	// the TLB during the address space switch, the normal cost is quite
	// cheap.
	//
cli
    mov     %cr3,%eax    // serializing instruction (tlb-invalidate)
    mov     %eax,%cr3
sti

intcall:
	mov		INTR_AREA(%ebx),%eax
	push	%ebx
	push	INTR_ID(%ebx)
	push	%eax
	//NYI: This actually doesn't work if we're handling interrupts
	//on multiple CPU's - it should be a per-CPU variable. Since it's
	//only used restoring from exceptions in ISR's and those likely will
	//take the system down due to a permanently asserted interrupt, we'll
	//live with it for right now. Once we switch over to using EBP as
	//a per-CPU data area pointer, we can revisit the situation.
	mov		%esp,intrespsave		/ Save esp for restoration if intr handler faults
	call	*%edx
intrret:
	add		$12,%esp
#ifdef VARIANT_instr
	movl	int_exit_enable_mask,%edx
	test	%edx,%edx
	jz		__skip_trace_ihexit2
	push	%eax
	push	%eax
	push	%esi
	call	add_ktrace_int_handler_exit // add_ktrace_int_handler_exit(INTRLEVEL* lp, const struct sigevent *ev)
	pop		%esi
	pop		%eax
	pop		%eax
__skip_trace_ihexit2:
#endif

	// Call the irq handler exit marker for monartis
	movl	mt_tracebuf_addr,%edx
	test	%edx,%edx
	jz		__skip_irq_handler_exit2
	mov		INTR_LEVEL(%ebx),%edx
	test	%edx,%edx
	jz		__skip_irq_handler_exit2 		/ Ignore the irq timer
	push	%eax
	mov		%edx,%eax
	call	mt_trace_irq_handler_exit
	pop		%eax
__skip_irq_handler_exit2:

	or		%eax,%eax
	jz		noeve
	mov		INTR_THREAD(%ebx),%edx
	mov		%ebx,%ecx
	orl		$AP_INTREVENT_FROM_IO_FLAG,%edx
	call	intrevent_add
noeve:
	mov		INTR_NEXT(%ebx),%ebx
	test	%ebx,%ebx
	jnz		intrnext

	pop		%eax

#if defined(VARIANT_instr)
	testb	$3,20(%esp)	/ check saved CS (from intr_entry_start seq)
#else
	testb	$3,16(%esp)	/ check saved CS (from intr_entry_start seq)
#endif
	jnz		intrdone	/ If not from kernel, ker_exit will restore aspace
	test	%eax,%eax
	je		intrdone
	lea		SMPREF(aspaces_prp,%ebp,4),%ecx
	cmp		(%ecx),%eax
	je		intrdone
	push	%ecx
	push	%eax
	call	*MEMMGR_ASPACE+memmgr
	add		$8,%esp
intrdone:
#if defined(VARIANT_instr)
	movl	int_exit_enable_mask,%ebx
	test	%ebx,%ebx
	jz		__skip_trace_iexit
	call	add_ktrace_int_exit
__skip_trace_iexit:
	add		$4, %esp
#endif

	// Call the irq exit marker for monartis
	movl	mt_tracebuf_addr,%ebx
	test	%ebx,%ebx
	jz		__skip_irq_exit
	mov		INTRLEVEL_QUEUE(%esi),%ebx
	test	%ebx,%ebx
	jz		__skip_irq_enter
	mov		INTR_LEVEL(%ebx),%eax
	test	%eax,%eax
	jz		__irq_timer_exit
	call	mt_trace_irq_exit
	jmp		__skip_irq_exit
__irq_timer_exit:
	call	mt_trace_irq_timer_exit
__skip_irq_exit:

	pop		intrespsave		/ restore old fault esp
	pop		%eax			/ restore for EOI code
	mov		SMPREF(cpupageptr,%ebp,4),%ecx
	cli
	popl	CPUPAGE_STATE(%ecx) / restore old interrupt state indicator
	SPINLOCK intr_slock,%ecx,0
	ret

/
/	All done dispatching this interrupt. An EOI has been sent to the
/	interrupt controller by the startup/generated code. Check to see
/	if a cpu exception has occurred and handle it if so. If not,
/	figure out how to continue the interrupted context.
/	Interupts are disabled.
/
intr_done_chk_fault:
	/;EAX - sigcode
	/;EDX - reference address

	SPINUNLOCK intr_slock

	LOCKOP
	decl	inkernel

	test	%eax,%eax
	jz		intr_done2

	pop		%edi		/ remove saved CS from stack (from intr_entry_start seq)
	testl	$0x03,%edi	/ were we in the kernel when interrupt occured?
	mov		%edx,%edi	/ exc wants addr in EDI
	jz		sys_exc		/ goto if in kernel previously
	jmp		usr_exc		/ goto if in user thread previously


/
/	All done dispatching this interrupt. An EOI has been sent to the
/	interrupt controller by the startup/generated code.
/	Figure out how to continue the interrupted context.
/	Interupts are disabled.
/
intr_done:
	SPINUNLOCK intr_slock

	LOCKOP
	decl	inkernel
intr_done2:
	GETCPU	%bp, %ebp

	pop		%eax				/ get saved CS (from intr_entry_start seq)
	test	$3,%eax				/ If priv 0 then we are nested
/
/ We have 6 cases to consider (3 in non-smp).
/
/          This CPU      Another CPU    Action
/		-------------------------
/ case1    from user     in user        Become the kernel
/ case2    from user     in kernel      Check preempt and maybe ipi (SMP)
/ case3    from kernel   in user        Check preempt and maybe become kernel
/ case4    from intr     in user        Return to user
/ case5    from intr     in kernel      Return to user (SMP)
/ case6    spin kentry   *              Try to become kernel, else return (SMP)
/
/ We can examine the previous privity, inkernel, and cpunum
/ to figure out where we came from and where to go
/
	jz	from_kerorintr

from_user:

case1:
#ifdef	VARIANT_smp
	/ See if we can become the kernel
1:
	mov		inkernel,%eax
	test	$INKERNEL_NOW+INKERNEL_LOCK,%eax
	jnz		case2
	mov		%eax,%edx
	mov 	%ebp,%ecx
	andl	$0x00ffffff,%edx
	shl		$24,%ecx
	orl		%edx,%ecx		/ Set cpunum
	orl		$INKERNEL_NOW+INKERNEL_LOCK,%ecx
	lock; cmpxchg	%ecx,inkernel
	/ Are we the kernel?
	jz		__ker_exit
	pause
	jmp		1b

#else
	LOCKOP
	orl		$INKERNEL_NOW+INKERNEL_LOCK,inkernel
	jmp		__ker_exit
#endif

#ifdef	VARIANT_smp
case2:
	mov		SMPREF(actives,%ebp,4),%ebx
	/
	/ Have to restore correct address space since intr_process_queue
	/ didn't (thought we were going to do a __ker_exit since we're coming
	/ from user space).
	/
	lea		SMPREF(aspaces_prp,%ebp,4),%ecx
	mov		ASPACE_PRP(%ebx),%eax
	cmp		(%ecx),%eax
	je		1f
	test	%eax,%eax
	je		1f

	/ Protect us from being ripped out on another interrupt
	LOCKOP
	incl	inkernel

	push	%ecx
	push	%eax
	call	*MEMMGR_ASPACE+memmgr
	add		$8,%esp

	cli
	LOCKOP
	decl	inkernel
1:

	testl	$INKERNEL_LOCK+INKERNEL_SPECRET,inkernel
	jnz		__ker_exit2
	movzbl	PRIORITY(%ebx),%eax
/ We might want to compare queued_event_priority with inkernel cpu priority
	cmpl	queued_event_priority,%eax
	jae		__ker_exit2

	// also check for need_to_run; if it's set we also want to send the IPI
	cmpl    $0,need_to_run
	je      __ker_exit2

	call	send_ipi2

/ We are out of here
	jmp		__ker_exit2


#ifdef SMP_MSGOPT
reacquire_kernel:
	sti
	push	%ecx
	push	%edx
1001:
	mov		inkernel,%eax
	test	$INKERNEL_NOW+INKERNEL_LOCK,%eax
	jz		1002f
	pause
	jmp		1001b
1002:
	mov		%ebp,%ecx
	mov		%eax,%edx
	andl	$0x00ffffff,%edx
	shl		$24,%ecx
	orl		%edx,%ecx		/ Set cpunum
	orl		$INKERNEL_NOW,%ecx
	lock
	 cmpxchg	%ecx,inkernel
	jnz		1001b
	pop		%edx
	pop		%ecx
	cli
	ret
reacquire_kernel_end:
#endif

#endif

from_kerorintr:

#ifdef	VARIANT_smp
#ifdef	SMP_MSGOPT

	/ Were we spinning waiting to re-acquire the kernel?
	movl	STK_EIP(%esp),%esi
	cmp		$reacquire_kernel,%esi
	jb		1f
	cmp		$reacquire_kernel_end,%esi
	jbe		kernel_continue

1:
	/ Check for MSG_XFER case, if SMP_RESCHED is set, preempt the msg xfer.
/ optimize these codes later
	mov		SMPREF(actives,%ebp,4),%ebx
	testb	$_NTO_ITF_MSG_DELIVERY,ITFLAGS(%ebx)
	jz		from_kerorintr2
	testl	$INKERNEL_INTRMASK,inkernel
	/ We are in interrupt, nested. no preemption.
	jnz		case5
	testl	$_NTO_ATF_SMP_RESCHED,ATFLAGS(%ebx)
	jz		from_kerorintr1
	testl	$INKERNEL_LOCK,inkernel
	jz		preempt
	/ kernel is locked
	mov		%ebp,%eax
	cmpb	cpunum,%al
	jz		case4	/ I lock the kernel, no preemption
	/ preempt the msg xfer
	jmp		preempt
from_kerorintr1:
	/ if someone else is in kernel, we could just go back
	testl	$INKERNEL_NOW,inkernel
	jz		from_kerorintr2
	mov		%ebp,%eax
	cmpb	cpunum,%al
	jnz		case4

from_kerorintr2:
#endif
	/ Figure out if someone is in the kernel
	mov		inkernel,%eax
	testl	$INKERNEL_LOCK+INKERNEL_INTRMASK,%eax

	/ Someone else (or us) is in the kernel, locked or handling an interrupt.
	/ Just unwind, the event will get drained.
	jnz		case4

	/ Were we spinning waiting for the kernel?
	movl	STK_EIP(%esp),%esi
	cmp		$acquire_kernel_attempt,%esi
	jb		1f
	cmp		$end_acquire_kernel_attempt,%esi
	jbe		case6

1:
	/ There is 1 case left; we were the kernel
	/get rid of this 'ifndef'
#ifndef	SMP_MSGOPT
	testl	$INKERNEL_NOW,%eax
jz	case3 / could be in idle or big msgxfer, use standard preemption check
	mov		%ebp,%ecx
	cmpb	cpunum,%cl
jne		__hardcrash
#endif

#else
	mov		inkernel,%eax
	testl	$INKERNEL_INTRMASK+INKERNEL_LOCK,%eax
	jnz		case4

#endif

case3:
	mov		SMPREF(actives,%ebp,4),%ebx
	movzbl	PRIORITY(%ebx),%eax
	cmpl	queued_event_priority,%eax
	jb		preempt
#ifdef	VARIANT_smp
/ IPI_RESCHED case, should check for idle case and to improve interrupt latency
	testl	$_NTO_ATF_SMP_RESCHED,ATFLAGS(%ebx)
	jnz		preempt
#endif

#if 0

	// if need_to_run is set we also want to preempt.
	// XXXdbg -- unfortunately if we do this the kernel will
	//           crash if you exit from photon with a lot of
	//           apps running.  I don't understand why.
	cmpl    $0,need_to_run
	jne     preempt

#endif

case4:
case5:
#ifdef VARIANT_smp
// Similar to before, if we get here and there's an interrupt
// event pending that we haven't handled then we fire off an
// ipi to another cpu to get them to handle it.
//
	cmpl    $0,intrevent_pending
	je      kernel_continue

    // if we're here it means there's an interrupt event pending
	// but we're not going to drain it.  so we tell another cpu
	// to deal with it so that the event gets drained in a timely
	// fashion.
    //
	// we pick another cpu by incrementing our cpu number and doing
    // a modulo on the number of cpu's present.
	//

// If interrupt counter is not zero, somebody is either in interrupt handling
// or temperorily increase the counter; he will either deliver the interrupt
// events or go to here or __ker_exit2 to do the check again
	mov		inkernel,%eax
	testl	$INKERNEL_INTRMASK,%eax
	jne		kernel_continue
// If somebody is in kernel but not set EXIT flag yet, he will go through
// ker_exit sequence and deliver the interrupt events.
	testl	$INKERNEL_NOW,%eax
	je		1f
	testl	$INKERNEL_EXIT,%eax
	je		kernel_continue

1:
    movl    %ebp,%eax
    incl    %eax                  // increment our cpu number
	cmpl	%eax,num_processors
	jne		1f
	xorl	%eax,%eax			  // wrap-around on num processors
1:
	movl    $IPI_CHECK_INTR,%edx   // ipi cmd goes in %edx
	call	send_ipi
#endif

kernel_continue:
	POPREG
	iret

case6:
	// Back up saved EIP and go to user case
	mov		SMPREF(actives,%ebp,4),%ebx
	subl	$KER_ENTRY_SIZE,REG_OFF+REG_EIP(%ebx)
	mov		SMPREF(ker_stack,%ebp,4),%esp
	jmp		case1

preempt:

#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
	/ test if we got the inkern flag
	testl	$INKERNEL_NOW+INKERNEL_LOCK,inkernel
	jz		pe0
	mov		%ebp,%eax
	cmpb	%al,cpunum
	je		pe1
pe0:
	/ we are in msg xfer, and need to get inkern flag back
	call	reacquire_kernel
pe1:
#endif
	mov		inkernel,%eax
	test	$INKERNEL_EXIT,%eax
	jnz		pe2
	sub		$KER_ENTRY_SIZE,REG_OFF+REG_EIP(%ebx)
pe2:
	LOCKOP
	orl		$INKERNEL_NOW+INKERNEL_LOCK,inkernel
	LOCKOP
	andl	$~INKERNEL_SPECRET,inkernel

#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
	/ might need to restore itflags if during msg xfer
	testb	$_NTO_ITF_MSG_DELIVERY,ITFLAGS(%ebx)
	jz		pe3
	push	%ebx
	call	xfer_restorestate
	add		$4,%esp
pe3:
#endif

	GETXFERHANDLER	%eax
	or		%eax,%eax
	jz		__ker_exit			/ No, abort entire call
	SETXFERHANDLER	0,%edx,0
	mov		4(%eax),%eax			/ Get address of restart code
	or		%eax,%eax
	jz		__ker_exit			/ No handler so abort entire call
	mov		%esp,%ecx
	push	%ecx
	push	%ebx
	call	*%eax
	add		$8,%esp
	jmp		__ker_exit


/ EXCEPTIONS
/;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/
/ exc8, exc9, exca, excb, excf, exc12 --> __hardcrash
/
/ exc0, exc4, exc5 --------SIGFPE---+---> usr_fault
/                                   |
/ exc1, exc3 -------------SIGTRAP---+
/                                   |
/ exc6 --------------------SIGILL---+
/                                   |
/ excc, excd -------------SIGSEGV---+
/                                   |
/ exc11 -------------------SIGBUS---+
/
/ exc7 --> device not available (threads first fpu opcode with a real fpu)
/ exc7emul -->                  (no fpu, emulator trap, conforming code seg)
/
#define	_CMPXCHG_BX		0x1ab10ff0		// lock cmpxchg ebx,[edx]
#define	_CMPXCHG_CX		0x0ab10ff0		// lock cmpxchg ecx,[edx]
#define	_RDTSC			0x310f
#define	_SYSENTER		0x340f


/
/ Hard faults which should not be possible.
/
__exc8:					/ Double Fault
	pop	%ss:kdbg_fault_code
	PUSHREG
	mov	$SIGSEGV+(SEGV_GPERR*256)+(FLTDBLFLT*65536)+SIGCODE_FATAL,%eax
	jmp	__hardcrash

__exc9:					/ Reserved
	PUSHREG
	mov	$SIGSEGV+(SEGV_GPERR*256)+(FLTOLDFPE*65536)+SIGCODE_FATAL,%eax
	jmp	__hardcrash

__exca:					/ Invalid TSS
	pop	%ss:kdbg_fault_code
	PUSHREG
	mov	$SIGSEGV+(SEGV_GPERR*256)+(FLTTSS*65536)+SIGCODE_FATAL,%eax
	jmp	__hardcrash

__excb:					/ Segment Not Present
	pop	%ss:kdbg_fault_code
	PUSHREG
	mov	$SIGSEGV+(SEGV_GPERR*256)+(FLTSEG*65536)+SIGCODE_FATAL,%eax
	jmp	__hardcrash

__excf:					/ Unknown
	PUSHREG
	mov	$SIGSEGV+(SEGV_GPERR*256)+(FLTRSVD*65536)+SIGCODE_FATAL,%eax
	jmp	__hardcrash

__exc12:				/ Machine Check
	PUSHREG
	mov	$SIGSEGV+(SEGV_GPERR*256)+(FLTMACHCHK*65536)+SIGCODE_FATAL,%eax
	jmp	__hardcrash

/*
/ We assume threads don't use the FPU/MMX registers and set the
/ TS bit in the msw to catch them if they do. When they try to use it,
/ we'll fault end end up here. If this is the first time, we'll allocate
/ a new FPU context. Subsequently, we'll use this to save and restore
/ the FPU context on-demand.
/ Things are a little more complicated on SMP, as this interacts with
/ the IPI to fetch the context from another CPU...
*/
__exc7:					/ Device Not Available (real fpu but TS bit was set)
	PUSHREG
	GETCPU	%bp, %ebp
	mov		SMPREF(actives,%ebp,4),%ebx

#ifdef VARIANT_instr
	// Save stack pointer
	mov		%esp, %edx
	mov		SMPREF(ker_stack,%ebp,4), %esp
	SAVE_PERFREGS 0
	// Return to original stack
	mov		%edx, %esp
#endif

	mov		FPUDATA(%ebx),%edx
	or		%edx,%edx
	jz		no_save_area

#ifdef VARIANT_smp
	clts
	mov		SMPREF(actives_fpu,%ebp,4),%ecx
	cmp		%ecx,%ebx
	je		fpuregs_ok
	or		%ecx,%ecx
	jz		no_save_needed

	/ FPU has a context, save it first
	mov		FPUDATA(%ecx),%eax
	andl	$FPUDATA_MASK,%eax
	testl	$X86_CPU_FXSR,__cpu_flags
	jz		1f
	FXSAVE_EAX
	jmp		2f
1:
	fnsave	(%eax)
2:
	mov		%eax,FPUDATA(%ecx)

	/ Now we need to load the current context
no_save_needed:
	testl	$FPUDATA_BUSY,%edx
	jnz		fetch_needed

	mov		%ebx,SMPREF(actives_fpu,%ebp,4)
	testl	$X86_CPU_FXSR,__cpu_flags
	jz		1f
	FXRSTOR_EDX
	jmp		2f
1:
	fnclex
	frstor	(%edx)
2:
	orl		$FPUDATA_BUSY,%edx
	or		%ebp,%edx
	mov		%edx,FPUDATA(%ebx)
	jmp		fpuregs_ok

fetch_needed:
	/ Context is busy on another CPU. Need to IPI to fetch it out.
	xor		%eax,%eax
	mov		%eax,SMPREF(actives_fpu,%ebp,4)
	andl	$FPUDATA_CPUMASK,%edx
	jmp		fetch_fpu_context

fpu_context_not_busy:
	mov		FPUDATA(%ebx),%edx

	mov		%ebx,SMPREF(actives_fpu,%ebp,4)
	testl	$X86_CPU_FXSR,__cpu_flags
	jz		1f
	FXRSTOR_EDX
	jmp		2f
1:
	fnclex
	frstor	(%edx)
2:
	orl		$FPUDATA_BUSY,%edx
	or		%ebp,%edx
	mov		%edx,FPUDATA(%ebx)
#ifndef NDEBUG
	/ make sure we ARE on the user stack, since we are assuming that below
	testl	$0x03,STK_CS(%esp)
	jz		__hardcrash
#endif
	/ __ker_exit2 may actually end up doing an IPI_CHECK_INTR so we
	/ need to switch to the kernel stack before jumping to __ker_exit2!
	mov		SMPREF(ker_stack,%ebp,4),%esp
	/ Interrupts were enabled during the fetch, so we may need to restore TLS
	jmp		__ker_exit2

#else
    clts
	mov		SMPREF(actives_fpu,%ebp,4),%ecx
	cmp		%ecx,%ebx
	je		fpuregs_ok
	or		%ecx,%ecx
	jz		1f
	mov		FPUDATA(%ecx),%ecx
	testl	$X86_CPU_FXSR,__cpu_flags
	jz		2f
	FXSAVE_ECX
	jmp		3f
2:
	fnsave	(%ecx)
3:

1:
	mov		%ebx,SMPREF(actives_fpu,%ebp,4)
	testl	$X86_CPU_FXSR,__cpu_flags
	jz		1f
	FXRSTOR_EDX
	jmp		2f
1:
	fnclex
	frstor	(%edx)
2:
#endif

fpuregs_ok:
	POPREG
	iret

no_save_area:
#ifdef	VARIANT_smp
	mov	$_NTO_ATF_FPUSAVE_ALLOC,%eax
	jmp	force_kernel
#else
	mov	SMPREF(ker_stack,%ebp,4),%esp
	orl	$INKERNEL_NOW+INKERNEL_LOCK,inkernel
	orl	$_NTO_ATF_FPUSAVE_ALLOC,ATFLAGS(%ebx)
	jmp	__ker_exit
#endif

/*
/ We don't have a real FPU so we point the fault vector at some code which
/ transfers control to an emulator. Each threads process local storage
/ contains a pointer to the emulator. This is a TRAP_GATE to a conforming CS.
/ Note that this code should be position-independent, as it may get remapped
/ to another vaddr (when using large pages)
*/
__exc7emul:				/ Device Not Available (emulator trap)
	sub	$8,%esp				/ Make space for ss,esp (Not there because it was no privity switch)
	pusha
	lea	REG_EFL(%esp),%esi
	lea	REG_EIP(%esp),%edi
	movsl					/ Move eip,cs,efl
	movsl
	movsl
	lea	SIZEOF_REG(%esp),%eax	/ Original esp
	mov	%eax,REG_ESP(%esp)	/ save original esp
	sub	%eax,%eax
	movw	%ss,%ax
	mov	%eax,REG_SS(%esp)		/ and original ss
	push	%esp				/ Arg2 - Pointer to integer registers
	mov	CPUPAGE_SEG:CPUPAGE_TLS,%eax
	add	$TLS_FPUEMU_DATA,%eax	/ Address of a pointer to fpu registers
	push	%eax				/ Arg1
	mov	$SIGFPE+(FPE_NOFPU*256)+(FLTNOFPU*65536),%edx
	push	%edx
	mov	CPUPAGE_SEG:CPUPAGE_PLS,%eax
	or	%eax,%eax
	je	noemul
	mov	4(%eax),%eax	/ Entry point of emulator
	or	%eax,%eax
	je	noemul
	call	*%eax		/ Call the emulator
	test	%eax,%eax
	jnz	emudie		/ eax has the signal to drop on thread
	add	$12,%esp
	popa
	push	8(%esp)		/ push flags
	popfl
	ret	$16			/ ignore cs,flags,esp,ss
noemul:
	mov	%edx,%eax
emudie:
	test	$0x0ffff0000,%eax
	jnz	hasflt
	mov	(%esp),%edx
	and	$0x0ffff0000,%edx
	or	%edx,%eax
hasflt:	mov	%eax,4(%esp)
	mov	__KER_SIGNAL_FAULT,%eax		/ __KER_SIGNAL_FAULT
	int	$0x28			/ will not return...
__exc7emul_end:

/
/ There are two methods in which an FPU error is triggered. We support one.
/
/ 1. Via hardware interrupt 13 decimal on a PC if NE bit is clear in CR0.
/    (not supported)
/ 2. Via fault vector 10h if the NE bit is set in CR0.
/
/ We always set the NE bit so in theory, we should always come here!
/
__exc10:				/ Floating Point Error
	PUSHREG
	smsww	%ax
	and	$~8,%eax			/ Clr the TS (task switched) flag
	lmsww	%ax
	xor	%eax,%eax
	fnstsw	%ax				/ Save the status
	fnclex
	xor	%edx,%edx
	bsf	%eax,%edx				/ Get first bit index set
	xor	%eax,%eax
	movb	fpuerr2code(%edx),%ah/ Map it to a POSIX fault code
	or	$SIGFPE+(FLTFPE*65536),%eax
	jmp	exc

/
/ Debugger exceptions. If the _NTO_PF_DEBUGING process flag
/ is not set, they will try to pass debugging on to the low level
/ debugger. If they detect a debug point the usr_fault function
/ will stop all threads and/or enqueue a signal on the process.
/
__exc1:					/ Debug Exception (Single Step or Watchpoint)
	PUSHREG
	mov	$SIGTRAP+(TRAP_TRACE*256)+(FLTTRACE*65536),%eax
	jmp	debug_exc

__exc3:					/ Breakpoint
	PUSHREG
	sub	$1,REG_EIP(%esp)	/ Move eip back to brkpoint
	mov	$SIGTRAP+(TRAP_BRKPT*256)+(FLTBPT*65536),%eax
	jmp	debug_exc

/
/ Arithmetic and opcode faults
/
__exc0:					/ Divide Error
	PUSHREG
	mov	$SIGFPE+(FPE_INTDIV*256)+(FLTIZDIV*65536),%eax
	jmp	exc

__exc4:					/ Overflow
	PUSHREG
	mov	$SIGFPE+(FPE_INTOVF*256)+(FLTIOVF*65536),%eax
	jmp	exc

__exc5:					/ Bounds Check
	PUSHREG
	mov	$SIGFPE+(FPE_INTOVF*256)+(FLTBOUNDS*65536),%eax
	jmp	exc

__exc6:					/ Invalid Opcode
	push	%eax
	mov		4(%esp),%eax		/ Point at faulting opcode
	mov		(%eax),%eax			/ Get faulting opcode
	cmpw	$_SYSENTER,%ax		/ Was it a sysenter instruction?
	jne		not_sysenter		/ Nope
	pop		%eax
	addl	$2,0(%esp)			/ Step over instruction (2 byte opcode)
	jmp		__ker_entry

not_sysenter:
	cmp		$_CMPXCHG_BX,%eax	/ Was it a lock cmpxchg instruction?
	jne		not_cmpxchg_bx		/ Nope
	pop		%eax
/*
 We use "cmpxchg 0[edx],ebx" for mutex's.
 It does not exist on a 386 so we emulate it.
*/
	addl	$4,0(%esp)			/ Step over instruction (3 byte opcode + lock)
	cmp		0(%edx),%eax
	jne		mutex_locked
	mov		%ebx,0(%edx)		/ Move src into dest
	orl		$0x40,8(%esp)		/ turn on Z flag
	iret

not_cmpxchg_bx:
	cmp		$_CMPXCHG_CX,%eax	/ Was it a lock cmpxchg instruction?
	jne		not_cmpxchg_cx		/ Nope
	pop		%eax
/*
 We use "cmpxchg 0[edx],ecx" for mutex's.
 It does not exist on a 386 so we emulate it
*/
	addl	$4,0(%esp)			/ Step over instruction (3 byte opcode + lock)
	cmp		0(%edx),%eax
	jne		mutex_locked
	mov		%ecx,0(%edx)		/ Move src into dest
	orl		$0x40,8(%esp)		/ turn on Z flag
	iret

mutex_locked:
	mov		0(%edx),%eax		/ Move dest into eax
	andl	$~0x40,8(%esp)		/ turn off Z flag
	iret

not_cmpxchg_cx:
	cmpw	$_RDTSC,%ax			/ Was it a rdtsc instruction?
	jne		not_rdtsc			/ Nope
	//
	// This is a bit complicated. We need to run the RDTSC emulation
	// code with interrupts enabled so that we can detect when an
	// interrupt goes off during the emulation - otherwise the cycles
	// variable and the return value from the callout_timer_value
	// function will be out of whack. In order to enable interrupts,
	// we can either A) completely enter the kernel, or B) not run
	// the code with CPL==0 (so system/user checks in interrupt handling
	// works out OK). Doing A) is tough in the SMP case, since some
	// other CPU could be in the kernel, and we don't want to have to
	// wait for them to get out. Therefore, we save the address of
	// the RDTSC opcode in %edx:%eax and then iret to the rdtsc_emu label,
	// _but using the same %cs value as a a procnto thread_. This
	// means that we're running as ring1, so nobody thinks we're
	// in the kernel, but we still have IOPL privledges and all the page
	// table permission checks are still supervisor level. The
	// upshot is that we can then re-enable interrupts and get the
	// proper result for the instruction. The downside is that the
	// code isn't currently smart enough to handle emulation of RDTSC
	// from a procnto thread (big whoops :-). If it ever becomes required
	// to allow RDTSC emulation from a procnto thread, some extra checks
	// for the original ring level have to be added to avoid the first two
	// pushes in the rdtsc_emu code.
	//
	pop		%eax
	addl	$2,(%esp)			/ Step over instruction (2 byte opcode)

	testl	$0x3,0x4(%esp)		/ were we already at ring0?
	jz		rdtsc_emu_ring0		/ if so, just go ahead and emulate...

	movzwl	sys_ss,%eax			/ cram new stack selector
	mov		%eax,16(%esp)		/ ...
	pop		%eax				/ save original CS:EIP
	pop		%edx				/ ....
	pushw   $0					/ point at emulation code
	pushw	sys_cs				/ ...
	push	$rdtsc_emu			/ ...
	iret						/ and do it

rdtsc_emu:
	pushl	%ds					/ build new iretd frame
	push	%esp
	addl	$4,(%esp)			/ Adjust for the push of DS
	pushfl
	push	%edx
	push	%eax
rdtsc_emu_ring0:
	sti							/ interrupts are OK again
	push	%ebx
	push	%ecx
1:
	mov		cycles,%ebx
	mov		cycles+4,%ecx
	mov		qtimeptr,%edx		/ get the timer count value
	mov		_syspage_ptr,%eax
	call	*callout_timer_value
	cmp		%ebx,cycles			/ if cycles has changed, have to try again
	jne		1b
	cmp		%ecx,cycles+4
	jne		1b
	xor		%edx,%edx			/ add it to the current number of cycles
	add		%ebx,%eax
	adc		%ecx,%edx
	pop		%ecx
	pop		%ebx
	iret

not_rdtsc:
	pop		%eax
	PUSHREG
	mov		$SIGILL+(ILL_ILLOPC*256)+(FLTILL*65536),%eax
	jmp		exc


/
/ Segment faults
/
__excc:					/ Stack Exception
	pop		%ss:kdbg_fault_code
	PUSHREG
	mov		$SIGSEGV+(SEGV_STKERR*256)+(FLTSTACK*65536),%eax
	jmp		exc

__excd:					/ General Protection
	pop		%ss:kdbg_fault_code
	testl	$X86_PSW_VM,8(%esp)	/ Check for V86
	jnz		__v86

#ifndef	__SEGMENTS__
	push	%eax
	push	%ebp
	GETCPU	%bp, %ebp
	mov		%ds,%ax
	cmp		%ax,%ss:usr_ds
	jne		fixsegs
	mov		%es,%ax
	cmp		%ax,%ss:usr_ds
	jne		fixsegs
	mov		CPUPAGE_SEG,%ax
	cmp		SMPREF(%ss:cpupage_segs,%ebp,2),%ax
	jne		fixsegs
	cmpl	$__keriret_iret_instr,0x8(%esp)
	je		fixsegs2
not_fixsegs:
	pop		%ebp
	pop		%eax
#endif
	PUSHREG
	mov		$SIGSEGV+(SEGV_GPERR*256)+(FLTPRIV*65536),%eax
	jmp		exc

#ifndef	__SEGMENTS__
fixsegs:
	mov		%ss:usr_ds,%ds
	mov		%ss:usr_ds,%es
	mov 	SMPREF(%ss:cpupage_segs,%ebp,2),CPUPAGE_SEG
	pop		%ebp
	pop		%eax
	iret

	// Deal with a chip bug on the AMD SC400. It can let a bad value
	// into the CS register that we then trip on when attempting to
	// "iret" back into user code. See if the saved CS value from the
	// register context is incorrect and correct it if need be.
fixsegs2:
	movzwl	%ss:usr_cs,%eax
	cmp		0x18(%esp),%eax
	je		not_fixsegs
	mov		%eax,0x18(%esp)
	pop		%ebp
	pop		%eax
	iret
#endif

__exc11:					/ Alignment Check
	pop	%ss:kdbg_fault_code
	PUSHREG
	mov	$SIGBUS+(BUS_ADRALN*256)+(FLTACCESS*65536),%eax
	jmp	exc

/
/ Page fault
/ We need to extract two pieces of information on a page fault
/  - the address of the data reference (CR2)
/  - did the fault happened on a read or a write (ss:fault_code)
/ A page fault may be recoverable with an external pager.
/
__exce:					/ Page Fault
	SPINLOCK	exce_slock,%edx,1	/ preserves edx
	pop		%ss:fault_code
	PUSHREG
	mov		fault_code,%esi	/ Grab fault code while interrupts are disabled
	mov		%cr2,%edi			/ Get the target address
	SPINUNLOCK	exce_slock
	mov		$PAGE_FAULT_CODE,%eax
	jmp		exc

__fault_error:
	stc
	mov		$-1,%eax
__fault_noerror:
	iret


debug_exc:
	testl	$0x03,STK_CS(%esp)
	jnz		exc

    //
	// If we're here it means that we get a debug exception in kernel
	// mode.  First we check if it was a trace exception which would
	// happen if someone tried to single step with a debugger into a
	// kernel call.
    //
	testl   $X86_PSW_TF,STK_EFL(%esp)
	jz      not_sstep

    // if the single-step fault did not happen on the __ker_sysenter
	// then it is probably a kernel debugger, so skip over this
    cmpl    $__ker_sysenter,STK_EIP(%esp)
	jne     not_sstep

    // Ok, someone tried to single-step into the kernel.
	//
    // First we blow away the exception state since we don't care about
	// it.  Down below we fake things up to look like we came from a
	// sysenter call.
	//
    popa

	//dump the eip, cs and eflags, but do _NOT_ clobber the eax register
	//as it holds our syscall number, which we'll need when we jmp
	//to fixup_tf_entry
	add $12, %esp // dump the eip, cs, eflags and do _NOT_ clobber eax

    //
	// This is a duplicate of the code at the beginning of __ker_sysenter.
	// We have to do this to make sure that the saved copy of eflags has
	// the TF bit set and that it is turned off while we're executing in
	// kernel mode.
	//
	push	%ebx
	push	%ebp
	GETCPU	%bp, %ebp
	mov		SMPREF(actives,%ebp,4),%ebx

	movl	%edi,REG_OFF+REG_EDI(%ebx)

    // Get the copy of eflags
	pushf
	popl	%edi

    // Turn of T-bit so we don't get trace exceptions while
    // in the kernel.
	// ?? Shouldn't it already be off, or we'd be getting more
	// ?? single step exceptions? I don't think this code is
	// ?? needed. bstecher
    andl    $~X86_PSW_TF,%edi
	push    %edi
	popf

	// Make sure the the saved copy of the flags has the T bit on.
	orl     $X86_PSW_TF,%edi

    // Now that everything is patched up, lets jump to the right place
    // inside of __ker_sysenter and continue on like nothing happened.
    jmp     fixup_tf_entry

not_sstep:

    // If we're here it means we got a genuine exception in kernel mode.
	// Yikes!  Indicate this in the sigcode (in eax) and then call
	// kdebug_callout to handle it.  If kdebug_callout were to return 0
	// we'll just skip out and continue execution.
	//
	mov		%esp,%edx
	or		$SIGCODE_KERNEL,%eax
	call	kdebug_callout
	test	%eax,%eax
	jz		fault_handled

/
/ Common exception handler. At this point we have done a PUSHREG and
/ eax contains the fault code. We classify the fault as user or system
/ based upon the privity level of the CS where we came from.
/ (for page faults, esi & edi also contain useful information)
/
exc:
	GETCPU	%bp, %ebp
	testl	$X86_PSW_VM,STK_EFL(%esp)	/ Check for V86
	jnz		usr_exc
	testl	$0x03,STK_CS(%esp)
	jz		sys_exc

usr_exc:
#ifdef	VARIANT_smp
	/ See if we can become the kernel
	mov		%eax,%ebx
1:
	mov		inkernel,%edx
	test	$INKERNEL_NOW+INKERNEL_LOCK,%edx
	jnz		usr_exc_smp
	mov		%edx,%eax
	mov 	%ebp,%ecx
	andl	$0x00ffffff,%edx
	shl		$24,%ecx
	orl		%edx,%ecx		/ Set cpunum
	orl		$INKERNEL_NOW+INKERNEL_LOCK,%ecx
	lock; cmpxchg	%ecx,inkernel
	/ Restore EAX in case we go to usr_exc_smp
	mov		%ebx,%eax
	jz		usr_exc1
	pause
	jmp		1b
usr_exc1:
	/ We are the kernel!
#else
	LOCKOP
	orl		$INKERNEL_NOW+INKERNEL_LOCK,inkernel
#endif
	mov		SMPREF(ker_stack,%ebp,4),%esp
	mov		SMPREF(actives,%ebp,4),%ebx

#ifdef VARIANT_instr
	SAVE_PERFREGS 0
#endif

	sti
	cld
	cmp		$PAGE_FAULT_CODE,%eax
	je		usr_page_fault
	mov		%ebx,%edx
	mov		REG_OFF+REG_EIP(%edx),%ecx	/ faulting address
	mov		%ecx,%ebx	/	watcom wants ebx, gcc wants ecx
	push	$__ker_exit
	jmp		usr_fault

/
/ An exception occurred while executing system code or an interrupt handler.
/ We will be using the kernel stack.
/
sys_exc:
//CHKSTKOVER %ecx,0
	mov		inkernel,%ecx
	mov		SMPREF(actives,%ebp,4),%ebx
	test	$INKERNEL_INTRMASK,%ecx
	jnz		fixup_intr
sys1:
	test	$INKERNEL_NOW,%ecx
#ifdef	VARIANT_smp
	jz		sys3
/ am I in kernel?
	mov		%ecx,%edx
	shr		$24,%edx
	cmp		%ebp,%edx
	jne		sys3
sys5:
#else
	jz		__hardcrash
#endif
	test	$INKERNEL_LOCK,%ecx
	jnz		__hardcrash
	test	$INKERNEL_SPECRET,%ecx
	jnz		fixup_specret
	cmp		$PAGE_FAULT_CODE,%eax
	je		sys_page_fault
	mov		$0,%esi			/ Pretend it was a page fault on read
sys2:
	GETXFERHANDLER	%edx
	cmpl	$0,%edx
	jne		fixup_xfer
	or		$SIGCODE_USER,%eax
	mov		%esp,%edx
	call	kdebug_callout
	test	%eax,%eax
	jz		fault_handled

fixup_kcall:
	movl	$0,TIMEOUT_FLAGS(%ebx)	/ Remove any timeout flags
	movl	$ERRNO_EFAULT,REG_OFF+REG_EAX(%ebx)
	movl	TFLAGS(%ebx),%edx	/ Get flags
	orl		$_NTO_TF_KERERR_SET,TFLAGS(%ebx)/ Set error indicator
	test	$_NTO_TF_KERERR_SET,%edx	/ If indicator set we have already bumped eip
	jnz		__ker_exit
  	incl	REG_OFF+REG_EIP(%ebx)		/ Set over kcall ret
	jmp		__ker_exit

#ifdef	VARIANT_smp
sys3:
#ifdef	SMP_MSGOPT
	push	%eax
	call	reacquire_kernel
	pop		%eax
	mov		inkernel,%ecx
	jmp		sys5
#else
	jmp __hardcrash
#endif
#endif


/
/ Knock down the special return flag which is active and return from kcall
/
fixup_specret:
	cmp		$PAGE_FAULT_CODE,%eax
	je		specret_page_fault
	cmpb	$SIGTRAP,%al
	jne		spec1
	or		$SIGCODE_KERNEL,%eax
	jmp		spec2
spec1:
	or		$SIGCODE_USER,%eax
spec2:
	mov		%esp,%edx
	call	kdebug_callout
	test	%eax,%eax
	jz		fault_handled
	LOCKOP
	orl		$INKERNEL_LOCK,inkernel
	LOCKOP
	andl	$~INKERNEL_SPECRET,inkernel
fixup_specret2:
	mov		inspecret,%edx
	not		%edx
	and		%edx,TFLAGS(%ebx)
	GETXFERHANDLER	%edx
	cmpl	$0,%edx
	je		fixup_kcall

/
/ The fixup code is in nano_xfer
/ esi must be the fault code
/
fixup_xfer:
	GETXFERHANDLER	%eax
	SETXFERHANDLER	0,%edx,0
	mov		%esp,%edx
	push	%esi
	push	%edx
	push	%ebx
	call	*0(%eax)				/ ebx (active) is preserved
	add		$12,%esp
	jmp		fixup_kcall

/
/
/
fixup_intr:

#ifdef	VARIANT_smp
	/ Were we really in an interrupt? Do not touch %ecx since it is used by sys1
	mov		SMPREF(cpupageptr,%ebp,4),%edx
	cmpl	$0,CPUPAGE_STATE(%edx)
	jz		sys1
#endif

	or		$SIGCODE_INTR,%eax
	mov		%esp,%edx
	call	kdebug_callout
	test	%eax,%eax
	jz		fault_handled
		/ should clear SIG_BLOCKED and SIG_IGNORE
	movb	%al,intr_fault_event+SIGEV_SIGNO
	mov		REG_EIP(%esp),%eax			/ Get fault EIP
	mov		intrespsave,%esp				/ Restore esp
	mov		8(%esp),%ebx					/ Restore ebx
	mov		INTR_THREAD(%ebx),%edx		/ Find intr thread
	mov		%eax,REG_OFF+REG_EIP(%edx)	/ Change thread EIP to match
	mov		$intr_fault_event,%eax
	jmp		intrret

fault_handled:
	POPREG
	iret

		/ on entry to all the page fault handlers
		/ EAX - signal code
		/ ESI - x86 fault code
		/ EDI - faulting address
		/ EBX - holds active

usr_page_fault:
	call	handle_page_fault
	jz		__ker_exit
	mov		%ebx,%edx
	mov		%edi,%ecx
	mov		%ecx,%ebx	/	watcom wants ebx, gcc wants ecx
	push	$__ker_exit
	jmp		usr_fault

specret_page_fault:
	LOCKOP
	orl		$INKERNEL_LOCK,inkernel
	LOCKOP
	andl	$~INKERNEL_SPECRET,inkernel
	orl		$0xc0000000,%esi // temp hack to tell shim we're in specret
	call	handle_page_fault
	jnz		fixup_specret2
	GETXFERHANDLER	%eax
	or		%eax,%eax
	jz		__ker_exit			/ No xfer_handler
	SETXFERHANDLER	0,%edx,0
	mov		4(%eax),%eax		/ Get address of restart code
	or		%eax,%eax
	jz		__ker_exit			/ No preempt handler, just exit
	mov		%esp,%ecx
	push	%ecx
	push	%ebx
	call	*%eax
	add		$8,%esp
	jmp		__ker_exit

sys_page_fault:
	LOCKOP
	orl		$INKERNEL_LOCK,inkernel
	orl		$0x80000000,%esi // temp hack to tell shim we're in system
	call	handle_page_fault
	jnz		sys2
	jmp		preempt		/ From kernel the call must be restarted (ebx contains prempted thread)

handle_page_fault:
	sti
	mov		SMPREF(actives,%ebp,4),%ebx
	mov		SMPREF(aspaces_prp,%ebp,4),%eax
	push	%esi
	push	%edi
	push	%eax		/ Call fault handler (previous active is preserved in ebx)
	call	vmm_fault_shim // fault handler may call block changing active
	add		$3*4,%esp
	test	%eax,%eax
	ret

__intr_unexpected:		/ What the hey?
	PUSHREG
	mov		$SIGKILL,%eax
	/;
	/; Fall through to __hardcrash
	/;

/
/ For diagnostic purposes eax = signal | (si_code*256) | (fault_num*65536)
/
__hardcrash:
	or		$SIGCODE_FATAL,%eax
	mov		%esp,%edx
	mov		%esp,%ebx
	testl	$3,STK_CS(%edx)
	jz		1f
	GETCPU	%cx, %ecx
	mov		SMPREF(ker_stack,%ecx,4),%esp
1:
	call	kdebug_callout
	mov		%ebx,%esp
	test	%eax,%eax
	jz		fault_handled
	lup:
	hlt
	jmp	lup

twiddle:
	shl		$1,%eax
	add		realmode_addr,%eax
	incb	0x0b8000(%eax)
	ret

halt:
	cmpb	$0,nmi_pending
	jne		1f

#ifdef	VARIANT_smp
/ check atflag, if resched flag is set, no halt
	GETCPU	%cx,%ecx
	mov		SMPREF(actives,%ecx,4),%eax
	testl	$_NTO_ATF_SMP_RESCHED,ATFLAGS(%eax)
	jnz		2f

/ give up INKERNEL_NOW flag
	andb	$~(INKERNEL_NOW>>8),(inkernel+1)
#endif

hlt

#ifdef	VARIANT_smp
/ get back inkernel flag
4:
	mov		inkernel,%eax
	test	$INKERNEL_NOW+INKERNEL_LOCK,%eax
	jz		45f
	pause
	jmp		4b
45:
	mov		%eax,%edx
	andl	$0x00ffffff,%edx
	shl		$24,%ecx
	orl		%edx,%ecx		/ Set cpunum
	orl		$INKERNEL_NOW,%ecx
	lock; cmpxchg	%ecx,inkernel
	jz		2f
	GETCPU	%cx,%ecx
	pause
	jmp		4b
2:
#endif
1:
	ret

cpu_force_fpu_save:
	clts
	mov		4(%esp),%eax
	lea		FPUDATA(%eax),%eax
	mov		(%eax),%ecx
	andl	$FPUDATA_MASK,%ecx
	testl	$X86_CPU_FXSR,__cpu_flags
	jz		1f
	FXSAVE_ECX
	jmp		2f
1:
	fnsave	(%ecx)
2:
	mov		%ecx,(%eax)
	ret

#ifdef	VARIANT_smp

/	extrn	ipicmds

/*
/ The kernel is busy on another processor so we make this thread
/ do a nop kernel call. When it manages to get into the kernel, we'll
/ process the exception.
*/

usr_exc_smp:
	mov		SMPREF(actives,%ebp,4),%ebx

	mov		%eax,ARGS_ASYNC_CODE(%ebx)
	mov		%esi,ARGS_ASYNC_FAULT_TYPE(%ebx)
	mov		%edi,ARGS_ASYNC_FAULT_ADDR(%ebx)
	mov		$_NTO_ATF_SMP_EXCEPTION,%eax

force_kernel:
	or		$_NTO_ATF_FORCED_KERNEL,%eax
	mov		%eax,%ecx
	mov		ATFLAGS(%ebx),%eax
1:
	mov		%ecx,%edx
	or		%eax,%edx					/ turn on new bit(s)
	lock; cmpxchg	%edx,ATFLAGS(%ebx)	/ put new bits back atomically
	jnz		1b
	test	$_NTO_ATF_FORCED_KERNEL,%eax
	jnz		already_forced
	mov		REG_OFF+REG_EIP(%ebx),%edx
	mov		kercallptr,%eax
	test	$_NTO_ATF_WAIT_FOR_KER,%ecx
	jz		2f
	/ Spining for kernel, have to act like we were in userland
	subl	$KER_ENTRY_SIZE,%edx
	addl	$KER_ENTRY_SIZE,%eax
	//NOTE: If the register used to save the kernel call number in
	//'acquire_kernel_attempt' is changed, this code needs to change as well.
	movl	$__KER_NOP,STK_ESI(%esp)
2:
	mov		REG_OFF+REG_EAX(%ebx),%ecx
	mov		%ecx,ARGS_ASYNC_TYPE(%ebx)
	mov		%edx,ARGS_ASYNC_IP(%ebx)
	movl	$__KER_NOP,REG_OFF+REG_EAX(%ebx)
	mov		%eax,REG_OFF+REG_EIP(%ebx)
already_forced:
	POPREG
	iret

	/ Another CPU owns the FPU context that were trying to load. Send
	/ it an IPI and spin while it is saving it
	/
	/ EDX has the cpu that we need to IPI
	/ EBX has the thp for which we are trying to get the context
	/
	/ Note that at this point, we still have not entered the kernel
fetch_fpu_context:
	mov		%esp,%edi		/ save original ESP value
	testl	$0x03,STK_CS(%esp)
	jz		1f
	mov		SMPREF(ker_stack,%ebp,4),%esp	/ get some stack space for call
1:
	GETCPU	%cx,%ecx
	cmp	%edx,%ecx
	jne	1f
	jmp __hardcrash
1:
	movl	%edx,%eax
	movl	$IPI_CONTEXT_SAVE,%edx
	call	send_ipi

	GETCPU	%cx,%ecx
	LOCKOP
	incl	inkernel

	/ Now spin waiting for context to be freed
	sti
1:
	mov		FPUDATA(%ebx),%edx
	testl	$FPUDATA_BUSY,%edx
	jnz		1b

	cli
	LOCKOP
	decl	inkernel
	mov		%edi,%esp		/ restore original ESP value
	jmp		fpu_context_not_busy


/ IPI INTERRUPTS
/;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
/
/ SMP IPI handler and related support routines.
/


/
/ SMP inter-processor interrupt handlers
/
	.globl	__ipi1
	.globl	__ipi2

#define	IPI_OP	0

__ipi1:
	PUSHREG
	GETCPU	%bp, %ebp
//		mov eax,realmode_addr
//		inc byte ptr 0b8000h[ebp*2][eax]

	/ assuming the IPI_NUM == 4
	xor		%esi,%esi
	xchg	SMPREF(ipicmds,%ebp,4),%esi

	mov		lapicid_addr,%edx
	movl	$0,0x0b0-0x20(%edx)
	test	$IPI_PARKIT,%esi
	jz		ipi_no_parkit

	// Freeze the system, we have a kernel dump happening
	orb		$0x2,alives(%ebp)	// Indicate the CPU has parked
2:	jmp		2b

ipi_no_parkit:
	test	$IPI_CONTEXT_SAVE,%esi
	jz		ipi_no_context_save

	movl	SMPREF(actives_fpu,%ebp,4),%eax
	or		%eax,%eax
	jz		ipi_no_context_save

	smsww	%cx
	test	$8,%ecx		/ Is TS (task switched) flag already set?
	jz		1f
	clts
1:
	movl	FPUDATA(%eax),%edx
	andl	$FPUDATA_MASK,%edx
	testl	$X86_CPU_FXSR,__cpu_flags
	jz		1f
	FXSAVE_EDX
	jmp		2f
1:
	fnsave	(%edx)
2:
	orl		$8,%ecx		/ Set the TS (task switched) flag
	lmsww	%cx
	movl	%edx,FPUDATA(%eax)
	xor		%eax,%eax
	movl	%eax,SMPREF(actives_fpu,%ebp,4)

ipi_no_context_save:
	test	$IPI_TLB_SAFE,%esi
	jz		ipi_no_tlb_safe
	mov		%esp,%edi		/ save original ESP value
	testl	$X86_PSW_VM,STK_EFL(%esp)	/ Check for V86
	jnz		10f
	testl	$0x03,STK_CS(%esp)
	jnz		10f

	mov		SMPREF(actives,%ebp,4),%eax
#if defined (SMP_MSGOPT)
	testb	$_NTO_ITF_MSG_DELIVERY,ITFLAGS(%eax)
	jnz		ipi_no_tlb_safe
#endif
	testl	$INKERNEL_INTRMASK,inkernel
	jz		1f
	jmp		ipi_no_tlb_safe
	// If we are nested, simply ignore the TLB_SAFE IPI.
	// We could be processing interrupts, in which case we could unmap the
	// interrupt handlers address space...
	// The kernel will simply resend the IPI if it needs to.
10:
	mov		SMPREF(ker_stack,%ebp,4),%esp	/ get some stack space for call
1:
	mov		%ebp,%eax		/ pass current CPU
	LOCKOP
	incl	inkernel
	call	set_safe_aspace
	cli						/ virtual_aspace() enabled interrupts :-(
	LOCKOP
	decl	inkernel
	mov		%edi,%esp		/ restore original ESP value
ipi_no_tlb_safe:

	test	$IPI_CLOCK_LOAD,%esi
	jz		ipi_no_clock_load
	mov		%esp,%edi		/ save original ESP value
	testl	$X86_PSW_VM,STK_EFL(%esp)	/ Check for V86
	jnz	2f
	testl	$0x03,STK_CS(%esp)
	jz		1f
2:
	mov		SMPREF(ker_stack,%ebp,4),%esp	/ get some stack space for call
1:
	LOCKOP
	incl	inkernel
	call	clock_load
	cli						/ clock_load() enabled interrupts :-(
	LOCKOP
	decl	inkernel
	mov		%edi,%esp		/ restore original ESP value
ipi_no_clock_load:

	test	$IPI_INTR_UNMASK,%esi
	jz		ipi_no_intr_unmask
	mov		%esp,%edi		/ save original ESP value
	testl	$X86_PSW_VM,STK_EFL(%esp)	/ Check for V86
	jnz		2f
	testl	$0x03,STK_CS(%esp)
	jz		1f
2:
	mov		SMPREF(ker_stack,%ebp,4),%esp	/ get some stack space for call
1:
	LOCKOP
	incl	inkernel
	movl	$INTR_FLAG_SMP_BROADCAST_UNMASK,%eax
	call	interrupt_smp_sync
	cli						/ interrupt_smp_sync() enabled interrupts :-(
	LOCKOP
	decl	inkernel
	mov		%edi,%esp		/ restore original ESP value
ipi_no_intr_unmask:

	test	$IPI_INTR_MASK,%esi
	jz		ipi_no_intr_mask
	mov		%esp,%edi		/ save original ESP value
	testl	$X86_PSW_VM,STK_EFL(%esp)	/ Check for V86
	jnz		2f
	testl	$0x03,STK_CS(%esp)
	jz		1f
2:
	mov		SMPREF(ker_stack,%ebp,4),%esp	/ get some stack space for call
1:
	LOCKOP
	incl	inkernel
	movl	$INTR_FLAG_SMP_BROADCAST_MASK,%eax
	call	interrupt_smp_sync
	cli						/ interrupt_smp_sync() enabled interrupts :-(
	LOCKOP
	decl	inkernel
	mov		%edi,%esp		/ restore original ESP value
ipi_no_intr_mask:

	xor		%eax,%eax
	test	$IPI_RESCHED,%esi
	jz		ipi_no_resched
	or		$_NTO_ATF_SMP_RESCHED,%eax

ipi_no_resched:
	test	$IPI_TIMESLICE,%esi
	jz		ipi_no_timeslice
	or		$_NTO_ATF_TIMESLICE,%eax

ipi_no_timeslice:
	test	$IPI_TLB_FLUSH,%esi
	jz		ipi_no_tlb_flush
	mov		%cr3,%ecx
	mov		%ecx,%cr3

// We don't explicitly check for the IPI_CHECK_INTR bit because the only
// thing we have to do for it is to fall back through intr_done2.  That
// is taken care of down below in it ipi_done.

ipi_no_tlb_flush:
	test	%eax,%eax
	jz		ipi_done
	mov		SMPREF(actives,%ebp,4),%ebx
	testl	$0x03,STK_CS(%esp)
	jnz		force_kernel	/ if user exception

	// Were we spinning waiting for the kernel?
	movl	STK_EIP(%esp),%ecx
	cmp		$acquire_kernel_attempt,%ecx
	jb		1f
	cmp		$end_acquire_kernel_attempt,%ecx
	ja		1f

#if 1
	// We're spining, waiting for the kernel. Pretend we were in
	// userland
	or		$_NTO_ATF_WAIT_FOR_KER,%eax
	jmp		force_kernel
#endif

1:
	lock; or %eax,ATFLAGS(%ebx)

ipi_done:
    // it is "safe" to handle the TLB_SAFE, RESCHED and CHECK_INTR ipi's but not the others
	test	$IPI_TLB_SAFE+IPI_RESCHED+IPI_TIMESLICE+IPI_CHECK_INTR+IPI_INTR_MASK+IPI_INTR_UNMASK,%esi
	jz		irq_not_enabled
	mov		STK_CS(%esp),%eax
	/ @@@ Hmmm. I think we should go to intr_done2 for all cases (preempt case)
	testl	$0x03,%eax
	jnz		1f
	testl	$X86_PSW_VM,STK_EFL(%esp)	/ Check for V86
	jz		2f
1:
	/ If we were in user mode, we need to make sure interrupt queue gets drained
	mov		SMPREF(ker_stack,%ebp,4),%esp	/ load proper stack
	or 		$3,%eax
2:
	push	%eax
	jmp		intr_done2

irq_not_enabled:
	POPREG
	iret

__ipi2:
	PUSHREG
	mov		lapicid_addr,%ebx
	xor		%edx,%edx
	mov		%edx,0x0b0-0x20(%ebx)
	testl	$X86_PSW_VM,STK_EFL(%esp)	/ Check for V86
	jnz		irq_not_enabled
	mov		STK_CS(%esp),%eax	/ Load pushed CS
	push	%eax
	test	$3,%eax				/ Nested?
	jz		intr_done2			/ already on kernel stack if so
	//
	// Strange case. We were in the kernel when the other CPU took an
	// interrupt. It noticed that there was a queued_event_priority higher
	// than it's active[cpu]->priority, but it couldn't do the re-sched
	// since we were in the kernel. It sent this IPI to us, but by the
	// time we got it, we were already back in user code. Force a
	// NOP kernel call to make sure that we didn't miss draining the
	// event queue.
	//
	pop		%eax
	xor		%eax,%eax
	GETCPU	%bp, %ebp
	mov		SMPREF(actives,%ebp,4),%ebx
	jmp		force_kernel

#endif
