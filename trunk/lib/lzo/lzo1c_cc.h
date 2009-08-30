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




/* lzo1c_cc.h -- definitions for the the LZO1C compression driver

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */


#ifndef __LZO1C_CC_H
#define __LZO1C_CC_H


/***********************************************************************
//
************************************************************************/

extern const lzo_compress_t _lzo1c_1_compress_func;
extern const lzo_compress_t _lzo1c_2_compress_func;
extern const lzo_compress_t _lzo1c_3_compress_func;
extern const lzo_compress_t _lzo1c_4_compress_func;
extern const lzo_compress_t _lzo1c_5_compress_func;
extern const lzo_compress_t _lzo1c_6_compress_func;
extern const lzo_compress_t _lzo1c_7_compress_func;
extern const lzo_compress_t _lzo1c_8_compress_func;
extern const lzo_compress_t _lzo1c_9_compress_func;

extern const lzo_compress_t _lzo1c_99_compress_func;


/***********************************************************************
//
************************************************************************/

LZO_EXTERN(lzo_byte *)
_lzo1c_store_run ( lzo_byte * const oo, const lzo_byte * const ii,
				   lzo_uint r_len);

#define STORE_RUN	_lzo1c_store_run


lzo_compress_t _lzo1c_get_compress_func(int clevel);

int _lzo1c_do_compress   ( const lzo_byte *in,  lzo_uint  in_len,
								 lzo_byte *out, lzo_uint *out_len,
								 lzo_voidp wrkmem,
								 lzo_compress_t func );


#endif /* already included */

/*
vi:ts=4
*/


