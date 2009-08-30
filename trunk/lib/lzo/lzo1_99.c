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




/* lzo1_99.c -- implementation of the LZO1-99 algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */



#include <lzoconf.h>
#if !defined(LZO_99_UNSUPPORTED)

#define COMPRESS_ID		99

#define DDBITS			3
#define CLEVEL			9


/***********************************************************************
//
************************************************************************/

#define LZO_NEED_DICT_H
#include "config1.h"


/***********************************************************************
// compression internal entry point.
************************************************************************/

static int
_lzo1_do_compress ( const lzo_byte *in,  lzo_uint  in_len,
                          lzo_byte *out, lzo_uint *out_len,
                          lzo_voidp wrkmem,
                          lzo_compress_t func )
{
	int r;

	/* don't try to compress a block that's too short */
	if (in_len <= 0)
	{
		*out_len = 0;
		r = LZO_E_OK;
	}
	else if (in_len <= MIN_LOOKAHEAD + 1)
	{
#if defined(LZO_RETURN_IF_NOT_COMPRESSIBLE)
		*out_len = 0;
		r = LZO_E_NOT_COMPRESSIBLE;
#else
		*out_len = STORE_RUN(out,in,in_len) - out;
		r = (*out_len > in_len) ? LZO_E_OK : LZO_E_ERROR;
#endif
	}
	else
		r = func(in,in_len,out,out_len,wrkmem);

	return r;
}


/***********************************************************************
//
************************************************************************/

#if !defined(COMPRESS_ID)
#define COMPRESS_ID		_LZO_ECONCAT2(DD_BITS,CLEVEL)
#endif


#define LZO_CODE_MATCH_INCLUDE_FILE		"lzo1_cm.ch"
#include "lzo1b_c.ch"


/***********************************************************************
//
************************************************************************/

#define LZO_COMPRESS \
	_LZO_ECONCAT3(lzo1_,COMPRESS_ID,_compress)

#define LZO_COMPRESS_FUNC \
	_LZO_ECONCAT3(_lzo1_,COMPRESS_ID,_compress_func)


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(int)
LZO_COMPRESS ( const lzo_byte *in,  lzo_uint  in_len,
                     lzo_byte *out, lzo_uint *out_len,
                     lzo_voidp wrkmem )
{
#if defined(__LZO_QUERY_COMPRESS)
	if (__LZO_IS_COMPRESS_QUERY(in,in_len,out,out_len,wrkmem))
		return __LZO_QUERY_COMPRESS(in,in_len,out,out_len,wrkmem,D_SIZE,lzo_sizeof(lzo_dict_t));
#endif

	return _lzo1_do_compress(in,in_len,out,out_len,wrkmem,do_compress);
}

#endif

/*
vi:ts=4
*/
