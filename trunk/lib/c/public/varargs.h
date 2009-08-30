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



/*
 *  varargs.h   Variable argument macros (UNIX System V definition)
 *              definitions for use with variable argument lists
 *

 *
 */
#ifndef _VARARGS_H_INCLUDED
#define _VARARGS_H_INCLUDED

#if defined (__GNUC__) && (__GNUC__ >= 3)
#error "GCC no longer implements <varargs.h>."
#error "Revise your code to use <stdarg.h>."
#endif

#ifndef _STDARG_H_INCLUDED /* ignore if stdarg.h already used */

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

typedef	__NTO_va_list			va_list;
#define va_alist			__NTO_va_alist
#define va_dcl				__NTO_va_dcl
#define va_start(__p1)		__NTO_va_start_vararg(__p1)
#define va_arg(__p1,__p2)	__NTO_va_arg(__p1,__p2)
#define va_end(__p1)		__NTO_va_end(__p1)

#endif
#endif

/* __SRCVERSION("varargs.h $Rev: 164949 $"); */
