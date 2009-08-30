#include "asmoff.def"

	.globl	outside_display_char
	.globl	outside_fault_entry

	.text

outside_display_char:
	b		display_char


outside_fault_entry:
	b		fault_entry
