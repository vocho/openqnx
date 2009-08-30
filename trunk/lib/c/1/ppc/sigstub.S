/*
Copyright 2001, QNX Software Systems Ltd. All Rights Reserved
 
This source code has been published by QNX Software Systems Ltd. (QSSL).
However, any use, reproduction, modification, distribution or transfer of
this software, or any software which includes or is based upon any of this
code, is only permitted under the terms of the QNX Realtime Plaform End User
License Agreement (see licensing.qnx.com for details) or as otherwise
expressly authorized by a written license agreement from QSSL. For more
information, please email licensing@qnx.com.
*/

/*
 * WARNING WARNING WARNING - gdb uses a bytecode signature to sniff for signal handler
 * frames.
 *
 * DON'T CHANGE THIS FILE without consulting with the gdb guys...
 *
 */

/*
 * sigstub.s
 *	Routine for delivering a signal to a thread
 */
	.include "asmoff.def"
	.include "ppc/util.ah"

	.extern SignalReturn


/* Unfortunately, we've got to use the macros instead of opcodes so that
we maintain backwards compatability with earlier tools... */

.macro _evmergehi rd,ra,rb
	.long 0b00010000000000000000001000101100 + (&rd<<21) + (&ra<<16) +(&rb<<11)
.endm

.macro _evmergelo rd,ra,rb
	.long 0b00010000000000000000001000101101 + (&rd<<21) + (&ra<<16) +(&rb<<11)
.endm

.macro _evxor rd,ra,rb
	.long 0b00010000000000000000001000010110 + (&rd<<21) + (&ra<<16) +(&rb<<11)
.endm

.macro _evmwumiaa rd,ra,rb
	.long 0b00010000000000000000010101011000 + (&rd<<21) + (&ra<<16) +(&rb<<11)
.endm

.macro _evstdd rs,d,ra
	.long 0b00010000000000000000001100100001 + (&rs<<21) + (&ra<<16) +((&d/8)<<11)
.endm

.macro _evldd rd,d,ra
	.long 0b00010000000000000000001100000001 + (&rd<<21) + (&ra<<16) +((&d/8)<<11)
.endm

.macro _evmra ra,rd
	.long 0b00010000000000000000010011000100 + (&rd<<21) + (&ra<<16)
.endm



/* Use FPU registers to store HI portion... It's disconcerting
that these values have to be hardcoded as opposed to determined 
from the .h file... There are 41 32 bit ints in the CPU registers
struct and it's followed directly by the FPU registers. */

#define REG_CTR			(32*4)
#define REG_LR			(33*4)

#define REG_SPEFSCR		(40*4)
#define SPR_SPEFSCR		512

/* Offset of FPU registers given CPU offset. */
#define START_FPU 		(UCONTEXT_FPU - UCONTEXT_CPU)

/* Store accumulator after high order word of GPRs. */
#define REG_SPE_ACC		(32*4)



.macro SAVE_EVR reg,tmp,base
.if &reg - &tmp
	_evmergehi	&tmp,&tmp,&reg
	stw			&tmp,(&reg*4)(&base)
.endif
.endm

.macro REST_EVR reg,tmp,base
.if &reg - &tmp
	lwz			&tmp,(&reg*4)(&base)
	_evmergelo	&reg,&tmp,&reg
.endif
.endm

.macro SAVE_SPE tmp,base
	_evmergehi	&tmp,&tmp,&tmp
	stw			&tmp,(&tmp*4)(&base)
	SAVE_EVR 0,&tmp,&base
	SAVE_EVR 1,&tmp,&base
	SAVE_EVR 2,&tmp,&base
	SAVE_EVR 3,&tmp,&base
	SAVE_EVR 4,&tmp,&base
	SAVE_EVR 5,&tmp,&base
	SAVE_EVR 6,&tmp,&base
	SAVE_EVR 7,&tmp,&base
	SAVE_EVR 8,&tmp,&base
	SAVE_EVR 9,&tmp,&base
	SAVE_EVR 10,&tmp,&base
	SAVE_EVR 11,&tmp,&base
	SAVE_EVR 12,&tmp,&base
	SAVE_EVR 13,&tmp,&base
	SAVE_EVR 14,&tmp,&base
	SAVE_EVR 15,&tmp,&base
	SAVE_EVR 16,&tmp,&base
	SAVE_EVR 17,&tmp,&base
	SAVE_EVR 18,&tmp,&base
	SAVE_EVR 19,&tmp,&base
	SAVE_EVR 20,&tmp,&base
	SAVE_EVR 21,&tmp,&base
	SAVE_EVR 22,&tmp,&base
	SAVE_EVR 23,&tmp,&base
	SAVE_EVR 24,&tmp,&base
	SAVE_EVR 25,&tmp,&base
	SAVE_EVR 26,&tmp,&base
	SAVE_EVR 27,&tmp,&base
	SAVE_EVR 28,&tmp,&base
	SAVE_EVR 29,&tmp,&base
	SAVE_EVR 30,&tmp,&base
	SAVE_EVR 31,&tmp,&base
	_evxor		&tmp,&tmp,&tmp
	_evmwumiaa	&tmp,&tmp,&tmp
	_evstdd		&tmp,REG_SPE_ACC, &base
.endm

.macro REST_SPE tmp,base
	_evldd		&tmp,REG_SPE_ACC, &base
	_evmra		&tmp,&tmp
	REST_EVR 0,&tmp,&base
	REST_EVR 1,&tmp,&base
	REST_EVR 2,&tmp,&base
	REST_EVR 3,&tmp,&base
	REST_EVR 4,&tmp,&base
	REST_EVR 5,&tmp,&base
	REST_EVR 6,&tmp,&base
	REST_EVR 7,&tmp,&base
	REST_EVR 8,&tmp,&base
	REST_EVR 9,&tmp,&base
	REST_EVR 10,&tmp,&base
	REST_EVR 11,&tmp,&base
	REST_EVR 12,&tmp,&base
	REST_EVR 13,&tmp,&base
	REST_EVR 14,&tmp,&base
	REST_EVR 15,&tmp,&base
	REST_EVR 16,&tmp,&base
	REST_EVR 17,&tmp,&base
	REST_EVR 18,&tmp,&base
	REST_EVR 19,&tmp,&base
	REST_EVR 20,&tmp,&base
	REST_EVR 21,&tmp,&base
	REST_EVR 22,&tmp,&base
	REST_EVR 23,&tmp,&base
	REST_EVR 24,&tmp,&base
	REST_EVR 25,&tmp,&base
	REST_EVR 26,&tmp,&base
	REST_EVR 27,&tmp,&base
	REST_EVR 28,&tmp,&base
	REST_EVR 29,&tmp,&base
	REST_EVR 30,&tmp,&base
	REST_EVR 31,&tmp,&base
	lwz			&tmp,(&tmp*4)(&base)
	_evmergelo	&tmp,&tmp,&tmp
.endm


.macro SAVE_REG reg,base
	stw			&reg, (&reg*4)(&base)
.endm

.macro REST_REG reg,base
	lwz			&reg, (&reg*4)(&base)
.endm


.macro SAVE_SPEFSCR tmp,base
	mfspr		&tmp,SPR_SPEFSCR
	stw			&tmp,REG_SPEFSCR(&base)
.endm

.macro REST_SPEFSCR tmp,base
	lwz			&tmp,REG_SPEFSCR(&base)
	mtspr		SPR_SPEFSCR,&tmp
.endm




 #
 # int __signalstub (struct _sighandler_info *ptr)	// in R0
 #
 # The kernel has already saved GPR0,GPR1,GPR3,MSR,IAR,CR,XER for us. It also 
 # ensures that the stack is 16-byte aligned.
 # New kernels (>= 6.3.0) also save GPR11, 12  and CTR since they can be
 # damaged by function call sequences (e.g. PLT code).
 #
 	.global 	__signalstub
	.section 	".text"
	.type 		__signalstub,@function
__signalstub:
	addi	%r1,%r1,-16					# alloc new stack frame area
	mr		%r3,%r0						# save sighandler_info pointer
	lwz		%r3,SIGSTACK_CONTEXT(%r3)	# get pointer to context save area
	addi	%r3,%r3,UCONTEXT_CPU		# ...

	SAVE_REG	2,3
	SAVE_REG	4,3
	SAVE_REG	5,3
	SAVE_REG	6,3
	SAVE_REG	7,3
	SAVE_REG	8,3
	SAVE_REG	9,3
	SAVE_REG	10,3

	# The kernel, if it has saved R11/R12/CTR, will set CR7 to 0xf, an 
	# 'impossible' value for any normal instruction to set it to. We can 
	# use this as a trigger to avoid storing the (possibly corrupted) values
	# again.
	mfcr	%r4
	andi.	%r4,%r4,0xf
	cmplwi	%r4,0xf
	beq+	1f
	SAVE_REG	11,3
	SAVE_REG	12,3
	mfctr	%r11
	stw		%r11,REG_CTR(%r3)
1:

	SAVE_REG	13,3
	SAVE_REG	14,3
	SAVE_REG	15,3
	SAVE_REG	16,3
	SAVE_REG	17,3
	SAVE_REG	18,3
	SAVE_REG	19,3
	SAVE_REG	20,3
	SAVE_REG	21,3
	SAVE_REG	22,3
	SAVE_REG	23,3
	SAVE_REG	24,3
	SAVE_REG	25,3
	SAVE_REG	26,3
	SAVE_REG	27,3
	SAVE_REG	28,3
	SAVE_REG	29,3
	SAVE_REG	30,3
	SAVE_REG	31,3

	mflr	%r4
	stw		%r4,REG_LR(%r3)

	/* Check and see if this is an SPE enabled CPU and
	if so, save the additional SPE register context. */
#ifdef __PIC__
	bl		_GLOBAL_OFFSET_TABLE_@local-4
	mflr	%r4
	lwz		%r4, __cpu_flags@got(%r4)
	lwz		%r4, 0(%r4)
#else
	lis		%r4, __cpu_flags@ha
	lwz		%r4, __cpu_flags@l(%r4)
#endif
	bittst	%r29, %r4, PPC_CPU_SPE
	beq+ 	1f

	SAVE_SPEFSCR	4,3
	
	/* Start of FPU context. */
	addi 	%r2, %r3, START_FPU
	SAVE_SPE	4,2

1:
	mr		%r31,%r3					# save context pointer
	mr		%r30,%r0					# save sighandler pointer
	
	mr		%r4,%r0
	lwz		%r0,SIGSTACK_HANDLER(%r4) 	# get handler routine
	subi	%r5,%r3,UCONTEXT_CPU
	mtctr	%r0
	lwz		%r3,SIGSTACK_SIGNO(%r4) 	# get signal number
	bctrl								# call user signal handler
	
    mr		%r3,%r30					# restore sighandler pointer

	bittst	%r29, %r29, PPC_CPU_SPE
	beq+ 	2f

	addi		%r2, %r31, START_FPU
	REST_SPE	0,2
	
	REST_SPEFSCR	0,31

2:
	lwz		%r0,REG_LR(%r31)
	mtlr	%r0
	lwz		%r0,REG_CTR(%r31)
	mtctr	%r0


	REST_REG	2,31
	REST_REG	4,31
	REST_REG	5,31
	REST_REG	6,31
	REST_REG	7,31
	REST_REG	8,31
	REST_REG	9,31
	REST_REG	10,31
	REST_REG	11,31
	REST_REG	12,31
	REST_REG	13,31
	REST_REG	14,31
	REST_REG	15,31
	REST_REG	16,31
	REST_REG	17,31
	REST_REG	18,31
	REST_REG	19,31
	REST_REG	20,31
	REST_REG	21,31
	REST_REG	22,31
	REST_REG	23,31
	REST_REG	24,31
	REST_REG	25,31
	REST_REG	26,31
	REST_REG	27,31
	REST_REG	28,31
	REST_REG	29,31
	REST_REG	30,31
	REST_REG	31,31
#ifdef __PIC__	
	b		SignalReturn@plt
#else
	b		SignalReturn	# tell the kernel the signal is done
#endif
	.size 	__signalstub,.-__signalstub
