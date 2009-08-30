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
 *  sys/compiler_intel.h
 *

 *
 *  The only symbols made visible by this header are
 *  OS/compiler reserved symbols.
 */
#ifndef __COMPILER_INTEL_H_INCLUDED
#define __COMPILER_INTEL_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#error sys/compiler_intel.h should not be included directly.
#endif

#ifdef __CHAR_UNSIGNED__
#undef __CHAR_SIGNED__
#else
#ifndef __CHAR_SIGNED__
#define __CHAR_SIGNED__
#endif
#endif

#if !defined(__BIGENDIAN__) && !defined(__LITTLEENDIAN__)
#error Endian not defined
#endif

#ifndef	__CDEFS_H_INCLUDED
#include _NTO_HDR_(sys/cdefs.h)
#endif

#ifdef __BIGREGS__
#define __INT_BITS__	64
#define __PTR_BITS__	64
#define __LONG_BITS__	64
#else
#define __INT_BITS__	32
#define __PTR_BITS__	32
#define __LONG_BITS__	32
#endif

#if defined(__X86__)
#define __LONGDOUBLE_BITS__		80
#elif defined(__PPC__) || defined(__MIPS__) || defined(__SH__) || defined(__ARM__) || defined(__SPARC__)
#define __LONGDOUBLE_BITS__		64
#else
 #error not configured for CPU
#endif

typedef int							_Int64t __attribute__((__mode__(__DI__),__aligned__(8)));
typedef unsigned int				_Uint64t __attribute__((__mode__(__DI__),__aligned__(8)));

#if __INT_BITS__ == 32
typedef unsigned		 			_Uint32t __attribute__((__aligned__(4)));
typedef int				 			_Int32t __attribute__((__aligned__(4)));
#else
typedef int							_Int32t __attribute__((__mode__(__SI__),__aligned__(4)));
typedef unsigned int				_Uint32t  __attribute__((__mode__(__SI__),__aligned__(4)));
#endif

#if __INT_BITS__ == 16
typedef unsigned		 			_Uint16t __attribute__((__aligned__(2)));
typedef int				 			_Int16t __attribute__((__aligned__(2)));
#else
typedef int							_Int16t __attribute__((__mode__(__HI__),__aligned__(2)));
typedef unsigned int				_Uint16t __attribute__((__mode__(__HI__),__aligned__(2)));
#endif

typedef int							_Int8t __attribute__((__mode__(__QI__),__aligned__(1)));
typedef unsigned int				_Uint8t __attribute__((__mode__(__QI__),__aligned__(1)));

typedef int							_Intptrt __attribute__((__mode__(__pointer__)));
typedef unsigned int				_Uintptrt __attribute__((__mode__(__pointer__)));

__extension__ typedef long long				_Longlong;
__extension__ typedef unsigned long long	_ULonglong;

/*
 * -ansi			defines __STRICT_ANSI__
 * -traditional		undefines __STDC__ and __STDC_VERSION__
 */

#if defined(__STDC__) && !defined(__STRICT_ANSI__)
#define __EXT
#endif

#ifdef __ia64
typedef void *__NTO_va_list;
#else
typedef char *__NTO_va_list;
#endif

#endif


/* __SRCVERSION("compiler_intel.h $Rev: 164949 $"); */
