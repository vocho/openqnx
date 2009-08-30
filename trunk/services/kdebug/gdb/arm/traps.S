/*
 * Stub trap handlers used until the kernel sets up its own handlers
 */

	.globl	init_traps

#include "asmoff.def"

	.text

init_traps:
	/*
	 * Copy trap handler entry points.
	 * The startup program has set each entry point to "ldr pc, [pc, #0x18]"
	 *
	 * If MMU X bit (bit 13) is set, vectors are at 0xffff0000
	 */
	mov		r1, #32
	mrc		p15, 0, r0, c1, c0, 0
	tst		r0, #(1 << 13)
	orrne	r1, r1, #0xff000000
	orrne	r1, r1, #0x00ff0000
	adr		r0, __trap_vectors
	mov		r2, #8
0:	ldr		ip, [r0], #4
	str		ip, [r1], #4
	subs	r2, r2, #1
	bne		0b
	mov		pc, lr

__trap_vectors:
	.word	__rst_entry
	.word	__und_entry
	.word	__swi_entry
	.word	__prf_entry
	.word	__abt_entry
	.word	__rsv_entry
	.word	__irq_entry
	.word	__fiq_entry

__sigtrap: .word SIGTRAP | SIGCODE_KERNEL
__sigsegv: .word SIGSEGV | SIGCODE_KERNEL
__sigint:  .word SIGINT  | SIGCODE_KERNEL
__sigsys:  .word SIGSYS  | SIGCODE_KERNEL

.macro	SAVE_CONTEXT pc_adjust
	/*
	 * Save context on gdb stack
	 */
	ldr		sp, =_stack_top
	ldr		sp, [sp]
	sub		sp, sp, #17*4

	/*
	 * Non-banked registers
	 */
	stmia	sp, {r0-r12}

	/*
	 * Return pc and status register
	 */
	sub		lr, lr, #&pc_adjust
	str		lr, [sp, #ARM_REG_PC*4]
	mrs		r0, spsr
	str		r0, [sp, #ARM_REG_SPSR*4]

	/*
	 * Switch to SVC mode and save banked registers
	 */
	mov		r2, sp
	add		r3, sp, #ARM_REG_SP*4
	mrs		r1, cpsr
	bic		r1, r1, #ARM_CPSR_MODE_MASK
	orr		r1, r1, #ARM_CPSR_MODE_SVC
	and		r0, r0, #ARM_CPSR_MODE_MASK
	teq		r0, #ARM_CPSR_MODE_SVC
	stmeqia	r3, {sp,lr}
	stmneia	r3, {sp,lr}^
.endm

.macro	RESTORE_CONTEXT
	ldr		r0, [sp, #ARM_REG_SPSR*4]
	msr		spsr, r0
	mov		r0, sp
	ldmia	r0, {r0-r15}^
.endm

/*
 * Reset exception (branch though zero)
 */
__rst_entry:
// FIXME

/*
 * Undefined instruction exception
 */
__und_entry:
	SAVE_CONTEXT 4
	mov		r0, #0
	ldr		r1, __sigtrap
	mov		sp, r2
	bl		handle_exception
	RESTORE_CONTEXT

/*
 * SWI instruction
 */
__swi_entry:
	sub		sp, sp, #17*4
	/*
	 * Non-banked registers
	 */
	stmia	sp, {r0-r12}

	/*
	 * Return pc and status register
	 */
	sub		lr, lr, #4
	str		lr, [sp, #ARM_REG_PC*4]
	mrs		r0, spsr
	str		r0, [sp, #ARM_REG_SPSR*4]

	/*
	 * Banked registers
	 */
	and		r0, r0, #ARM_CPSR_MODE_MASK
	teq		r0, #ARM_CPSR_MODE_SVC
	stmeqia	r3, {sp,lr}
	stmneia	r3, {sp,lr}^

	mov		r0, #0
	ldr		r1, __sigsys
	mov		sp, r2
	bl		handle_exception
	RESTORE_CONTEXT

/*
 * Prefetch abort exception
 */
__prf_entry:
	SAVE_CONTEXT 4
	mov		r0, #0
	ldr		r1, __sigsegv
	mov		sp, r2
	bl		handle_exception
	RESTORE_CONTEXT

/*
 * Data abort exception
 */
__abt_entry:
	SAVE_CONTEXT 8
	mov		r0, #0
	ldr		r1, __sigsegv
	mov		sp, r2
	bl		handle_exception
	RESTORE_CONTEXT

/*
 * Reserved exception
 */
__rsv_entry:
// FIXME

/*
 * IRQ interrupt
 */
__irq_entry:
	SAVE_CONTEXT 4
	mov		r0, #0
	ldr		r1, __sigint
	mov		sp, r2
	bl		handle_exception
	RESTORE_CONTEXT

/*
 * FIQ interrupt
 */
__fiq_entry:
	SAVE_CONTEXT 4
	mov		r0, #0
	ldr		r1, __sigint
	mov		sp, r2
	bl		handle_exception
	RESTORE_CONTEXT
