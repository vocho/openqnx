#include "asmoff.def"

	.globl	outside_timer_reload
	.globl	outside_update_plist
	.globl	outside_watch_entry
	.globl	outside_msg_entry
	.globl	outside_fault_entry

	.text

outside_timer_reload:
	b		kdebug_timer_reload

outside_update_plist:
	mov		pc, lr

outside_watch_entry:
	/*
	 * FIXME: do we want to switch onto _stack_top?
	 */
	b		watch_entry

outside_msg_entry:
	b		msg_entry

outside_fault_entry:
	stmdb	sp!,{r4,r5,r6,lr}

	/*
	 * Make sure interrupts are off
	 */
	mrs		r4, cpsr
	orr		ip, r4, #ARM_CPSR_I | ARM_CPSR_F
	msr		cpsr, ip

	/*
	 * Switch to gdb stack
	 */
	mov		r5, sp
	ldr		sp, =_stack_top
	mov		r6, lr
	ldr		sp, [sp]
	bl		handle_exception

	/*
	 * Switch back to kernel stack and restore interrupts
	 */
	mov		sp, r5
	msr		cpsr, r4
	ldmia	sp!,{r4,r5,r6,pc}
