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
 *  stdarg.h    Variable argument macros
 *              definitions for use with variable argument lists
 *

 */
#ifndef _STDARG_H_INCLUDED
#define _STDARG_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

typedef	__NTO_va_list			va_list;
#define va_start(__p1,__p2)	__NTO_va_start_stdarg(__p1,__p2)
#define va_arg(__p1,__p2)	__NTO_va_arg(__p1,__p2)
#define __va_arg(__p1,__p2)	__NTO_va_arg(__p1,__p2)
#define va_end(__p1)		__NTO_va_end(__p1)
#define va_copy(__d,__s)	__NTO_va_copy(__d,__s)

#endif

/* __SRCVERSION("stdarg.h $Rev: 164964 $"); */
