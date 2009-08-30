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




/* lzo_str.c -- string functions for the the LZO library

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#include "lzo_conf.h"


/***********************************************************************
// slow but portable <string.h> stuff, only used in assertions
************************************************************************/

LZO_PUBLIC(int)
lzo_memcmp(const lzo_voidp s1, const lzo_voidp s2, lzo_uint len)
{
#if (LZO_UINT_MAX <= SIZE_T_MAX) && defined(HAVE_MEMCMP)
	return memcmp(s1,s2,len);
#else
	const lzo_byte *p1 = (const lzo_byte *) s1;
	const lzo_byte *p2 = (const lzo_byte *) s2;
	int d;

	if (len > 0) do
	{
		d = *p1 - *p2;
		if (d != 0)
			return d;
		p1++;
		p2++;
	}
	while (--len > 0);
	return 0;
#endif
}


LZO_PUBLIC(lzo_voidp)
lzo_memcpy(lzo_voidp dest, const lzo_voidp src, lzo_uint len)
{
#if (LZO_UINT_MAX <= SIZE_T_MAX) && defined(HAVE_MEMCPY)
	return memcpy(dest,src,len);
#else
	lzo_byte *p1 = (lzo_byte *) dest;
	const lzo_byte *p2 = (const lzo_byte *) src;

	if (len <= 0 || p1 == p2)
		return dest;
	do
		*p1++ = *p2++;
	while (--len > 0);
	return dest;
#endif
}


LZO_PUBLIC(lzo_voidp)
lzo_memmove(lzo_voidp dest, const lzo_voidp src, lzo_uint len)
{
#if (LZO_UINT_MAX <= SIZE_T_MAX) && defined(HAVE_MEMMOVE)
	return memmove(dest,src,len);
#else
	lzo_byte *p1 = (lzo_byte *) dest;
	const lzo_byte *p2 = (const lzo_byte *) src;

	if (len <= 0 || p1 == p2)
		return dest;

	if (p1 < p2)
	{
		do
			*p1++ = *p2++;
		while (--len > 0);
	}
	else
	{
		p1 += len;
		p2 += len;
		do
			*--p1 = *--p2;
		while (--len > 0);
	}
	return dest;
#endif
}


LZO_PUBLIC(lzo_voidp)
lzo_memset(lzo_voidp s, int c, lzo_uint len)
{
#if (LZO_UINT_MAX <= SIZE_T_MAX) && defined(HAVE_MEMSET)
	return memset(s,c,len);
#else
	lzo_byte *p = (lzo_byte *) s;

	if (len > 0) do
		*p++ = LZO_BYTE(c);
	while (--len > 0);
	return s;
#endif
}


/*
vi:ts=4
*/
