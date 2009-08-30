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
 *  gulliver.h - Handles endian operations
 *
 * "It is computed that eleven Thousand Persons have, at several Times,
 * suffered Death, rather than submit to break their Eggs at the
 * smaller End. Many hundred large Volumes have been published upon
 * this Controversy ...
 *                             Jonathan Swift, Gulliver's Travels
 *
 *

 */

#ifndef _GULLIVER_H_INCLUDED
#define _GULLIVER_H_INCLUDED

#ifdef __SOLARIS__
#include <lib/compat.h>
#else
#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif
#endif

#ifdef __QNXNTO__
#ifndef __CPUINLINE_H_INCLUDED
#include _NTO_HDR_(sys/cpuinline.h)
#endif
#endif

#ifdef __CPU_ENDIAN_RET16
#define ENDIAN_RET16(__x)		__cpu_endian_ret16(__x)
#else
#define ENDIAN_RET16(__x)		((((__x) >> 8) & 0xff) | \
								(((__x) & 0xff) << 8))
#endif

#ifdef __CPU_ENDIAN_RET32
#define ENDIAN_RET32(__x)		__cpu_endian_ret32(__x)
#else
#define ENDIAN_RET32(__x)		((((__x) >> 24) & 0xff) | \
								(((__x) >> 8) & 0xff00) | \
								(((__x) & 0xff00) << 8) | \
								(((__x) & 0xff) << 24))
#endif

#ifdef __CPU_ENDIAN_RET64
#define ENDIAN_RET64(__x)		__cpu_endian_ret64(__x)
#else
#define ENDIAN_RET64(__x)		((((__x) >> 56) & 0xff) | \
								 (((__x) >> 40) & 0xff00) | \
								 (((__x) >> 24) & 0xff0000) | \
								 (((__x) >>  8) & 0xff000000) | \
								 (((__x) & 0xff000000) <<  8) | \
								 (((__x) & 0xff0000) << 24) | \
								 (((__x) & 0xff00) << 40) | \
								 (((__x) & 0xff) << 56))
#endif

#ifdef __CPU_ENDIAN_SWAP16
#define ENDIAN_SWAP16(__x)		__cpu_endian_swap16(__x)
#else
#define ENDIAN_SWAP16(__x)		(*(_Uint16t *)(__x) = ENDIAN_RET16(*(_Uint16t *)(__x)))
#endif

#ifdef __CPU_ENDIAN_SWAP32
#define ENDIAN_SWAP32(__x)		__cpu_endian_swap32(__x)
#else
#define ENDIAN_SWAP32(__x)		(*(_Uint32t *)(__x) = ENDIAN_RET32(*(_Uint32t *)(__x)))
#endif

#ifdef __CPU_ENDIAN_SWAP64
#define ENDIAN_SWAP64(__x)		__cpu_endian_swap64(__x)
#else
#define ENDIAN_SWAP64(__x)		(*(_Uint64t *)(__x) = ENDIAN_RET64(*(_Uint64t *)(__x)))
#endif

#if defined(__LITTLEENDIAN__)
#define ENDIAN_STRINGNAME		"le"

#define ENDIAN_LE16(__x)		(__x)
#define ENDIAN_LE32(__x)		(__x)
#define ENDIAN_LE64(__x)		(__x)
#define ENDIAN_BE16(__x)		ENDIAN_RET16(__x)
#define ENDIAN_BE32(__x)		ENDIAN_RET32(__x)
#define ENDIAN_BE64(__x)		ENDIAN_RET64(__x)

#ifndef __CPU_UNALIGNED_RET16
#define UNALIGNED_RET16(__p)	(((_Uint8t volatile *)(__p))[0] | \
								(((_Uint8t volatile *)(__p))[1] << 8))
#endif

#ifndef __CPU_UNALIGNED_RET32
#define UNALIGNED_RET32(__p)	(((_Uint8t volatile *)(__p))[0] | \
								(((_Uint8t volatile *)(__p))[1] << 8) | \
								(((_Uint8t volatile *)(__p))[2] << 16) | \
								(((_Uint8t volatile *)(__p))[3] << 24))
#endif

#ifndef __CPU_UNALIGNED_RET64
#define UNALIGNED_RET64(__p)	(((_Uint8t volatile *)(__p))[0] | \
								(((_Uint8t volatile *)(__p))[1] << 8) | \
								(((_Uint8t volatile *)(__p))[2] << 16) | \
								((unsigned)(((_Uint8t volatile *)(__p))[3]) << 24) | \
								(((_Uint64t)((_Uint8t volatile *)(__p))[4]) << 32) | \
								(((_Uint64t)((_Uint8t volatile *)(__p))[5]) << 40) | \
								(((_Uint64t)((_Uint8t volatile *)(__p))[6]) << 48) | \
								(((_Uint64t)((_Uint8t volatile *)(__p))[7]) << 56))
#endif

#ifndef __CPU_UNALIGNED_PUT16
#define UNALIGNED_PUT16(__p,__x) (((_Uint8t volatile *)(__p))[0] = (__x) & 0xff, \
								((_Uint8t volatile *)(__p))[1] = ((__x) >> 8) & 0xff)
#endif

#ifndef __CPU_UNALIGNED_PUT32
#define UNALIGNED_PUT32(__p,__x) (((_Uint8t volatile *)(__p))[0] = (__x) & 0xff, \
								((_Uint8t volatile *)(__p))[1] = ((__x) >> 8) & 0xff, \
								((_Uint8t volatile *)(__p))[2] = ((__x) >> 16) & 0xff, \
								((_Uint8t volatile *)(__p))[3] = ((__x) >> 24) & 0xff)
#endif

#ifndef __CPU_UNALIGNED_PUT64
#define UNALIGNED_PUT64(__p,__x) (((_Uint8t volatile *)(__p))[0] = (__x) & 0xff, \
								((_Uint8t volatile *)(__p))[1] = ((__x) >> 8) & 0xff, \
								((_Uint8t volatile *)(__p))[2] = ((__x) >> 16) & 0xff, \
								((_Uint8t volatile *)(__p))[3] = ((__x) >> 24) & 0xff, \
								((_Uint8t volatile *)(__p))[4] = ((__x) >> 32) & 0xff, \
								((_Uint8t volatile *)(__p))[5] = ((__x) >> 40) & 0xff, \
								((_Uint8t volatile *)(__p))[6] = ((__x) >> 48) & 0xff, \
								((_Uint8t volatile *)(__p))[7] = ((__x) >> 56) & 0xff)
#endif

#elif defined(__BIGENDIAN__)
#define ENDIAN_STRINGNAME		"be"

#define ENDIAN_LE16(__x)		ENDIAN_RET16(__x)
#define ENDIAN_LE32(__x)		ENDIAN_RET32(__x)
#define ENDIAN_LE64(__x)		ENDIAN_RET64(__x)
#define ENDIAN_BE16(__x)		(__x)
#define ENDIAN_BE32(__x)		(__x)
#define ENDIAN_BE64(__x)		(__x)

#ifndef __CPU_UNALIGNED_RET16
#define UNALIGNED_RET16(__p)	((((_Uint8t volatile *)(__p))[0] << 8) | \
								((_Uint8t volatile *)(__p))[1])
#endif

#ifndef __CPU_UNALIGNED_RET32
#define UNALIGNED_RET32(__p)	((((_Uint8t volatile *)(__p))[0] << 24) | \
								(((_Uint8t volatile *)(__p))[1] << 16) | \
								(((_Uint8t volatile *)(__p))[2] << 8) | \
								((_Uint8t volatile *)(__p))[3])
#endif

#ifndef __CPU_UNALIGNED_RET64
#define UNALIGNED_RET64(__p)	(((_Uint8t volatile *)(__p))[7] | \
								(((_Uint8t volatile *)(__p))[6] << 8) | \
								(((_Uint8t volatile *)(__p))[5] << 16) | \
								((unsigned)(((_Uint8t volatile *)(__p))[4]) << 24) | \
								(((_Uint64t)((_Uint8t volatile *)(__p))[3]) << 32) | \
								(((_Uint64t)((_Uint8t volatile *)(__p))[2]) << 40) | \
								(((_Uint64t)((_Uint8t volatile *)(__p))[1]) << 48) | \
								(((_Uint64t)((_Uint8t volatile *)(__p))[0]) << 56))
#endif

#ifndef __CPU_UNALIGNED_PUT16
#define UNALIGNED_PUT16(__p,__x) (((_Uint8t volatile *)(__p))[0] = ((__x) >> 8) & 0xff, \
								((_Uint8t volatile *)(__p))[1] = (__x) & 0xff)
#endif

#ifndef __CPU_UNALIGNED_PUT32
#define UNALIGNED_PUT32(__p,__x) (((_Uint8t volatile *)(__p))[0] = ((__x) >> 24) & 0xff, \
								((_Uint8t volatile *)(__p))[1] = ((__x) >> 16) & 0xff, \
								((_Uint8t volatile *)(__p))[2] = ((__x) >> 8) & 0xff, \
								((_Uint8t volatile *)(__p))[3] = (__x) & 0xff)
#endif

#ifndef __CPU_UNALIGNED_PUT64
#define UNALIGNED_PUT64(__p,__x) (((_Uint8t volatile *)(__p))[7] = (__x) & 0xff, \
								((_Uint8t volatile *)(__p))[6] = ((__x) >> 8) & 0xff, \
								((_Uint8t volatile *)(__p))[5] = ((__x) >> 16) & 0xff, \
								((_Uint8t volatile *)(__p))[4] = ((__x) >> 24) & 0xff, \
								((_Uint8t volatile *)(__p))[3] = ((__x) >> 32) & 0xff, \
								((_Uint8t volatile *)(__p))[2] = ((__x) >> 40) & 0xff, \
								((_Uint8t volatile *)(__p))[1] = ((__x) >> 48) & 0xff, \
								((_Uint8t volatile *)(__p))[0] = ((__x) >> 56) & 0xff)
#endif
#else
#error ENDIAN Not defined for system
#endif

#ifdef __CPU_UNALIGNED_RET16
#define UNALIGNED_RET16(__p)	__cpu_unaligned_ret16(__p)
#endif
#ifdef __CPU_UNALIGNED_RET32
#define UNALIGNED_RET32(__p)	__cpu_unaligned_ret32(__p)
#endif
#ifdef __CPU_UNALIGNED_RET64
#define UNALIGNED_RET64(__p)	__cpu_unaligned_ret64(__p)
#endif
#ifdef __CPU_UNALIGNED_PUT16
#define UNALIGNED_PUT16(__p,__x) __cpu_unaligned_put16((__p),(__x))
#endif
#ifdef __CPU_UNALIGNED_PUT32
#define UNALIGNED_PUT32(__p,__x) __cpu_unaligned_put32((__p),(__x))
#endif
#ifdef __CPU_UNALIGNED_PUT64
#define UNALIGNED_PUT64(__p,__x) __cpu_unaligned_put64((__p),(__x))
#endif

#if 0
#define ENDIAN_CHKMSG(info)	((info)->flags & _NTO_MI_ENDIAN_DIFF)
#else
#define ENDIAN_CHKMSG(info) (0)
#endif

__BEGIN_DECLS

#if 0
extern int endian_swap(void *__data, void *__instr);
#else
#define endian_swap(__x, __y)	(0)
#endif

__END_DECLS

#endif

/* __SRCVERSION("gulliver.h $Rev: 153052 $"); */
