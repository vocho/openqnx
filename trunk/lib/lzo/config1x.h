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




/* config1x.h -- configuration for the LZO1X algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */


#ifndef __LZO_CONFIG1X_H
#define __LZO_CONFIG1X_H

#if !defined(LZO1X) && !defined(LZO1Y) && !defined(LZO1Z)
#  define LZO1X
#endif

#if !defined(__LZO_IN_MINILZO)
#include <lzo1x.h>
#endif
#include "lzo_conf.h"
#include "lzo_util.h"


/***********************************************************************
//
************************************************************************/

#define LZO_EOF_CODE
#undef LZO_DETERMINISTIC

#define M1_MAX_OFFSET	0x0400
#ifndef M2_MAX_OFFSET
#define M2_MAX_OFFSET	0x0800
#endif
#define M3_MAX_OFFSET	0x4000
#define M4_MAX_OFFSET	0xbfff

#define MX_MAX_OFFSET	(M1_MAX_OFFSET + M2_MAX_OFFSET)

#define M1_MIN_LEN		2
#define M1_MAX_LEN		2
#define M2_MIN_LEN		3
#ifndef M2_MAX_LEN
#define M2_MAX_LEN		8
#endif
#define M3_MIN_LEN		3
#define M3_MAX_LEN		33
#define M4_MIN_LEN		3
#define M4_MAX_LEN		9

#define M1_MARKER		0
#define M2_MARKER		64
#define M3_MARKER		32
#define M4_MARKER		16


/***********************************************************************
//
************************************************************************/

#ifndef MIN_LOOKAHEAD
#define MIN_LOOKAHEAD		(M2_MAX_LEN + 1)
#endif

#if defined(LZO_NEED_DICT_H)

#ifndef LZO_HASH
#define LZO_HASH			LZO_HASH_LZO_INCREMENTAL_B
#endif
#define DL_MIN_LEN			M2_MIN_LEN
#include "lzo_dict.h"

#endif



#endif /* already included */

/*
vi:ts=4
*/

