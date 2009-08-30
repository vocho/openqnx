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
 *  sys/compiler_highc.h
 *

 *
 *  The only symbols made visible by this header are
 *  OS/compiler reserved symbols.
 */
#ifndef __COMPILER_HIGHC_H_INCLUDED
#define __COMPILER_HICHC_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#error sys/compiler_highc.h should not be included directly.
#endif

#error Not currently configured for HIGHC

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
#else
#define __INT_BITS__	32
#define __PTR_BITS__	32
#endif

#define __LONGDOUBLE_BITS__		64

typedef unsigned long long			_Uint64t;
typedef signed long long			_Int64t;

typedef unsigned long				_Uint32t;
typedef signed long					_Int32t;

typedef unsigned short				_Uint16t;
typedef signed short				_Int16t;

typedef unsigned char				_Uint8t;
typedef signed char					_Int8t;

typedef unsigned long				_Uintptrt;
typedef signed long					_Intptrt;

typedef long long					_Longlong;
typedef unsigned long long			_ULonglong;


#endif

/* __SRCVERSION("compiler_highc.h $Rev: 153052 $"); */
