	.code32

	.global outside_fault_entry
	.global outside_display_char


/* 
unsigned outside_fault_entry(struct kdebug_entry *entry,  
                             unsigned code,				  
							 CPU_REGISTERS *regs);       
*/

outside_fault_entry:
	jmp		fault_entry


/* 
unsigned outside_display_char(struct syspage_entry *sp, char c);
*/

outside_display_char:
	jmp		display_char
