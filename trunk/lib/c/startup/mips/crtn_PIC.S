
/* This is to make a placeholder note section in the executable */
	.section .note,"a"
	.abicalls
	.align 4
	
	.set noreorder

	.section .init
	lw		$31,32($29)
	jr		$31
	addiu	$29,32 

	.section .fini
	lw		$31,32($29)
	jr		$31
	addiu	$29,32 
