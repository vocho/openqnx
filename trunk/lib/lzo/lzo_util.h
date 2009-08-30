/*
 * $QNXtpLicenseC:
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




/* lzo_util.h -- utilities for the LZO library

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */


#ifndef __LZO_UTIL_H
#define __LZO_UTIL_H

#ifndef __LZO_CONF_H
#  include "lzo_conf.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
// fast memcpy that copies multiples of 8 byte chunks.
// len is the number of bytes.
// note: all parameters must be lvalues, len >= 8
//       dest and src advance, len is undefined afterwards
************************************************************************/

#if 1 && defined(HAVE_MEMCPY)
#if !defined(__LZO_DOS16) && !defined(__LZO_WIN16)

#define MEMCPY8_DS(dest,src,len) \
	memcpy(dest,src,len); \
	dest += len; \
	src += len

#endif
#endif


#if 0 && !defined(MEMCPY8_DS)

#define MEMCPY8_DS(dest,src,len) \
	{ do { \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
		len -= 8; \
	} while (len > 0); }

#endif


#if !defined(MEMCPY8_DS)

#define MEMCPY8_DS(dest,src,len) \
	{ register lzo_uint __l = (len) / 8; \
	do { \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
		*dest++ = *src++; \
	} while (--__l > 0); }

#endif


/***********************************************************************
// memcpy and pseudo-memmove
// len is the number of bytes.
// note: all parameters must be lvalues, len > 0
//       dest and src advance, len is undefined afterwards
************************************************************************/

#define MEMCPY_DS(dest,src,len) \
	do *dest++ = *src++; \
	while (--len > 0)

#define MEMMOVE_DS(dest,src,len) \
	do *dest++ = *src++; \
	while (--len > 0)


/***********************************************************************
// fast bzero that clears multiples of 8 pointers
// n is the number of pointers.
// note: n > 0
//       s and n are undefined afterwards
************************************************************************/

#if 0 && defined(LZO_OPTIMIZE_GNUC_i386)

#define BZERO8_PTR(s,l,n) \
__asm__ __volatile__( \
	"movl  %0,%%eax \n"             \
	"movl  %1,%%edi \n"             \
	"movl  %2,%%ecx \n"             \
	"cld \n"                        \
	"rep \n"                        \
	"stosl %%eax,(%%edi) \n"        \
	: /* no outputs */              \
	:"g" (0),"g" (s),"g" (n)	  	\
	:"eax","edi","ecx", "memory", "cc" \
)

#elif (LZO_UINT_MAX <= SIZE_T_MAX) && defined(HAVE_MEMSET)

#if 1
#define BZERO8_PTR(s,l,n)	memset((s),0,(lzo_uint)(l)*(n))
#else
#define BZERO8_PTR(s,l,n)	memset((lzo_voidp)(s),0,(lzo_uint)(l)*(n))
#endif

#else

#define BZERO8_PTR(s,l,n) \
	lzo_memset((lzo_voidp)(s),0,(lzo_uint)(l)*(n))

#endif


/***********************************************************************
// rotate (not used at the moment)
************************************************************************/

#if 0
#if defined(__GNUC__) && defined(__i386__)

unsigned char lzo_rotr8(unsigned char value, int shift);
extern __inline__ unsigned char lzo_rotr8(unsigned char value, int shift)
{
	unsigned char result;

	__asm__ __volatile__ ("movb %b1, %b0; rorb %b2, %b0"
                      	: "=a"(result) : "g"(value), "c"(shift));
	return result;
}

unsigned short lzo_rotr16(unsigned short value, int shift);
extern __inline__ unsigned short lzo_rotr16(unsigned short value, int shift)
{
	unsigned short result;

	__asm__ __volatile__ ("movw %b1, %b0; rorw %b2, %b0"
                      	: "=a"(result) : "g"(value), "c"(shift));
	return result;
}

#endif
#endif



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* already included */

/*
vi:ts=4
*/
