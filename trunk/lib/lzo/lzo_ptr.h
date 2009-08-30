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




/* lzo_ptr.h -- low-level pointer constructs

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */


#ifndef __LZO_PTR_H
#define __LZO_PTR_H

#ifdef __cplusplus
extern "C" {
#endif


/* This is the lowest part of the LZO library.
 * It deals with pointer representations at bit level.
 */


/***********************************************************************
// Includes
************************************************************************/

#if defined(__LZO_DOS16) || defined(__LZO_WIN16)
#  include <dos.h>
#  if 1 && defined(__WATCOMC__)
#    include <i86.h>
     __LZO_EXTERN_C unsigned char _HShift;
#    define __LZO_HShift	_HShift
#  elif 1 && defined(_MSC_VER)
     __LZO_EXTERN_C unsigned short __near _AHSHIFT;
#    define __LZO_HShift	((unsigned) &_AHSHIFT)
#  elif defined(__LZO_WIN16)
#    define __LZO_HShift	3
#  else
#    define __LZO_HShift	12
#  endif
#  if !defined(_FP_SEG) && defined(FP_SEG)
#    define _FP_SEG			FP_SEG
#  endif
#  if !defined(_FP_OFF) && defined(FP_OFF)
#    define _FP_OFF			FP_OFF
#  endif
#endif


/***********************************************************************
// Integral types
************************************************************************/

/* ptrdiff_t */
#if (UINT_MAX >= LZO_0xffffffffL)
   typedef ptrdiff_t        	lzo_ptrdiff_t;
#else
   typedef long             	lzo_ptrdiff_t;
#endif


/* Unsigned type that has *exactly* the same number of bits as a lzo_voidp */
#if !defined(__LZO_HAVE_PTR_T)
#  if defined(lzo_ptr_t)
#    define __LZO_HAVE_PTR_T
#  endif
#endif
#if !defined(__LZO_HAVE_PTR_T)
#  if defined(SIZEOF_CHAR_P) && defined(SIZEOF_UNSIGNED_LONG)
#    if (SIZEOF_CHAR_P == SIZEOF_UNSIGNED_LONG)
       typedef unsigned long  	lzo_ptr_t;
       typedef long           	lzo_sptr_t;
#      define __LZO_HAVE_PTR_T
#    endif
#  endif
#endif
#if !defined(__LZO_HAVE_PTR_T)
#  if defined(SIZEOF_CHAR_P) && defined(SIZEOF_UNSIGNED)
#    if (SIZEOF_CHAR_P == SIZEOF_UNSIGNED)
       typedef unsigned int   	lzo_ptr_t;
       typedef int            	lzo_sptr_t;
#      define __LZO_HAVE_PTR_T
#    endif
#  endif
#endif
#if !defined(__LZO_HAVE_PTR_T)
#  if defined(SIZEOF_CHAR_P) && defined(SIZEOF_UNSIGNED_SHORT)
#    if (SIZEOF_CHAR_P == SIZEOF_UNSIGNED_SHORT)
       typedef unsigned short 	lzo_ptr_t;
       typedef short          	lzo_sptr_t;
#      define __LZO_HAVE_PTR_T
#    endif
#  endif
#endif
#if !defined(__LZO_HAVE_PTR_T)
#  if defined(LZO_HAVE_CONFIG_H) || defined(SIZEOF_CHAR_P)
#    error "no suitable type for lzo_ptr_t"
#  else
     typedef unsigned long  	lzo_ptr_t;
     typedef long           	lzo_sptr_t;
#    define __LZO_HAVE_PTR_T
#  endif
#endif


/***********************************************************************
//
************************************************************************/

/* Always use the safe (=integral) version for pointer-comparisions.
 * The compiler should optimize away the additional casts anyway.
 *
 * Note that this only works if the representation and ordering
 * of the pointer and the integral is the same (at bit level).
 *
 * Most 16 bit compilers have their own view about pointers -
 * fortunately they don't care about comparing pointers
 * that are pointing to Nirvana.
 */

#if defined(__LZO_DOS16) || defined(__LZO_WIN16)
#define PTR(a)				((lzo_bytep) (a))
/* only need the low bits of the pointer -> offset is ok */
#define PTR_ALIGNED_4(a)	((_FP_OFF(a) & 3) == 0)
#define PTR_ALIGNED2_4(a,b)	(((_FP_OFF(a) | _FP_OFF(b)) & 3) == 0)
#else
#define PTR(a)				((lzo_ptr_t) (a))
#define PTR_LINEAR(a)		PTR(a)
#define PTR_ALIGNED_4(a)	((PTR_LINEAR(a) & 3) == 0)
#define PTR_ALIGNED_8(a)	((PTR_LINEAR(a) & 7) == 0)
#define PTR_ALIGNED2_4(a,b)	(((PTR_LINEAR(a) | PTR_LINEAR(b)) & 3) == 0)
#define PTR_ALIGNED2_8(a,b)	(((PTR_LINEAR(a) | PTR_LINEAR(b)) & 7) == 0)
#endif

#define PTR_LT(a,b)			(PTR(a) < PTR(b))
#define PTR_GE(a,b)			(PTR(a) >= PTR(b))
#define PTR_DIFF(a,b)		((lzo_ptrdiff_t) (PTR(a) - PTR(b)))


LZO_EXTERN(lzo_ptr_t)
__lzo_ptr_linear(const lzo_voidp ptr);


typedef union
{
	char			a_char;
	unsigned char	a_uchar;
	short			a_short;
	unsigned short	a_ushort;
	int				a_int;
	unsigned int	a_uint;
	long			a_long;
	unsigned long	a_ulong;
	lzo_int			a_lzo_int;
	lzo_uint		a_lzo_uint;
	lzo_int32		a_lzo_int32;
	lzo_uint32		a_lzo_uint32;
	ptrdiff_t		a_ptrdiff_t;
	lzo_ptrdiff_t	a_lzo_ptrdiff_t;
	lzo_ptr_t		a_lzo_ptr_t;
	char *			a_charp;
	lzo_bytep		a_lzo_bytep;
	lzo_bytepp		a_lzo_bytepp;
}
lzo_align_t;



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* already included */

/*
vi:ts=4
*/

