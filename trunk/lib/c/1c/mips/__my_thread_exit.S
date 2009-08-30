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



#include <mips/asm.h>

#
# __my_thread_exit()
# Catch threads which return off the top
#
# Trampoline code--this function is actually 'returned' to when the
# main thread function falls off the end of its code.  We snatch the 
# return value v0 of the main()-ish function into an argument register 
# before invoking pthread_exit().
# 

	.text
	.set noreorder
#ifdef __PIC__
	PIC_DIRECTIVE
#endif
	
	.globl __my_thread_exit
	.type __my_thread_exit,@function
	
__my_thread_exit:

#ifdef __PIC__
  # Does not matter if we clobber regs, we are never coming back
#ifdef __MIPS_ABICALLS__
  .cpload $31        
#else
  .cpload $25
#endif        
  la  $25,pthread_exit
  j   $25
#else
	j	pthread_exit
#endif 
	 move $4,$2
	 
 .size __my_thread_exit,.-__my_thread_exit
