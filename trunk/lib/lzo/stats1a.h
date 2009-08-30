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




/* stats1a.h -- statistics for the the LZO1A algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the LZO package and is subject
   to change.
 */


#ifndef __LZO_STATS1A_H
#define __LZO_STATS1A_H

#ifdef __cplusplus
extern "C" {
#endif



/***********************************************************************
// collect statistical information when compressing
// used for finetuning, view with a debugger
************************************************************************/

#if defined(LZO_COLLECT_STATS)
#  define LZO_STATS(expr)	expr
#else
#  define LZO_STATS(expr)	((void) 0)
#endif


/***********************************************************************
//
************************************************************************/

typedef struct {

/* configuration */
	unsigned rbits;
	unsigned clevel;

/* internal configuration */
	unsigned dbits;
	unsigned lbits;

/* constants */
	unsigned min_match_short;
	unsigned max_match_short;
	unsigned min_match_long;
	unsigned max_match_long;
	unsigned min_offset;
	unsigned max_offset;
	unsigned r0min;
	unsigned r0fast;
	unsigned r0max;

/* counts */
	long short_matches;
	long long_matches;
	long r1_matches;
	long lit_runs;
	long lit_runs_after_long_match;
	long r0short_runs;
	long r0fast_runs;
	long r0long_runs;

/* */
	long lit_run[RSIZE];
	long lit_run_after_long_match[RSIZE];
	long short_match[MAX_MATCH_SHORT + 1];
	long long_match[MAX_MATCH_LONG + 1];
	long marker[256];

/* these could prove useful for further optimizations */
	long short_match_offset_osize[MAX_MATCH_SHORT + 1];
	long short_match_offset_256[MAX_MATCH_SHORT + 1];
	long short_match_offset_1024[MAX_MATCH_SHORT + 1];
	long matches_out_of_range;
	long matches_out_of_range_2;
	long matches_out_of_range_4;
	long match_out_of_range[MAX_MATCH_SHORT + 1];

/* */
	long in_len;
	long out_len;
}
lzo1a_stats_t;

extern lzo1a_stats_t *lzo1a_stats;



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* already included */

/*
vi:ts=4
*/
