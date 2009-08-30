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




#define EXTRA_OVERHEAD		0
#define CALL_OVERHEAD		4

.text
.globl vfork
vfork:
#ifdef __PIC__
	pushl %ebx
	call 1f
1: 	popl %ebx
	addl $_GLOBAL_OFFSET_TABLE_+[.-1b],%ebx
#define LOCAL_FRAME_SIZE	4
#else
#define LOCAL_FRAME_SIZE	0
#endif
	movl CALL_OVERHEAD+LOCAL_FRAME_SIZE(%esp),%eax
	leal CALL_OVERHEAD+LOCAL_FRAME_SIZE+EXTRA_OVERHEAD(%esp),%eax
	pushl %eax
	pushl $0		/* No flags for vfork */
#ifdef __PIC__
	call _fork@PLT
#else
	call _fork
#endif
	addl $8,%esp
#ifdef __PIC__
	popl %ebx
#endif
	ret

	.type	 vfork,@function
	.size	 vfork,.-vfork
