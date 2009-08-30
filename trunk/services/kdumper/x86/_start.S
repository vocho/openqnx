	.extern	_main 
	.extern	_stack_top 
	.extern	_syspage_ptr 

	.global	_cstart_
	.global	_start

#include "asmoff.def"

// Must be just in front of "_start" label	
	.extern	bootstrap
.long	IFS_BOOTSTRAP_SIGNATURE
.long	bootstrap
_cstart_:
_start:
	cli
	movw	%ds,%ax
	movw	%ax,%ss
	mov		_stack_top,%esp
	cld
	push	%edi		// startup passed syspage pointer in EDI
	call	_main		// _main never returns

//end	_cstart_
