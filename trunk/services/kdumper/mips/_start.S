/*
 * cstart.S
 *	Startup entry of the gdb image.
 */

/*
 * include files
 */
#include <mips/asm.h>
#include <mips/cpu.h>
#include "asmoff.def"

	/*
	 * Externals referenced
	 */
	.extern _main
	.extern _stack_top,4

	.set mips2

/*
 * _start
 *	The very first code. The A0 register contains the kernel
 *  system page pointer on entry (initialized by startup). It gets passed
 *  to _main.
 */

// Must be just in front of "_start" label	
	.extern	bootstrap
.long	IFS_BOOTSTRAP_SIGNATURE
.long	bootstrap
FRAME(_start,sp,0,ra)
	.set	noreorder

	/*
	 * Set GP to the location assigned by "ld".  Our short data
	 * lives there.
	 */
	la		gp,_gp

	/*
	 * Initialize stack.
	 * The declaration of generic kdebug stack only guarantee
	 * 4 byte alignement, we need to make sure it's 8 byte aligned.
	 */
	lw		t0,_stack_top
	li		t1,0xFFFFFFF8
	and		t0,t0,t1
	move	sp,t0
	sw		sp,_stack_top

	/*
	 * Call _main
	 */
	j		_main			# and we are not coming back
	 move	ra,zero

ENDFRAME(_start)
