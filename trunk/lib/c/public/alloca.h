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
 *  alloca.h: Allocate memory from the stack.
 *

 */

#ifndef _ALLOCA_H_INCLUDED
#define _ALLOCA_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

extern _Sizet	__stackavail(void);

#undef alloca

#define __ALLOCA_ALIGN( s )   (((s)+(sizeof(_Uint64t)-1))&~(sizeof(_Uint64t)-1))

#if defined(__WATCOMC__)
	#pragma aux __stackavail __modify __nomemory;
	
	extern void *__doalloca(_Sizet __size);
	#pragma aux __doalloca = "sub esp,eax" __parm __nomemory [__eax] __value [__esp] __modify __exact __nomemory [__esp];
	
	#define _alloca(s)         __doalloca(__ALLOCA_ALIGN(s))
#elif defined(__GNUC__) || defined(__INTEL_COMPILER)
	extern void *__builtin_alloca(unsigned int __size);
 	#define _alloca(s)	__builtin_alloca(s)
#else
 	#error not configured for system
#endif

#define alloca(s)	(((__ALLOCA_ALIGN(s)+128)<__stackavail())?_alloca(s):0)
#define __alloca(s)	_alloca(s)

__END_DECLS

#endif

/* __SRCVERSION("alloca.h $Rev: 159125 $"); */
