	.code32

	.global outside_fault_entry

#include "asmoff.def"
#include "local-traps.h"

/* 
 Prototyped in kdebug.h as

unsigned outside_fault_entry(struct kdebug_entry *entry,  
                             unsigned code,				  
							 CPU_REGISTERS *regs);       

  Upon calling kdbg we put the code in eax, entry in ecx
  and the regs in edx (not use for Watcom debugger)
*/

outside_fault_entry:
	mov		4(%esp),%ecx
	mov		8(%esp),%eax
	mov		12(%esp),%edx
    jmp     kdbg
