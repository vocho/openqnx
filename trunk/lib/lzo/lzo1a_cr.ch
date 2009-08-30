/*
Copyright 2002, QNX Software Systems Ltd. Unpublished Work All Rights
Reserved.

 
This source code contains confidential information of QNX Software Systems
Ltd. (QSSL). Any use, reproduction, modification, disclosure, distribution
or transfer of this software, or any software which includes or is based
upon any of this code, is only permitted under the terms of the QNX
Confidential Source License version 1.0 (see licensing.qnx.com for details)
or as otherwise expressly authorized by a written license agreement from
QSSL. For more information, please email licensing@qnx.com.
*/
/* lzo1a_cr.ch -- literal run handling for the the LZO1A algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the LZO package and is subject
   to change.
 */


#ifndef __LZO_LRUN_H
#define __LZO_LRUN_H


/***********************************************************************
// code a literal run
************************************************************************/

static lzo_byte *
store_run(lzo_byte * const oo, const lzo_byte * const ii, lzo_uint r_len)
{
#if defined(LZO_OPTIMIZE_GNUC_i386)
	register lzo_byte *op __asm__("%edi");
	register const lzo_byte *ip __asm__("%esi");
	register lzo_uint t __asm__("%ecx");
#else
	register lzo_byte *op;
	register const lzo_byte *ip;
	register lzo_uint t;
#endif

	op = oo;
	ip = ii;
	assert(r_len > 0);

	/* code a long R0 run */
	if (r_len >= 512)
	{
		unsigned r_bits = 6;		/* 256 << 6 == 16384 */
		lzo_uint tt = 32768u;

		while (r_len >= (t = tt))
		{
			r_len -= t;
			*op++ = 0; *op++ = (R0MAX - R0MIN);
			MEMCPY8_DS(op, ip, t);
			LZO_STATS(lzo_stats->r0long_runs++);
		}
		tt >>= 1;
		do {
			if (r_len >= (t = tt))
			{
				r_len -= t;
				*op++ = 0; *op++ = LZO_BYTE((R0FAST - R0MIN) + r_bits);
				MEMCPY8_DS(op, ip, t);
				LZO_STATS(lzo_stats->r0long_runs++);
			}
			tt >>= 1;
		} while (--r_bits > 0);
	}
	assert(r_len < 512);

	while (r_len >= (t = R0FAST))
	{
		r_len -= t;
		*op++ = 0; *op++ = (R0FAST - R0MIN);
		MEMCPY8_DS(op, ip, t);
		LZO_STATS(lzo_stats->r0fast_runs++);
	}

	t = r_len;
	if (t >= R0MIN)
	{
		/* code a short R0 run */
		*op++ = 0; *op++ = LZO_BYTE(t - R0MIN);
		MEMCPY_DS(op, ip, t);
		LZO_STATS(lzo_stats->r0short_runs++);
	}
	else if (t > 0)
	{
		/* code a short literal run */
		LZO_STATS(lzo_stats->lit_runs++);
		LZO_STATS(lzo_stats->lit_run[t]++);
		*op++ = LZO_BYTE(t);
		MEMCPY_DS(op, ip, t);
	}

	return op;
}



#endif /* already included */

/*
vi:ts=4
*/

