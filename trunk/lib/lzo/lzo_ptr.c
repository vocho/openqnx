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




/* lzo_ptr.c -- low-level pointer constructs

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#include "lzo_conf.h"


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(lzo_ptr_t)
__lzo_ptr_linear(const lzo_voidp ptr)
{
	lzo_ptr_t p;

#if defined(__LZO_DOS16) || defined(__LZO_WIN16)
    p = (((lzo_ptr_t)(_FP_SEG(ptr))) << (16 - __LZO_HShift)) + (_FP_OFF(ptr));
#else
    p = PTR_LINEAR(ptr);
#endif

	return p;
}


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(unsigned)
__lzo_align_gap(const lzo_voidp ptr, lzo_uint size)
{
	lzo_ptr_t p, s, n;

	assert(size > 0);

    p = __lzo_ptr_linear(ptr);
	s = (lzo_ptr_t) (size - 1);
#if 0
	assert((size & (size - 1)) == 0);
    n = ((p + s) & ~s) - p;
#else
	n = (((p + s) / size) * size) - p;
#endif

	assert((long)n >= 0);
	assert(n <= s);

	return (unsigned)n;
}



/*
vi:ts=4
*/
