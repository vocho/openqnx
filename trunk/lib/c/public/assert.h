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
 *  assert.h
 *

 */

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

_C_STD_BEGIN

#undef assert

#ifdef NDEBUG
 #define assert(__ignore) ((void)0)
#else
__BEGIN_DECLS
extern void __assert(const char *__expr, const char *__file, unsigned __line, const char *__func);
__END_DECLS
#if (defined(__GNUC__) && (2 <= __GNUC__) && (!__STRICT_ANSI__)) || defined(__EXT_ANSIC_199901)
#define assert(__expr)   ((__expr)?(void)0: _CSTD __assert(#__expr,__FILE__,__LINE__,__func__))
#else
#define assert(__expr)   ((__expr)?(void)0: _CSTD __assert(#__expr,__FILE__,__LINE__,0))
#endif
#endif

_C_STD_END

/* __SRCVERSION("assert.h $Rev: 167935 $"); */
