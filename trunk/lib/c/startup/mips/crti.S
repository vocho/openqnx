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




#ifdef __PIC__
	.qnxpiccalls
#endif
	.section .init
	.globl 	_init
	.type 	_init,@function
_init:
	addiu	$29,-72
	sw		$31,64($29)
	sw		$23,60($29)
#ifdef __PIC__
	.cpload $31
#endif
	

	.section .fini
	.globl	_fini
	.type	_fini,@function
_fini:
	addiu	$29,-72
	sw		$23,60($29)
	sw		$31,64($29)
#ifdef __PIC__
	.cpload $31
#endif
