	.extern	handle_exception 
	.extern	kdebug_timer_reload 
	.extern	_stack_base 
	.extern	_stack_top 
	.extern	kerdebug_reg 
	.extern	ker_ds 
	.extern	__ds 
	.extern	__es 
	.extern	__fs 
	.extern	__gs 
	.extern	spinlock 

	.global	__trp_entry 
	.global	__msg_entry 
	.global	__dbg_entry 
	.global	__brk_entry 
	.global	__exc_entry 
	.global	outside_timer_reload 
	.global kdbg

#include "asmoff.def"
#include "local-traps.h"

.macro SPIN_ENTER  
	push	%eax
	movb	$1,%al
99:
	cmpb	$0,spinlock
	jnz 	99b	
	xchgb	spinlock,%al
	orb		%al,%al
	jnz		99b
	pop		%eax
.endm

.macro SPIN_EXIT  
	movb	$0,spinlock
.endm

.macro DEBUG_ENTER  
	pushal
	SPIN_ENTER
#ifdef	__SEGMENTS__
	push	%ds
	push	%es
	push	%fs
	push	%gs
#else
	mov		%ds,%ss:__ds
	mov		%es,%ss:__es
	mov		%fs,%ss:__fs
	mov		%gs,%ss:__gs
#endif

	movw	%cs:ker_ds,%bx
	movw	%bx,%es
	mov		%esp,%esi

	movw	%ss,%cx
	movw	%bx,%ds
	mov		$kerdebug_reg,%edi
	mov		%edi,%edx
	mov		$SIZEOF_REG>>2,%ecx			//Works, but SIZEOF_REG>>2 doesn't!

	cld
	rep;	movsl

	movw	%bx,%ds
	testl	$3,REG_CS(%edx)	// Check if a privity switch occured
	jnz	    98f
	lea		-2*4(%esi),%esi
	mov		%esi,REG_ESP(%edx)	// Save callers esp since hardware did not
	mov		%ss,REG_SS(%edx)		// Save callers ss since hardware did not
98:
	movw	%bx,%ss
	mov		_stack_top,%esp
	mov		$0,%ecx
.endm

.macro DEBUG_LEAVE  
	cli
	mov		$kerdebug_reg,%esp
#ifdef	__SEGMENTS__
	pop		%gs
	pop		%fs
	pop		%es
	pop		%ds
#else
	mov		%ss:__ds,%ds
	mov		%ss:__es,%es
	mov		%ss:__fs,%fs
	mov		%ss:__gs,%gs
#endif
	popal
	testl	$3,1*4(%esp)	// Check if a privity switch will occur
	jnz		97f
	pop		%ss:_stack_base+0	// Return eip
	pop		%ss:_stack_base+4	// Return cs
	pop		%ss:_stack_base+8	// Return flags
	lss		0*4(%esp),%esp	// Load esp,ss
	push	%cs:_stack_base+8	// Return flags
	push	%cs:_stack_base+4	// Return cs
	push	%cs:_stack_base+0	// Return eip
97:
	SPIN_EXIT
	iret
.endm

__trp_entry:
	DEBUG_ENTER
	mov		TRP_ENTRY_CODE,%eax
	jmp		dbg

__msg_entry:
	DEBUG_ENTER

	mov		MSG_ENTRY_CODE,%eax
	jmp		dbg

__brk_entry:
	DEBUG_ENTER

	mov		BRK_ENTRY_CODE,%eax
	sub		$1,REG_EIP(%edx)	// Move eip back to brkpoint
	jmp		dbg

__dbg_entry:
	DEBUG_ENTER

	mov		DBG_ENTRY_CODE,%eax
	jmp		dbg

__exc_entry:
	add		$4,%esp
	DEBUG_ENTER

	mov		EXC_ENTRY_CODE,%eax
	// fall through
	
dbg:
//ulong_t handle_exception(ulong_t sigcode, CPU_REGISTERS *ctx, struct kdebug_entry *entry);
	cli
	
	//TF ADD pass on args on stack (R->L)
	pushl	%ecx					//Entry (ecx)
	pushl	%edx					//Register (edx)
	pushl	%eax					//Code (eax)

	call    handle_exception
	//TF ADD get rid of stack items we passed
	addl	$0xc, %esp				//3*4 = 12 bytes

	DEBUG_LEAVE					//Return and don't come back
	

kdbg:							//This is where the outside_fault entry come in
	SPIN_ENTER
	push	%ebx
	push	%ecx
	push	%edx
	push	%esi
	push	%edi
	pushfl
	
	//REG in edx, CODE in eax, ENTRY in ecx
	
	mov		%esp,%ebx
	mov		%cs:_stack_top,%esp
	push	%ss
	push	%ebx
	mov		%cs:ker_ds,%ss
	cli
#ifndef	__SEGMENTS__
	mov		%ds,%ss:__ds
	mov		%es,%ss:__es
	mov		%fs,%ss:__fs
	mov		%gs,%ss:__gs
#endif
	testl	$3,REG_CS(%edx)	// Check if a privity switch occured
	jnz		full_save
	// no privity switch, ss:esp not saved
	add		$4,%esp	// throw away saved ESP
	pop		%ss:kerdebug_reg+REG_SS
	mov		REG_EXX(%edx),%ecx		// Get the original ESP value from PUSHA
	add		$3*4,%ecx				// adjust for IRET frame
	mov		%ecx,%ss:kerdebug_reg+REG_ESP
	mov		%cs:ker_ds,%es
	mov		$kerdebug_reg,%edi
	mov		%edx,%esi
	mov		%edi,%edx
	mov		$(SIZEOF_REG-8)>>2,%ecx
	cld
	rep;	movsl
	mov		%cs:ker_ds,%ds
	jmp		dbg

full_save:
	
//ulong_t handle_exception(ulong_t sigcode, CPU_REGISTERS *ctx, struct kdebug_entry *entry);
	//TF ADD pass on args on stack (R->L)
	pushl 	%ecx					//Entry (ecx)
	pushl 	%edx					//Register (edx)
	pushl 	%eax					//Code (eax)

	cli
	call    handle_exception
	//TF ADD get rid of stack items we passed
	addl 	$0xc,%esp				//3*4 = 12 bytes

#ifndef	__SEGMENTS__
	mov		__es,%es
	mov		__fs,%fs
	mov		__gs,%gs
	mov		__ds,%ds
#endif
	SPIN_EXIT
	pop		%edx
	pop		%ss
	mov		%edx,%esp
	popfl
	pop		%edi
	pop		%esi
	pop		%edx
	pop		%ecx
	pop		%ebx
	ret

outside_timer_reload:
	jmp		kdebug_timer_reload
