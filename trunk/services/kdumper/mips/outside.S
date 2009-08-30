#include "asmoff.def"
#include <mips/asm.h>
#include <mips/cpu.h>


/*
 * unsigned long outside_fault_entry(struct kdebug *entry, long_t sigcode, CPU_REGISTERS *ctx)
 *
 * Called by the kernel via syspage kdebug_entry to
 * give GDB a chance to handle the exception. 
 */

FRAME(outside_fault_entry,sp,0,ra)
	.set	noreorder
	.set	noat

	/*
	 * First turn off the interrupts
	 */
	mfc0    t0,CP0_SREG             # read status register
	 nop
	addiu   t1,zero,~MIPS_SREG_IE   # get IE mask
	and     t0,t0,t1                # mask off IE
	mtc0    t0,CP0_SREG             # write sreg
	 nop                            # let it settle

	/*
	 * Save caller's gp and make sure
	 * gp point to my small data area
	 */
	move	t2, gp
	la      gp,_gp

	/*
	 * Switch to GDB stack
	 */
	move	t3, sp
	lw		sp,_stack_top

	/*
	 * Create a stack frame on GDB stack
	 */
	addiu	sp,sp,-32
	sw		ra, 24(sp)
	sw		t2, 20(sp)
	sw		t3, 16(sp)

	/*
	 * Call dumper entry
	 */
	jal	fault_entry
	 nop

	lw		ra, 24(sp)
	lw		gp, 20(sp)
	lw		sp, 16(sp)

	jr		ra
	 nop

	.set	reorder
	.set	at
ENDFRAME(outside_fault_entry)


/*
 * void outside_display_char(struct syspage_entry *sp, char c)
 */
FRAME(outside_display_char,sp,0,ra)
	.set	noreorder
	/*
	 * Create a stack frame, save old gp and make it point
	 * to kernel debugger's small data area.
	 */
	addiu	sp,sp,-32
	sw		ra, 24(sp)
	sw		gp, 20(sp)

	la      gp,_gp

	/*
	 * Call dumper entry
	 */
	jal		display_char
	nop

	lw		ra, 24(sp)
	lw		gp, 20(sp)

	addiu	sp,sp,+32
	jr		ra
	 nop
ENDFRAME(outside_display_char)
