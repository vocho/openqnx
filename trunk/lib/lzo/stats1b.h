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




/* stats1b.h -- statistics for the the LZO library

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */


#ifndef __LZO_STATS1B_H
#define __LZO_STATS1B_H

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
// Collect statistical information when compressing.
// Useful for finetuning the compression algorithm.
// Examine the symbol 'lzo1b_stats' with a debugger.
************************************************************************/

#if defined(LZO_COLLECT_STATS)
#  define LZO_STATS(expr)	expr
#else
#  define LZO_STATS(expr)	((void) 0)
#endif


#if defined(LZO_COLLECT_STATS)

typedef struct
{
/* algorithm configuration */
	unsigned r_bits;
	unsigned m3o_bits;
	unsigned dd_bits;
	unsigned clevel;

/* internal configuration */
	unsigned d_bits;
	long min_lookahead;
	long max_lookbehind;
	const char *compress_id;

/* counts */
	long lit_runs;
	long r0short_runs;
	long r0fast_runs;
	long r0long_runs;
	long m1_matches;
	long m2_matches;
	long m3_matches;
	long m4_matches;
	long r1_matches;

/* */
	long lit_run[R0MIN];
	long m2_match[M2_MAX_LEN + 1];
	long m3_match[M3_MAX_LEN + 1];
#if (M3O_BITS < 8)
	long lit_runs_after_m3_match;
	long lit_run_after_m3_match[LZO_SIZE(8-M3O_BITS)];
#endif

/* */
	long matches;
	long match_bytes;
	long literals;
	long literal_overhead;
	long literal_bytes;
	float literal_overhead_percent;

/* */
	long unused_dict_entries;
	float unused_dict_entries_percent;

/* */
	long in_len;
	long out_len;
}
lzo1b_stats_t;


void _lzo1b_stats_init(lzo1b_stats_t *lzo_stats);
void _lzo1b_stats_calc(lzo1b_stats_t *lzo_stats);

extern lzo1b_stats_t * const lzo1b_stats;

#define lzo_stats_t		lzo1b_stats_t
#define lzo_stats		lzo1b_stats

#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* already included */

/*
vi:ts=4
*/
