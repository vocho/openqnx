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




.extern		main
.extern		_CMain

	.text
.extern _PLEASE_USE_GCC_
.long	_PLEASE_USE_GCC_
	.byte	'N', 'I', 'A', 'M'      /* Used by debugger for setting a break point */
    .long	main
.globl		_cstart_
_cstart_:
    jmp		_CMain
	.type	_cstart_,@function
	.size	_cstart_,.-_cstart_

