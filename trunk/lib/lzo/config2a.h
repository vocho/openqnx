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




/* config2a.h -- configuration for the LZO2A algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */


#ifndef __LZO_CONFIG2A_H
#define __LZO_CONFIG2A_H

#include <lzo2a.h>
#include "lzo_conf.h"

#include "lzo_util.h"


/***********************************************************************
// algorithm configuration
************************************************************************/

/* dictionary depth (0 - 6) - this only affects the compressor.
 * 0 is fastest, 6 is best compression ratio */
#if !defined(DDBITS)
#  define DDBITS	0
#endif

/* compression level (1 - 9) - this only affects the compressor.
 * 1 is fastest, 9 is best compression ratio */
#if !defined(CLEVEL)
#  define CLEVEL	1			/* fastest by default */
#endif


/* check configuration */
#if (DDBITS < 0 || DDBITS > 6)
#  error invalid DDBITS
#endif
#if (CLEVEL < 1 || CLEVEL > 9)
#  error invalid CLEVEL
#endif


/***********************************************************************
// internal configuration
************************************************************************/

#if 1
#define N		 	 8191			/* size of ring buffer */
#else
#define N		 	16383			/* size of ring buffer */
#endif

#define M1_MIN_LEN	2
#define M1_MAX_LEN	5
#define M2_MIN_LEN	3
#define M3_MIN_LEN	3


/* add a special code so that the decompressor can detect the
 * end of the compressed data block (overhead is 3 bytes per block) */
#undef LZO_EOF_CODE
#define LZO_EOF_CODE

/* return -1 instead of copying if the data cannot be compressed */
#undef LZO_RETURN_IF_NOT_COMPRESSIBLE

#undef LZO_DETERMINISTIC


/***********************************************************************
// algorithm internal configuration
************************************************************************/

/* choose the hashing strategy */
#ifndef LZO_HASH
#define LZO_HASH		LZO_HASH_LZO_INCREMENTAL_A
#endif

/* config */
#define DD_BITS			DDBITS
#ifndef D_BITS
#define D_BITS			14
#endif



/***********************************************************************
// optimization and debugging
************************************************************************/

/* Collect statistics */
#if 0 && !defined(LZO_COLLECT_STATS)
#  define LZO_COLLECT_STATS
#endif


/***********************************************************************
//
************************************************************************/

/* get bits */
#define _NEEDBITS \
	{ _NEEDBYTE; b |= ((lzo_uint32) _NEXTBYTE) << k; k += 8; assert(k <= 32); }
#define NEEDBITS(j)		{ assert((j) < 8); if (k < (j)) _NEEDBITS }

/* set bits */
#define SETBITS(j,x)	{ b |= (x) << k; k += (j); assert(k <= 32); }

/* access bits */
#define MASKBITS(j)		(b & ((((lzo_uint32)1 << (j)) - 1)))

/* drop bits */
#define DUMPBITS(j)		{ assert(k >= j); b >>= (j); k -= (j); }



#endif /* already included */

/*
vi:ts=4
*/

