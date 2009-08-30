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
 *  sys/compiler_watcom.h
 *

 *
 *  The only symbols made visible by this header are
 *  OS/compiler reserved symbols.
 */
#ifndef __COMPILER_WATCOM_H_INCLUDED
#define __COMPILER_WATCOM_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#error sys/compiler_watcom.h should not be included directly.
#endif

#ifdef __CHAR_SIGNED__
#undef __CHAR_UNSIGNED__
#else
#ifndef __CHAR_UNSIGNED__
#define __CHAR_UNSIGNED__
#endif
#endif

#ifdef __INLINE_FUNCTIONS__
#define __OPTIMIZE__
#endif

#if defined(__X86__)
#undef __BIGENDIAN__
#ifndef __LITTLEENDIAN__
#define __LITTLEENDIAN__
#endif
#else
#error CPU Not supported by compiler
#endif

#if defined(__386__)
#define __INT_BITS__	32
#if defined(__HUGE__) || defined(__COMPACT__) || defined(__LARGE__)
#define __PTR_BITS__	48
#else
#define __PTR_BITS__	32
#endif
#elif defined(__SMALL__) || defined(__MEDIUM__)
#define __INT_BITS__	16
#define __PTR_BITS__	16
#elif defined(__HUGE__) || defined(__COMPACT__) || defined(__LARGE__)
#define __INT_BITS__	16
#define __PTR_BITS__	32
#else
#error Unknown model
#endif
#define __LONG_BITS__	32

#define __LONGDOUBLE_BITS__		64

#ifndef	__CDEFS_H_INCLUDED
#include _NTO_HDR_(sys/cdefs.h)
#endif

#if __WATCOMC__+0 >= 1100
typedef unsigned __int64			_Uint64t;
typedef signed __int64				_Int64t;
#elif !defined(__QNXNTO__)
typedef struct { unsigned long __1,__2; } _Uint64t;
typedef struct { long          __1,__2; } _Int64t;
#else
#error Compiler Not supported
#endif

#if __INT_BITS__ == 32
typedef unsigned 					_Uint32t;
typedef int							_Int32t;
#else
typedef unsigned long				_Uint32t;
typedef signed long					_Int32t;
#endif

#if __INT_BITS__ == 16
typedef unsigned					_Uint16t;
typedef int							_Int16t;
#else
typedef unsigned short				_Uint16t;
typedef signed short				_Int16t;
#endif

typedef unsigned char				_Uint8t;
typedef signed char					_Int8t;

#if __PTR_BITS__ == 16
typedef _Uint16t					_Uintptrt;
typedef _Int16t						_Intptrt;
#elif __PTR_BITS__ == 32
typedef _Uint32t					_Uintptrt;
typedef _Int32t						_Intptrt;
#elif __PTR_BITS__ <= 64
typedef _Uint64t					_Uintptrt;
typedef _Int64t						_Intptrt;
#else
#error Unable to declare intregal pointer type
#endif

typedef _Int64t						_Longlong;
typedef _Uint64t					_ULonglong;

/*
cc file.c                    (ANSI/QNX/UNIX/POSIX1003.1a,b,c/POSIX-Draft)
cc -za file.c                (ANSI)
cc -D_POSIX_SOURCE           (ANSI/POSIX1003.1a)
cc -D_POSIX_C_SOURCE=199009  (ANSI/POSIX1003.1a)
cc -D_POSIX_C_SOURCE=199309  (ANSI/POSIX1003.1a,b)
cc -D_POSIX_C_SOURCE=199506  (ANSI/POSIX1003.1a,b,c)
cc -D_QNX_SOURCE             (ANSI/QNX/POSIX1003.1a,b,c/POSIX-Draft)
cc -za -D_UNIX_SOURCE        (ANSI/UNIX)

(cc -za defines NO_EXT_KEYS)
*/

#ifndef NO_EXT_KEYS
#define __EXT
#endif

#endif


/* __SRCVERSION("compiler_watcom.h $Rev: 153052 $"); */
