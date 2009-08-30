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




/* lzo1x_d3.c -- LZO1X decompression with preset dictionary

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#include "config1x.h"

#if 0
#undef NDEBUG
#include <assert.h>
#endif

#define LZO_TEST_DECOMPRESS_OVERRUN

#if 0 && defined(__linux__)
#  include <linux/string.h>
#endif


#define SLOW_MEMCPY(a,b,l)		{ do *a++ = *b++; while (--l > 0); }
#if 1 && defined(HAVE_MEMCPY)
#  if !defined(__LZO_DOS16) && !defined(__LZO_WIN16)
#    define FAST_MEMCPY(a,b,l)	{ memcpy(a,b,l); a += l; }
#  endif
#endif

#if 1 && defined(FAST_MEMCPY)
#  define DICT_MEMMOVE(op,m_pos,m_len,m_off) \
		if (m_off >= (m_len)) \
			FAST_MEMCPY(op,m_pos,m_len) \
		else \
			SLOW_MEMCPY(op,m_pos,m_len)
#else
#  define DICT_MEMMOVE(op,m_pos,m_len,m_off) \
		SLOW_MEMCPY(op,m_pos,m_len)
#endif

#if !defined(FAST_MEMCPY)
#  define FAST_MEMCPY	SLOW_MEMCPY
#endif


#define COPY_DICT_DICT(m_len,m_off) \
	{ \
		register const lzo_byte *m_pos; \
		m_off -= (lzo_moff_t) (op - out); assert(m_off > 0); \
		if (m_off > dict_len) goto lookbehind_overrun; \
		m_pos = dict_end - m_off; \
		if (m_len > m_off) \
		{ \
			m_len -= m_off; \
			FAST_MEMCPY(op,m_pos,m_off) \
			m_pos = out; \
			SLOW_MEMCPY(op,m_pos,m_len) \
		} \
		else \
			FAST_MEMCPY(op,m_pos,m_len) \
	}

#define COPY_DICT(m_len,m_off) \
	assert(m_len >= 2); assert(m_off > 0); assert(op > out); \
	if (m_off <= (lzo_moff_t) (op - out)) \
	{ \
		register const lzo_byte *m_pos = op - m_off; \
		DICT_MEMMOVE(op,m_pos,m_len,m_off) \
	} \
	else \
		COPY_DICT_DICT(m_len,m_off)




LZO_PUBLIC(int)
lzo1x_decompress_dict_safe ( const lzo_byte *in,  lzo_uint  in_len,
                                   lzo_byte *out, lzo_uint *out_len,
                                   lzo_voidp wrkmem /* NOT USED */,
                             const lzo_byte *dict, lzo_uint dict_len)


#include "lzo1x_d.ch"


/*
vi:ts=4
*/

