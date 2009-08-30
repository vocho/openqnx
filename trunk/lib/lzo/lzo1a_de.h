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




/* lzo1a_de.h -- definitions for the the LZO1A algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the LZO package and is subject
   to change.
 */


#ifndef __LZO_DEFS_H
#define __LZO_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
//
************************************************************************/

/*
	 Format of the marker byte


	 76543210
	 --------
	 00000000	a long literal run ('R0' run) - there are short and long R0 runs
	 000rrrrr	a short literal run with len r
	 mmmooooo	a short match (len = 2+m, o = offset low bits)
	 111ooooo	a long match (o = offset low bits)
*/


#define RSIZE	(1 << RBITS)
#define RMASK	(RSIZE - 1)

#define MBITS	(8 - OBITS)
#define MSIZE	(1 << MBITS)
#define MMASK	(MSIZE - 1)

#define OBITS	RBITS				/* offset and run-length use same bits */
#define OSIZE	(1 << OBITS)
#define OMASK	(OSIZE - 1)


/* additional bits for coding the length in a long match */
#define LBITS	8
#define LSIZE	(1 << LBITS)
#define LMASK	(LSIZE - 1)


/***********************************************************************
// some macros to improve readability
************************************************************************/

/* Minimum len of a match */
#define	MIN_MATCH			3
#define	THRESHOLD			(MIN_MATCH - 1)

/* Min-/Maximum len of a match coded in 2 bytes */
#define	MIN_MATCH_SHORT		(MIN_MATCH)
#define	MAX_MATCH_SHORT		(MIN_MATCH_SHORT + (MSIZE - 2) - 1)
/* why (MSIZE - 2) ? because 0 is used to mark runs,
 *                   and MSIZE-1 is used to mark a long match */

/* Min-/Maximum len of a match coded in 3 bytes */
#define	MIN_MATCH_LONG		(MAX_MATCH_SHORT + 1)
#define	MAX_MATCH_LONG		(MIN_MATCH_LONG + LSIZE - 1)

/* Min-/Maximum offset of a match */
#define	MIN_OFFSET			1
#define	MAX_OFFSET			(1 << (CHAR_BIT + OBITS))


/* R0 literal run (a long run) */

#define	R0MIN	(RSIZE)				/* Minimum len of R0 run of literals */
#define	R0MAX	(R0MIN + 255)		/* Maximum len of R0 run of literals */
#define	R0FAST	(R0MAX & ~7)		/* R0MAX aligned to 8 byte boundary */

#if (R0MAX - R0FAST != 7) || ((R0FAST & 7) != 0)
#  error something went wrong
#endif

/* 7 special codes from R0FAST+1 .. R0MAX
 * these codes mean long R0 runs with lengths
 * 512, 1024, 2048, 4096, 8192, 16384, 32768 */


/*

RBITS | MBITS  MIN  THR.  MSIZE  MAXS  MINL  MAXL   MAXO  R0MAX R0FAST
======+===============================================================
  3   |   5      3    2     32    32    33    288   2048    263   256
  4   |   4      3    2     16    16    17    272   4096    271   264
  5   |   3      3    2      8     8     9    264   8192    287   280

 */


/***********************************************************************
//
************************************************************************/

#define DBITS		13
#include "lzo_dict.h"
#define DVAL_LEN	DVAL_LOOKAHEAD



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* already included */

/*
vi:ts=4
*/

