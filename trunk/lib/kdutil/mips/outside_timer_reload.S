#include <mips/asm.h>

	.extern async_timer_reload

/*
 * void outside_timer_reload(struct syspage_entry *, struct qtime_entry *)
 *  Hook to check for async stop request
 */
FRAME(outside_timer_reload,sp,0,ra)
	.set	noreorder
	/*
	 * Create a stack frame, save old gp and make it point
	 * to kernel debugger's small data area.
	 */
	addiu	sp,sp,-32
	sw		ra, 24(sp)
	sw		gp, 20(sp)

	la      gp,_gp

	jal		async_timer_reload
	 nop

	lw		ra, 24(sp)
	lw		gp, 20(sp)

	addiu	sp,sp,+32
	jr		ra
	 nop
ENDFRAME(outside_timer_reload)
