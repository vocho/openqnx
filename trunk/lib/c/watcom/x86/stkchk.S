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




#include "asmoff.def"

	.type	 __STK,@function
.globl		__STK
	.type	 __CHK,@function
.globl		__CHK
	.type	 __GRO,@function
.globl		__GRO
	.type	 __STKOVERFLOW,@function
.globl		__STKOVERFLOW
.extern		__tls
.extern		__stkabort
__STK:
    xchgl     (%esp),%eax
    pushl     %eax
__CHK:
    pushl     %eax
    movw      %ss,%ax
    testb     $0x03,%al
    je        1f
    movl      0x8(%esp),%eax
    cmpl      %esp,%eax
    jae       __STKOVERFLOW
    subl      %esp,%eax
    negl      %eax
    pushl     %esi
    pushl     %eax
    call      __tls
    movl      %eax,%esi
    popl      %eax
    cmpl      TLS_STACKADDR(%esi),%eax
    popl      %esi
    jbe       __STKOVERFLOW
1:
    popl      %eax
__GRO:
    ret       $0x00000004
__STKOVERFLOW:
    popl      %eax
    popl      %eax
    addl      $0x00000100,%esp		/* Make some room */
    pushl     %eax
    jmp       __stkabort
