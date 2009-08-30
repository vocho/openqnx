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
 *  stdint.h Defined system types
 *

 */
#ifndef _STDINT_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _STDINT_H_INCLUDED
#endif

#ifndef _STDINT_H_DECLARED
#define _STDINT_H_DECLARED

_C_STD_BEGIN

typedef _Int8t					int8_t;
typedef _Uint8t					uint8_t;
typedef _Int16t					int16_t;
typedef _Uint16t				uint16_t;
typedef _Int32t					int32_t;
typedef _Uint32t				uint32_t;
typedef _Int64t					int64_t;
typedef _Uint64t				uint64_t;
#if defined(__INTPTR_T)
typedef __INTPTR_T				intptr_t;
#undef __INTPTR_T
#endif
typedef _Uintptrt				uintptr_t;

#define INT8_C(__x)				__x
#define INT16_C(__x)			__x
#define INT32_C(__x)			__x ## L
#if defined(__WATCOM_INT64__)
#define INT64_C(__x)			__x ## I64
#else
#define INT64_C(__x)			__x ## LL
#endif

#define UINT8_C(__x)			__x ## U
#define UINT16_C(__x)			__x ## U
#define UINT32_C(__x)			__x ## UL
#if defined(__WATCOM_INT64__)
#define UINT64_C(__x)			__x ## UI64
#else
#define UINT64_C(__x)			__x ## ULL
#endif

#if defined(__WATCOM_INT64__)
#define INTMAX_C(__x)			__x ## I64
#define UINTMAX_C(__x)			__x ## UI64
#else
#define INTMAX_C(__x)			__x ## LL
#define UINTMAX_C(__x)			__x ## ULL
#endif

#if defined(__EXT_QNX)
/*
 * These types are deprecated and applications should use the
 * int_/uint_ types defined below.
 */
typedef _Intleast8t				intleast8_t;
typedef _Uintleast8t			uintleast8_t;
typedef _Intfast8t				intfast8_t;
typedef _Uintfast8t				uintfast8_t;
 
typedef _Intleast16t			intleast16_t;
typedef _Uintleast16t			uintleast16_t;
typedef _Intfast16t				intfast16_t;
typedef _Uintfast16t			uintfast16_t;

typedef _Intleast32t			intleast32_t;
typedef _Uintleast32t			uintleast32_t;
typedef _Intfast32t				intfast32_t;
typedef _Uintfast32t			uintfast32_t;

typedef _Intleast64t			intleast64_t;
typedef _Uintleast64t			uintleast64_t;
typedef _Intfast64t				intfast64_t;
typedef _Uintfast64t			uintfast64_t;
#endif

typedef _Intleast8t				int_least8_t;
typedef _Uintleast8t			uint_least8_t;
typedef _Intfast8t				int_fast8_t;
typedef _Uintfast8t				uint_fast8_t;

typedef _Intleast16t			int_least16_t;
typedef _Uintleast16t			uint_least16_t;
typedef _Intfast16t				int_fast16_t;
typedef _Uintfast16t			uint_fast16_t;

typedef _Intleast32t			int_least32_t;
typedef _Uintleast32t			uint_least32_t;
typedef _Intfast32t				int_fast32_t;
typedef _Uintfast32t			uint_fast32_t;

typedef _Intleast64t			int_least64_t;
typedef _Uintleast64t			uint_least64_t;
typedef _Intfast64t				int_fast64_t;
typedef _Uintfast64t			uint_fast64_t;

#ifndef _INTMAXT
#define _INTMAXT
typedef _Intmaxt				intmax_t;
typedef _Uintmaxt				uintmax_t;
#endif

#define INT8_MIN				(INT8_C(-0x7f)-INT8_C(1))
#define INT16_MIN				(INT16_C(-0x7fff)-INT16_C(1))
#define INT32_MIN				(INT32_C(-0x7fffffff)-INT32_C(1))
#define INT64_MIN				(INT64_C(-0x7fffffffffffffff)-INT64_C(1))

#define INT8_MAX				INT8_C(0x7f)
#define INT16_MAX				INT16_C(0x7fff)
#define INT32_MAX				INT32_C(0x7fffffff)
#define INT64_MAX				INT64_C(0x7fffffffffffffff)

#define UINT8_MAX				UINT8_C(0xff)
#define UINT16_MAX				UINT16_C(0xffff)
#define UINT32_MAX				UINT32_C(0xffffffff)
#define UINT64_MAX				UINT64_C(0xffffffffffffffff)


#if __PTR_BITS__ <= 16
#define INTPTR_MIN				(INT16_C(-0x7fff)-INT16_C(1))
#define INTPTR_MAX				INT16_C(0x7fff)
#define UINTPTR_MAX				UINT16_C(0xffff)
#elif __PTR_BITS__ <= 32
#define INTPTR_MIN				(INT32_C(-0x7fffffff)-INT32_C(1))
#define INTPTR_MAX				INT32_C(0x7fffffff)
#define UINTPTR_MAX				UINT32_C(0xffffffff)
#elif __PTR_BITS__ <= 64
#define INTPTR_MIN				(INT64_C(-0x7fffffffffffffff)-INT64_C(1))
#define INTPTR_MAX				INT64_C(0x7fffffffffffffff)
#define UINTPTR_MAX				INT64_C(0xffffffffffffffff)
#else
#error Unable to determine pointer limits
#endif

#define PTRDIFF_MIN				INTPTR_MIN
#define PTRDIFF_MAX				INTPTR_MAX

#define INT_LEAST8_MIN			(INT8_C(-0x7f)-INT8_C(1))
#define INT_LEAST16_MIN			(INT16_C(-0x7fff)-INT16_C(1))
#define INT_LEAST32_MIN			(INT32_C(-0x7fffffff)-INT32_C(1))
#define INT_LEAST64_MIN			(INT64_C(-0x7fffffffffffffff)-INT64_C(1))

#define INT_LEAST8_MAX			INT8_C(0x7f)
#define INT_LEAST16_MAX			INT16_C(0x7fff)
#define INT_LEAST32_MAX			INT32_C(0x7fffffff)
#define INT_LEAST64_MAX			INT64_C(0x7fffffffffffffff)

#define UINT_LEAST8_MAX			UINT8_C(0xff)
#define UINT_LEAST16_MAX		UINT16_C(0xffff)
#define UINT_LEAST32_MAX		UINT32_C(0xffffffff)
#define UINT_LEAST64_MAX		UINT64_C(0xffffffffffffffff)

#define INT_FAST8_MIN			(INT8_C(-0x7f)-INT8_C(1))
#define INT_FAST16_MIN			(INT16_C(-0x7fff)-INT16_C(1))
#define INT_FAST32_MIN			(INT32_C(-0x7fffffff)-INT32_C(1))
#define INT_FAST64_MIN			(INT64_C(-0x7fffffffffffffff)-INT64_C(1))

#define INT_FAST8_MAX			INT8_C(0x7f)
#define INT_FAST16_MAX			INT16_C(0x7fff)
#define INT_FAST32_MAX			INT32_C(0x7fffffff)
#define INT_FAST64_MAX			INT64_C(0x7fffffffffffffff)

#define UINT_FAST8_MAX			UINT8_C(0xff)
#define UINT_FAST16_MAX			UINT16_C(0xffff)
#define UINT_FAST32_MAX			UINT32_C(0xffffffff)
#define UINT_FAST64_MAX			UINT64_C(0xffffffffffffffff)

#define INTMAX_MIN				(INT64_C(-0x7fffffffffffffff)-INT64_C(1))
#define INTMAX_MAX				INT64_C(0x7fffffffffffffff)
#define UINTMAX_MAX				UINT64_C(0xffffffffffffffff)

#if __INT_BITS__ <= 16
#define SIG_ATOMIC_MIN			INT16_MIN
#define SIG_ATOMIC_MAX			INT16_MAX
#elif __INT_BITS__ <= 32
#define SIG_ATOMIC_MIN			INT32_MIN
#define SIG_ATOMIC_MAX			INT32_MAX
#elif __INT_BITS__ <= 64
#define SIG_ATOMIC_MIN			INT64_MIN
#define SIG_ATOMIC_MAX			INT64_MAX
#else
#error Unable to determine sigatomic limits
#endif

#define SIZE_MAX				UINT32_MAX

#define WCHAR_MIN				_WCMIN
#define WCHAR_MAX				_WCMAX

#define WINT_MIN				0UL
#define WINT_MAX				UINT32_MAX

_C_STD_END

#endif

#ifdef _STD_USING
using std::int8_t; using std::uint8_t;
using std::int16_t; using std::uint16_t;
using std::int32_t; using std::uint32_t;
using std::int64_t; using std::uint64_t;
using std::intptr_t; using std::uintptr_t;
using std::intmax_t; using std::uintmax_t;
#if defined(__EXT_QNX)
using std::intleast8_t; using std::uintleast8_t;
using std::intleast16_t; using std::uintleast16_t;
using std::intleast32_t; using std::uintleast32_t;
using std::intleast64_t; using std::uintleast64_t;
using std::intfast8_t; using std::uintfast8_t;
using std::intfast16_t; using std::uintfast16_t;
using std::intfast32_t; using std::uintfast32_t;
using std::intfast64_t; using std::uintfast64_t;
#endif
using std::int_least8_t; using std::uint_least8_t;
using std::int_least16_t; using std::uint_least16_t;
using std::int_least32_t; using std::uint_least32_t;
using std::int_least64_t; using std::uint_least64_t;
using std::int_fast8_t; using std::uint_fast8_t;
using std::int_fast16_t; using std::uint_fast16_t;
using std::int_fast32_t; using std::uint_fast32_t;
using std::int_fast64_t; using std::uint_fast64_t;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("stdint.h $Rev: 153052 $"); */
