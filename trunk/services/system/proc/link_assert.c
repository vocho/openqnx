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

#ifndef NDEBUG

#include "externs.h"

/* A libc built with debug will pull in __assert which wants to use fprintf,
 * so we have to override it since we don't want any of that gear being
 * pulled into procnto.
 * Since it will only occur when procnto is linked against a debug libc though,
 * put it in it's own file so that it's not linked in unless needed.
 */
void __assert(const char *expr, const char *file, unsigned line, const char *func) {
	if(func) {	
		kprintf("In function %s -- ", func);
	}
	kprintf("%s:%d %s -- assertion failed\n", file, line, expr);
	crash();
}

#ifdef __PPC__
void __NTO_va_arg_type_violation(void) {
	kprintf("va_arg type violation\n");
	crash();
#ifndef _lint
	for( ;; ) {
		// Shut up GCC
	}
#endif	
}
#endif
#endif
