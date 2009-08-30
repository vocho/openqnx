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
 *  stddef.h    Standard definitions
 *

 */
#ifndef _STDDEF_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _STDDEF_H_INCLUDED
#endif

#ifndef _STDDEF_H_DECLARED
#define _STDDEF_H_DECLARED

_STD_BEGIN

#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif

#if defined(__PTRDIFF_T)
typedef __PTRDIFF_T	ptrdiff_t;
#undef __PTRDIFF_T
#endif

#if defined(__WCHAR_T)
typedef __WCHAR_T	wchar_t;
#undef __WCHAR_T
#endif

#ifndef NULL
#define NULL   _NULL
#endif

_STD_END

#if (__GNUC__ >= 4)
#define offsetof(__typ,__id) __builtin_offsetof(__typ,__id)
#elif defined(__cplusplus) && (__GNUC__ == 3 && __GNUC_MINOR__ == 4)
#define offsetof(__typ,__id)                                \
    (__offsetof__(reinterpret_cast <size_t>                 \
                 (&reinterpret_cast <const volatile char &> \
                 (static_cast<__typ *>(0)->__id))))
#else
#define offsetof(__typ,__id) ((_CSTD size_t)&(((__typ*)0)->__id))
#endif

#endif

#ifdef _STD_USING
using std::ptrdiff_t; using std::size_t;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("stddef.h $Rev: 153052 $"); */
