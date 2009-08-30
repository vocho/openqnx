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
 *  mips/platform.h
 *

 */

#ifndef _MIPS_PLATFORM_H_INCLUDED
#define _MIPS_PLATFORM_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#error mips/platform.h should not be included directly.
#endif

#if defined(__QNXNTO__)

/*
   For gcc-3 and higher, varargs.h is deprecated, and we use the gcc
   builtin var arg support for stdarg.h, set up in sys/compiler_gnu.h.
 */
#if (defined(__GNUC__) && __GNUC__ < 3)
typedef char *__NTO_va_list;
#define __NTO_va_copy(__d,__s)			((__d)=(__s))

#define	__NTO_va_align(ap, type) \
	((__NTO_va_list)(((int)ap + (sizeof(type) + __alignof__(type) - 1)) & ~(__alignof__(type) - 1)))

#define	__NTO_va_start_stdarg(ap, last) (ap = __NTO_va_align(((char *)&(last)), last))
#define __NTO_va_arg(ap, type)  		((type *)(ap = __NTO_va_align(ap, type)))[-1]
#define __NTO_va_end(ap) 				((void)0)

#if __GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 9)
#define __NTO_va_alist			void *__alist, ...
#define __NTO_va_dcl
#define __NTO_va_start_vararg(ap)		(ap = ((char *)&(__alist)))
#else  /* __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 9) */
#define __NTO_va_alist			__builtin_va_alist
#define __NTO_va_dcl			void *__builtin_va_alist; ...
#define __NTO_va_start_vararg(ap)		(ap = ((char *)&(__builtin_va_alist)))
#endif

#endif /* __GNUC__ < 3 */

/*- this needs to be thought out */
#define __JMPBUFSIZE 	32
typedef unsigned 		__jmpbufalign;

#else
#error Not configured for target
#endif

#endif

/* __SRCVERSION("platform.h $Rev: 167027 $"); */
