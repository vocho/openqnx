/*
 * $QNXLicenseC:
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





#define LZO_NEED_DICT_H
#include "config1b.h"


#if !defined(COMPRESS_ID)
#define COMPRESS_ID		_LZO_ECONCAT2(DD_BITS,CLEVEL)
#endif


#include "lzo1b_c.ch"


/***********************************************************************
//
************************************************************************/

#define LZO_COMPRESS \
	_LZO_ECONCAT3(lzo1b_,COMPRESS_ID,_compress)

#define LZO_COMPRESS_FUNC \
	_LZO_ECONCAT3(_lzo1b_,COMPRESS_ID,_compress_func)



/***********************************************************************
//
************************************************************************/

const lzo_compress_t LZO_COMPRESS_FUNC = do_compress;

LZO_PUBLIC(int)
LZO_COMPRESS ( const lzo_byte *in,  lzo_uint  in_len,
                     lzo_byte *out, lzo_uint *out_len,
                     lzo_voidp wrkmem )
{
#if defined(__LZO_QUERY_COMPRESS)
	if (__LZO_IS_COMPRESS_QUERY(in,in_len,out,out_len,wrkmem))
		return __LZO_QUERY_COMPRESS(in,in_len,out,out_len,wrkmem,D_SIZE,lzo_sizeof(lzo_dict_t));
#endif

	return _lzo1b_do_compress(in,in_len,out,out_len,wrkmem,do_compress);
}

/*
vi:ts=4
*/
