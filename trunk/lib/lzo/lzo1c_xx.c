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




/* lzo1c_xx.c -- LZO1C compression public entry point

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#include "config1c.h"


/***********************************************************************
//
************************************************************************/

static const lzo_compress_t * const c_funcs [9] =
{
	&_lzo1c_1_compress_func,
	&_lzo1c_2_compress_func,
	&_lzo1c_3_compress_func,
	&_lzo1c_4_compress_func,
	&_lzo1c_5_compress_func,
	&_lzo1c_6_compress_func,
	&_lzo1c_7_compress_func,
	&_lzo1c_8_compress_func,
	&_lzo1c_9_compress_func
};


lzo_compress_t _lzo1c_get_compress_func(int clevel)
{
	const lzo_compress_t *f;

	if (clevel < LZO1C_BEST_SPEED || clevel > LZO1C_BEST_COMPRESSION)
	{
		if (clevel == LZO1C_DEFAULT_COMPRESSION)
			clevel = LZO1C_BEST_SPEED;
		else
			return 0;
	}
	f = c_funcs[clevel-1];
	assert(f && *f);
	return *f;
}


LZO_PUBLIC(int)
lzo1c_compress ( const lzo_byte *src, lzo_uint  src_len,
                       lzo_byte *dst, lzo_uint *dst_len,
                       lzo_voidp wrkmem,
                       int clevel )
{
	lzo_compress_t f;

	f = _lzo1c_get_compress_func(clevel);
	if (!f)
		return LZO_E_ERROR;
	return _lzo1c_do_compress(src,src_len,dst,dst_len,wrkmem,f);
}



/*
vi:ts=4
*/

