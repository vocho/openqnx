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




/* lzo_util.c -- utilities for the LZO library

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#include "lzo_conf.h"
#include "lzo_util.h"


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(lzo_bool)
lzo_assert(int expr)
{
	return (expr) ? 1 : 0;
}


/***********************************************************************
//
************************************************************************/

/* If you use the LZO library in a product, you *must* keep this
 * copyright string in the executable of your product.
.*/

const lzo_byte __lzo_copyright[] =
#if !defined(__LZO_IN_MINLZO)
	/* save space as some people want a really small decompressor */
    LZO_VERSION_STRING;
#else
	"\n\n\n"
	"LZO real-time data compression library.\n"
	"Copyright (C) 1996, 1997, 1998, 1999 Markus Franz Xaver Johannes Oberhumer\n"
	"<markus.oberhumer@jk.uni-linz.ac.at>\n"
	"http://wildsau.idv.uni-linz.ac.at/mfx/lzo.html\n"
	"\n"
	"LZO version: v" LZO_VERSION_STRING ", " LZO_VERSION_DATE "\n"
	"LZO build date: " __DATE__ " " __TIME__ "\n\n"
	"LZO special compilation options:\n"
#ifdef __cplusplus
	" __cplusplus\n"
#endif
#if defined(__PIC__)
	" __PIC__\n"
#elif defined(__pic__)
	" __pic__\n"
#endif
#if (UINT_MAX < LZO_0xffffffffL)
	" 16BIT\n"
#endif
#if defined(__LZO_STRICT_16BIT)
	" __LZO_STRICT_16BIT\n"
#endif
#if (UINT_MAX > LZO_0xffffffffL)
	" UINT_MAX=" _LZO_MEXPAND(UINT_MAX) "\n"
#endif
#if (ULONG_MAX > LZO_0xffffffffL)
	" ULONG_MAX=" _LZO_MEXPAND(ULONG_MAX) "\n"
#endif
#if defined(LZO_BYTE_ORDER)
	" LZO_BYTE_ORDER=" _LZO_MEXPAND(LZO_BYTE_ORDER) "\n"
#endif
#if defined(LZO_UNALIGNED_OK_2)
	" LZO_UNALIGNED_OK_2\n"
#endif
#if defined(LZO_UNALIGNED_OK_4)
	" LZO_UNALIGNED_OK_4\n"
#endif
#if defined(LZO_ALIGNED_OK_4)
	" LZO_ALIGNED_OK_4\n"
#endif
#if defined(LZO_DICT_USE_PTR)
	" LZO_DICT_USE_PTR\n"
#endif
#if defined(__LZO_QUERY_COMPRESS)
	" __LZO_QUERY_COMPRESS\n"
#endif
#if defined(__LZO_QUERY_DECOMPRESS)
	" __LZO_QUERY_DECOMPRESS\n"
#endif
#if defined(__LZO_IN_MINILZO)
	" __LZO_IN_MINILZO\n"
#endif
	"\n\n"
/* RCS information */
	"$Id: LZO " LZO_VERSION_STRING " built " __DATE__ " " __TIME__
#if defined(__GNUC__) && defined(__VERSION__)
	" by gcc " __VERSION__
#elif defined(__BORLANDC__)
	" by Borland C " _LZO_MEXPAND(__BORLANDC__)
#elif defined(_MSC_VER)
	" by Microsoft C " _LZO_MEXPAND(_MSC_VER)
#elif defined(__PUREC__)
	" by Pure C " _LZO_MEXPAND(__PUREC__)
#elif defined(__SC__)
	" by Symantec C " _LZO_MEXPAND(__SC__)
#elif defined(__TURBOC__)
	" by Turbo C " _LZO_MEXPAND(__TURBOC__)
#elif defined(__WATCOMC__)
	" by Watcom C " _LZO_MEXPAND(__WATCOMC__)
#endif
	" $\n"
	"$Copyright: LZO (C) 1996, 1997, 1998, 1999 Markus Franz Xaver Johannes Oberhumer $\n";
#endif

LZO_PUBLIC(const lzo_byte *)
lzo_copyright(void)
{
	return __lzo_copyright;
}

LZO_PUBLIC(unsigned)
lzo_version(void)
{
	return LZO_VERSION;
}

LZO_PUBLIC(const char *)
lzo_version_string(void)
{
	return LZO_VERSION_STRING;
}

LZO_PUBLIC(const char *)
lzo_version_date(void)
{
	return LZO_VERSION_DATE;
}

LZO_PUBLIC(const lzo_charp)
_lzo_version_string(void)
{
	return LZO_VERSION_STRING;
}

LZO_PUBLIC(const lzo_charp)
_lzo_version_date(void)
{
	return LZO_VERSION_DATE;
}


/***********************************************************************
// adler32 checksum
// adapted from free code by Mark Adler <madler@alumni.caltech.edu>
// see http://www.cdrom.com/pub/infozip/zlib/
************************************************************************/

#define LZO_BASE 65521u /* largest prime smaller than 65536 */
#define LZO_NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define LZO_DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define LZO_DO2(buf,i)  LZO_DO1(buf,i); LZO_DO1(buf,i+1);
#define LZO_DO4(buf,i)  LZO_DO2(buf,i); LZO_DO2(buf,i+2);
#define LZO_DO8(buf,i)  LZO_DO4(buf,i); LZO_DO4(buf,i+4);
#define LZO_DO16(buf,i) LZO_DO8(buf,i); LZO_DO8(buf,i+8);

LZO_PUBLIC(lzo_uint32)
lzo_adler32(lzo_uint32 adler, const lzo_byte *buf, lzo_uint len)
{
	lzo_uint32 s1 = adler & 0xffff;
	lzo_uint32 s2 = (adler >> 16) & 0xffff;
	int k;

	if (buf == NULL)
		return 1;

	while (len > 0)
	{
		k = len < LZO_NMAX ? (int) len : LZO_NMAX;
		len -= k;
		if (k >= 16) do
		{
			LZO_DO16(buf,0);
			buf += 16;
			k -= 16;
		} while (k >= 16);
		if (k != 0) do
		{
			s1 += *buf++;
			s2 += s1;
		} while (--k > 0);
		s1 %= LZO_BASE;
		s2 %= LZO_BASE;
	}
	return (s2 << 16) | s1;
}


/*
vi:ts=4
*/
