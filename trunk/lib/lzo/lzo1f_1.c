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




/* lzo1f_1.c -- implementation of the LZO1F-1 compression algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#include <lzo1f.h>
#include "lzo_conf.h"


/***********************************************************************
//
************************************************************************/

#define M2_MAX_OFFSET	0x0800
#define M3_MAX_OFFSET	0x3fff
#define M3_MARKER		224


#ifndef LZO_HASH
#define LZO_HASH		LZO_HASH_LZO_INCREMENTAL_A
#endif
#define D_BITS			14
#define D_INDEX1(d,p)	d = DM((0x21*DX3(p,5,5,6)) >> 5)
#define D_INDEX2(d,p)	d = (d & (D_MASK & 0x7ff)) ^ (D_HIGH | 0x1f)
#include "lzo_dict.h"


/***********************************************************************
// compress a block of data.
************************************************************************/

static
int do_compress          ( const lzo_byte *in , lzo_uint  in_len,
                                 lzo_byte *out, lzo_uint *out_len,
                                 lzo_voidp wrkmem )
{
#if 0 && defined(__GNUC__) && defined(__i386__)
	register const lzo_byte *ip __asm__("%esi");
#else
	register const lzo_byte *ip;
#endif
	lzo_byte *op;
	const lzo_byte * const in_end = in + in_len;
	const lzo_byte * const ip_end = in + in_len - 9;
	const lzo_byte *ii;
	lzo_dict_p const dict = (lzo_dict_p) wrkmem;

	op = out;
	ip = in;
	ii = ip;

	ip++;
	for (;;)
	{
#if 0 && defined(__GNUC__) && defined(__i386__)
		register const lzo_byte *m_pos __asm__("%edi");
#else
		register const lzo_byte *m_pos;
#endif
		lzo_moff_t m_off;
		lzo_uint m_len;
		lzo_uint dindex;
		lzo_uint lit;

		DINDEX1(dindex,ip);
		GINDEX(m_pos,m_off,dict,dindex,in);
		if (LZO_CHECK_MPOS_NON_DET(m_pos,m_off,in,ip,M3_MAX_OFFSET))
			goto literal;
#if 1
		if (m_off <= M2_MAX_OFFSET || m_pos[3] == ip[3])
			goto try_match;
		DINDEX2(dindex,ip);
#endif
		GINDEX(m_pos,m_off,dict,dindex,in);
		if (LZO_CHECK_MPOS_NON_DET(m_pos,m_off,in,ip,M3_MAX_OFFSET))
			goto literal;
		if (m_off <= M2_MAX_OFFSET || m_pos[3] == ip[3])
			goto try_match;
		goto literal;


try_match:
#if 0 && defined(LZO_UNALIGNED_OK_2)
		if (* (const lzo_ushortp) m_pos != * (const lzo_ushortp) ip)
#else
		if (m_pos[0] != ip[0] || m_pos[1] != ip[1])
#endif
		{
		}
		else
		{
			if (m_pos[2] == ip[2])
			{
				m_pos += 3;
#if 0
				if (m_off <= M2_MAX_OFFSET)
					goto match;
				if (lit <= 3)
					goto match;
				if (lit == 3)			/* better compression, but slower */
				{
					assert(op - 2 > out); op[-2] |= LZO_BYTE(3);
					*op++ = *ii++; *op++ = *ii++; *op++ = *ii++;
					goto code_match;
				}
				if (*m_pos == ip[3])
#endif
					goto match;
			}
		}


	/* a literal */
literal:
		UPDATE_I(dict,0,dindex,ip,in);
		if (++ip >= ip_end)
			break;
		continue;


	/* a match */
match:
		UPDATE_I(dict,0,dindex,ip,in);
		/* store current literal run */
		lit = (lzo_uint)(ip - ii);
		if (lit > 0)
		{
			register lzo_uint t = lit;

			if (t < 4 && op > out)
				op[-2] |= LZO_BYTE(t);
			else if (t <= 31)
				*op++ = LZO_BYTE(t);
			else
			{
				register lzo_uint tt = t - 31;

				*op++ = 0;
				while (tt > 255)
				{
					tt -= 255;
					*op++ = 0;
				}
				assert(tt > 0);
				*op++ = LZO_BYTE(tt);
			}
			do *op++ = *ii++; while (--t > 0);
		}
		assert(ii == ip);


		/* code the match */
		ip += 3;
		if (*m_pos++ != *ip++ || *m_pos++ != *ip++ || *m_pos++ != *ip++ ||
		    *m_pos++ != *ip++ || *m_pos++ != *ip++ || *m_pos++ != *ip++)
		{
			--ip;
			m_len = ip - ii;
			assert(m_len >= 3); assert(m_len <= 8);

			if (m_off <= M2_MAX_OFFSET)
			{
				m_off -= 1;
				*op++ = LZO_BYTE(((m_len - 2) << 5) | ((m_off & 7) << 2));
				*op++ = LZO_BYTE(m_off >> 3);
			}
			else if (m_len == 3 && m_off <= 2*M2_MAX_OFFSET && lit > 0)
			{
				m_off -= 1;
				/* m_off -= M2_MAX_OFFSET; */
				*op++ = LZO_BYTE(((m_off & 7) << 2));
				*op++ = LZO_BYTE(m_off >> 3);
			}
			else
			{
				*op++ = LZO_BYTE(M3_MARKER | (m_len - 2));
				*op++ = LZO_BYTE((m_off & 63) << 2);
				*op++ = LZO_BYTE(m_off >> 6);
			}
		}
		else
		{
			{
				const lzo_byte *end;
				end = in_end;
				while (ip < end && *m_pos == *ip)
					m_pos++, ip++;
				m_len = (ip - ii);
			}
			assert(m_len >= 3);

			if (m_len <= 33)
				*op++ = LZO_BYTE(M3_MARKER | (m_len - 2));
			else
			{
				m_len -= 33;
				*op++ = M3_MARKER | 0;
				while (m_len > 255)
				{
					m_len -= 255;
					*op++ = 0;
				}
				assert(m_len > 0);
				*op++ = LZO_BYTE(m_len);
			}
			*op++ = LZO_BYTE((m_off & 63) << 2);
			*op++ = LZO_BYTE(m_off >> 6);
		}

		ii = ip;
		if (ip >= ip_end)
			break;
	}


	/* store final literal run */
	if (in_end - ii > 0)
	{
		register lzo_uint t = in_end - ii;

		if (t < 4 && op > out)
			op[-2] |= LZO_BYTE(t);
		else if (t <= 31)
			*op++ = LZO_BYTE(t);
		else
		{
			register lzo_uint tt = t - 31;

			*op++ = 0;
			while (tt > 255)
			{
				tt -= 255;
				*op++ = 0;
			}
			assert(tt > 0);
			*op++ = LZO_BYTE(tt);
		}
		do *op++ = *ii++; while (--t > 0);
	}

	*out_len = op - out;
	return LZO_E_OK;
}


/***********************************************************************
// public entry point
************************************************************************/

LZO_PUBLIC(int)
lzo1f_1_compress ( const lzo_byte *in , lzo_uint  in_len,
                         lzo_byte *out, lzo_uint *out_len,
                         lzo_voidp wrkmem )
{
	lzo_byte *op = out;
	int r = LZO_E_OK;

#if defined(__LZO_QUERY_COMPRESS)
	if (__LZO_IS_COMPRESS_QUERY(in,in_len,out,out_len,wrkmem))
		return __LZO_QUERY_COMPRESS(in,in_len,out,out_len,wrkmem,D_SIZE,lzo_sizeof(lzo_dict_t));
#endif

	if (in_len <= 0)
		*out_len = 0;
	else if (in_len <= 10)
	{
		*op++ = LZO_BYTE(in_len);
		do *op++ = *in++; while (--in_len > 0);
		*out_len = op - out;
	}
	else
		r = do_compress(in,in_len,out,out_len,wrkmem);

	if (r == LZO_E_OK)
	{
		op = out + *out_len;
		*op++ = M3_MARKER | 1;
		*op++ = 0;
		*op++ = 0;
		*out_len += 3;
	}

	return r;
}


/*
vi:ts=4
*/

