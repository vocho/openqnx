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




/* config1b.h -- configuration for the LZO1B algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */


#ifndef __LZO_CONFIG1B_H
#define __LZO_CONFIG1B_H

#include <lzo1b.h>
#include "lzo_conf.h"


/***********************************************************************
// algorithm configuration
************************************************************************/

/* run bits (4 - 5) - the compressor and the decompressor
 * must use the same value. */
#if !defined(RBITS)
#  define RBITS		5
#endif

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
#if (RBITS < 4 || RBITS > 5)
#  error invalid RBITS
#endif
#if (DDBITS < 0 || DDBITS > 6)
#  error invalid DDBITS
#endif
#if (CLEVEL < 1 || CLEVEL > 9)
#  error invalid CLEVEL
#endif


/***********************************************************************
// internal configuration
************************************************************************/

/* add a special code so that the decompressor can detect the
 * end of the compressed data block (overhead is 3 bytes per block) */
#undef LZO_EOF_CODE
#define LZO_EOF_CODE

/* return -1 instead of copying if the data cannot be compressed */
#undef LZO_RETURN_IF_NOT_COMPRESSIBLE


/***********************************************************************
// algorithm internal configuration
************************************************************************/

/* choose the hashing strategy */
#ifndef LZO_HASH
#define LZO_HASH		LZO_HASH_LZO_INCREMENTAL_A
#endif

/* config */
#define R_BITS			RBITS
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

#include "lzo_util.h"
#include "lzo1b_de.h"
#include "stats1b.h"

#include "lzo1b_cc.h"


#endif /* already included */

/*
vi:ts=4
*/

