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
 *  sys/compiler_mwerks.h
 *

 *
 *  The only symbols made visible by this header are
 *  OS/compiler reserved symbols.
 */
#ifndef __COMPILER_MWERKS_H_INCLUDED
#define __COMPILER_MWERKS_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#error sys/compiler_mwerks.h should not be included directly.
#endif

#ifdef __INTEL__
#define __X86__
#endif

#ifdef __POWERPC__
#define __PPC__
#endif

#if defined(__QNX__) && !defined(__QNXNTO__)
#define __QNXNTO__
#endif

#if __option(unsigned_char)
#undef __CHAR_SIGNED__
#define __CHAR_UNSIGNED__
#else
#define __CHAR_SIGNED__
#undef __CHAR_UNSIGNED__
#endif

#if __option(little_endian)
#define __LITTLEENDIAN__
#undef __BIGENDIAN__
#else
#undef __LITTLEENDIAN__
#define __BIGENDIAN__
#endif

#define __INT_BITS__	32
#define __PTR_BITS__	32

#define __LONGDOUBLE_BITS__		64

#ifndef	__CDEFS_H_INCLUDED
#include _NTO_HDR_(sys/cdefs.h)
#endif

#pragma longlong on

typedef unsigned long long			_Uint64t;
typedef signed long long			_Int64t;

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

#if __PTR_BITS__ <= 16
typedef _Uint16t					_Uintptrt;
typedef _Int16t						_Intptrt;
#elif __PTR_BITS__ <= 32
typedef _Uint32t					_Uintptrt;
typedef _Int32t						_Intptrt;
#elif __PTR_BITS__ <= 64
typedef _Uint64t					_Uintptrt;
typedef _Int64t						_Intptrt;
#else
#error Unable to declare intregal pointer type
#endif

typedef long long					_Longlong;
typedef unsigned long long			_ULonglong;

#define __EXT

#endif


/* __SRCVERSION("compiler_mwerks.h $Rev: 153052 $"); */
