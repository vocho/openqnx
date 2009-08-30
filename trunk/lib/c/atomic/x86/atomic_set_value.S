/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */




.text

atomic_set_value:
    movl    0x4(%esp),%edx
	movl	(%edx),%eax
1:
    movl    0x8(%esp),%ecx
	or		%eax,%ecx
	lock; cmpxchg	%ecx,(%edx)
	jnz		1b
    ret       
	
	.globl	atomic_set_value
	.type	atomic_set_value,@function
	.size	atomic_set_value,.-atomic_set_value
