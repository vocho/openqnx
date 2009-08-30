/*
 * This source code may contain confidential information of QNX Software
 * Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
 * modification, disclosure, distribution or transfer of this software,
 * or any software which includes or is based upon any of this code, is
 * prohibited unless expressly authorized by QSSL by written agreement. For
 * more information (including whether this source code file has been
 * published) please email licensing@qnx.com.
 */

/* ucl_util.c -- utilities for the UCL library

   This file is part of the UCL real-time data compression library.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   <markus.oberhumer@jk.uni-linz.ac.at>
   http://wildsau.idv.uni-linz.ac.at/mfx/ucl.html
 */


#include "ucl_conf.h"
#include "ucl_util.h"


/***********************************************************************
//
************************************************************************/

UCL_PUBLIC(ucl_bool)
ucl_assert(int expr)
{
    return (expr) ? 1 : 0;
}


/***********************************************************************
//
************************************************************************/

/* If you use the UCL library in a product, you *must* keep this
 * copyright string in the executable of your product.
.*/

const ucl_byte __ucl_copyright[] =
    "\n\n\n"
    "UCL real-time data compression library.\n"
    "Copyright (C) 1996, 1997, 1998, 1999, 2000 Markus Franz Xaver Johannes Oberhumer\n"
    "<markus.oberhumer@jk.uni-linz.ac.at>\n"
    "http://wildsau.idv.uni-linz.ac.at/mfx/ucl.html\n"
    "\n"
    "UCL version: v" UCL_VERSION_STRING ", " UCL_VERSION_DATE "\n"
    "UCL build date: " __DATE__ " " __TIME__ "\n\n"
    "UCL special compilation options:\n"
#ifdef __cplusplus
    " __cplusplus\n"
#endif
#if defined(__PIC__)
    " __PIC__\n"
#elif defined(__pic__)
    " __pic__\n"
#endif
#if (UINT_MAX < UCL_0xffffffffL)
    " 16BIT\n"
#endif
#if defined(__UCL_STRICT_16BIT)
    " __UCL_STRICT_16BIT\n"
#endif
#if (UINT_MAX > UCL_0xffffffffL)
    " UINT_MAX=" _UCL_MEXPAND(UINT_MAX) "\n"
#endif
#if (ULONG_MAX > UCL_0xffffffffL)
    " ULONG_MAX=" _UCL_MEXPAND(ULONG_MAX) "\n"
#endif
#if defined(UCL_BYTE_ORDER)
    " UCL_BYTE_ORDER=" _UCL_MEXPAND(UCL_BYTE_ORDER) "\n"
#endif
#if defined(UCL_UNALIGNED_OK_2)
    " UCL_UNALIGNED_OK_2\n"
#endif
#if defined(UCL_UNALIGNED_OK_4)
    " UCL_UNALIGNED_OK_4\n"
#endif
#if defined(UCL_ALIGNED_OK_4)
    " UCL_ALIGNED_OK_4\n"
#endif
#if defined(UCL_DICT_USE_PTR)
    " UCL_DICT_USE_PTR\n"
#endif
#if defined(__UCL_QUERY_COMPRESS)
    " __UCL_QUERY_COMPRESS\n"
#endif
#if defined(__UCL_QUERY_DECOMPRESS)
    " __UCL_QUERY_DECOMPRESS\n"
#endif
#if defined(__UCL_IN_MINIUCL)
    " __UCL_IN_MINIUCL\n"
#endif
    "\n\n"
/* RCS information */
    "$Id: UCL " UCL_VERSION_STRING " built " __DATE__ " " __TIME__
#if defined(__GNUC__) && defined(__VERSION__)
    " by gcc " __VERSION__
#elif defined(__BORLANDC__)
    " by Borland C " _UCL_MEXPAND(__BORLANDC__)
#elif defined(_MSC_VER)
    " by Microsoft C " _UCL_MEXPAND(_MSC_VER)
#elif defined(__PUREC__)
    " by Pure C " _UCL_MEXPAND(__PUREC__)
#elif defined(__SC__)
    " by Symantec C " _UCL_MEXPAND(__SC__)
#elif defined(__TURBOC__)
    " by Turbo C " _UCL_MEXPAND(__TURBOC__)
#elif defined(__WATCOMC__)
    " by Watcom C " _UCL_MEXPAND(__WATCOMC__)
#endif
    " $\n"
    "$Copyright: UCL (C) 1996, 1997, 1998, 1999, 2000 Markus Franz Xaver Johannes Oberhumer $\n";

UCL_PUBLIC(const ucl_byte *)
ucl_copyright(void)
{
    return __ucl_copyright;
}

UCL_PUBLIC(unsigned)
ucl_version(void)
{
    return UCL_VERSION;
}

UCL_PUBLIC(const char *)
ucl_version_string(void)
{
    return UCL_VERSION_STRING;
}

UCL_PUBLIC(const char *)
ucl_version_date(void)
{
    return UCL_VERSION_DATE;
}

UCL_PUBLIC(const ucl_charp)
_ucl_version_string(void)
{
    return UCL_VERSION_STRING;
}

UCL_PUBLIC(const ucl_charp)
_ucl_version_date(void)
{
    return UCL_VERSION_DATE;
}


/***********************************************************************
// adler32 checksum
// adapted from free code by Mark Adler <madler@alumni.caltech.edu>
// see http://www.cdrom.com/pub/infozip/zlib/
************************************************************************/

#define UCL_BASE 65521u /* largest prime smaller than 65536 */
#define UCL_NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define UCL_DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define UCL_DO2(buf,i)  UCL_DO1(buf,i); UCL_DO1(buf,i+1);
#define UCL_DO4(buf,i)  UCL_DO2(buf,i); UCL_DO2(buf,i+2);
#define UCL_DO8(buf,i)  UCL_DO4(buf,i); UCL_DO4(buf,i+4);
#define UCL_DO16(buf,i) UCL_DO8(buf,i); UCL_DO8(buf,i+8);

UCL_PUBLIC(ucl_uint32)
ucl_adler32(ucl_uint32 adler, const ucl_byte *buf, ucl_uint len)
{
    ucl_uint32 s1 = adler & 0xffff;
    ucl_uint32 s2 = (adler >> 16) & 0xffff;
    int k;

    if (buf == NULL)
        return 1;

    while (len > 0)
    {
        k = len < UCL_NMAX ? (int) len : UCL_NMAX;
        len -= k;
        if (k >= 16) do
        {
            UCL_DO16(buf,0);
            buf += 16;
            k -= 16;
        } while (k >= 16);
        if (k != 0) do
        {
            s1 += *buf++;
            s2 += s1;
        } while (--k > 0);
        s1 %= UCL_BASE;
        s2 %= UCL_BASE;
    }
    return (s2 << 16) | s1;
}


/*
vi:ts=4:et
*/
