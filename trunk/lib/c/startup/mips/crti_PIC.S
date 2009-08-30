    .set noreorder
	.section .init
	.abicalls
	.globl 	_init
	.type 	_init,@function
_init:
    .cpload $25;
	addiu	$29,-32
	sw		$31,32($29)
    .cprestore 16
	

	.section .fini
	.globl	_fini
	.type	_fini,@function
_fini:
    .cpload $25;
	addiu	$29,-32
	sw		$31,32($29)
    .cprestore 16
