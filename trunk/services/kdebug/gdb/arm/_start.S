/*
 * Entry point of gdb image
 */

#include "asmoff.def"

	.globl	_start

	.text

// Must be just in front of "_start" label	
	.extern	bootstrap
.long	IFS_BOOTSTRAP_SIGNATURE
.long	bootstrap
_start:
	/*
	 * Set up stack pointer and jump to _main
	 * Startup has passed syspageptr in r0
	 */
	ldr		sp, =_stack_top
	ldr		sp, [sp]
	bl		_main
0:	b		0b
