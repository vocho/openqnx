#include <mips/asm.h>

	.abicalls
	.text

	.extern	_gp
	.extern	_init_libc
	.extern main
	.extern exit
	.extern atexit
	.extern _init
	.extern _fini
	.extern	errno

	.set	noreorder

//	We don't really need to store the %ra -- or restore the %sp, but we
//	do so to make this look API compliant. GDB expects it.

#define STACK_SIZE	48

/*
 * SVR4 PIC defines gp to be caller save, so we need to restore it after any
 * jump to a subroutine.
 * However the MIPS assembler will only do this automatically when reordering is set.
 * Since we don't like to trust the assembler, we need to restore them automatically
 * Technically this means we don't really set the .cprestore directive, but we'll leave
 * it in for completeness.
 */
#define GP_RESTORE_OFFSET 24

	.globl	_start
_start:
	.cpload $25
	move	t0,sp		//Save pointer to argc, argv, envv, auxv list
	addiu	sp,-STACK_SIZE //Make room for parms
	move	ra,zero // load 0 address to ra so gdb won't try to take further traceback
	sw		ra,(STACK_SIZE-4)(sp)
	.cprestore GP_RESTORE_OFFSET // see note above about GP restoration
	lw		a0,(t0)		// get argc
	addiu	a1,t0,4		// point to argv
	sll		t1,a0,2		// skip over argv list
	addu	a2,t1,a1	// ...
	addiu	a2,4		// and point at envv
	move	a3,a2		// find auxv
1:	
	lw		t1,(a3)
	bne		t1,zero, 1b
	 addiu	a3,4

	move	s0,a0			//Save argc, argv, envv for later
	move	s1,a1
	move	s2,a2
	move	s3,a3                   //Save auxv
	la		t9,_init_libc	//Might be too far away for direct calls
	jal		t9
	 sw		v0,16(sp)	// Pass atexit function ptr to _init_libc
	lw		gp,GP_RESTORE_OFFSET(sp) // Set note above, about gp restore
	 
	la		t9,atexit
	la		a0,_fini
	jal 	t9
	 nop
	lw		gp,GP_RESTORE_OFFSET(sp) // Set note above, about gp restore

	la		t9,_init
	jal		t9
	 nop
	lw		gp,GP_RESTORE_OFFSET(sp) // Set note above, about gp restore

	sw		zero,errno
	
	move	a0,s0
	move	a1,s1
	la      t9,	main
	jal t9
	 move	a2,s2
	lw		gp,GP_RESTORE_OFFSET(sp) // Set note above, about gp restore
	 
	move	a0,v0
	la		t9,exit
	j		t9
	 move	ra,zero
	lw		gp,GP_RESTORE_OFFSET(sp) // Set note above, about gp restore

	// not reached -- but the API requires this.
	lw		ra,(STACK_SIZE-4)(sp)
	j		ra
	 addiu	sp,STACK_SIZE //Restore stack
	lw		gp,GP_RESTORE_OFFSET(sp) // Set note above, about gp restore

	.size _start,.-_start
	.type _start,@function
