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




/* io.c -- io functions

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#include "lzo_conf.h"

#if !defined(NO_STDIO_H)

#include <stdio.h>
#include <lzoutil.h>

#undef lzo_fread
#undef lzo_fwrite


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(lzo_uint)
lzo_fread(FILE *f, lzo_voidp s, lzo_uint len)
{
#if 1 && (LZO_UINT_MAX <= SIZE_T_MAX)
	return fread(s,1,len,f);
#else
	lzo_byte *p = (lzo_byte *) s;
	lzo_uint l = 0;
	size_t k;
	unsigned char *b;
	unsigned char buf[512];

	while (l < len)
	{
		k = len - l > sizeof(buf) ? sizeof(buf) : (size_t) (len - l);
		k = fread(buf,1,k,f);
		if (k <= 0)
			break;
		l += k;
		b = buf; do *p++ = *b++; while (--k > 0);
	}
	return l;
#endif
}


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(lzo_uint)
lzo_fwrite(FILE *f, const lzo_voidp s, lzo_uint len)
{
#if 1 && (LZO_UINT_MAX <= SIZE_T_MAX)
	return fwrite(s,1,len,f);
#else
	const lzo_byte *p = (const lzo_byte *) s;
	lzo_uint l = 0;
	size_t k, n;
	unsigned char *b;
	unsigned char buf[512];

	while (l < len)
	{
		k = len - l > sizeof(buf) ? sizeof(buf) : (size_t) (len - l);
		b = buf; n = k; do *b++ = *p++; while (--n > 0);
		k = fwrite(buf,1,k,f);
		if (k <= 0)
			break;
		l += k;
	}
	return l;
#endif
}


#endif /* !defined(NO_STDIO_H) */


/*
vi:ts=4
*/
