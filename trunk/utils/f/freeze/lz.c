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





#include "freeze.h"
#include "lz.h"

/*----------------------------------------------------------------------*/
/*									*/
/*                          LZSS ENCODING                               */
/*									*/
/*----------------------------------------------------------------------*/

uchar    text_buf[N + _F - 1];
u_short   match_position, match_length;

/* next[N+1..] is used as hash table,
	the rest of next is a link down,
	prev is a link up.
*/

hash_t             prev[N + 1];

#ifndef __XENIX__
u_short
#ifdef __TURBOC__
	huge
#endif
		next[array_size];
#else
#if parts == 2
u_short next0[32768], next1[8193];
#else
#	if parts == 3
u_short next0[32768], next1[32768], next2[8193];
#	else
#		if parts == 5
u_short next0[32768], next1[32768], next2[32768], next3[32768], next4[8193];
#		else
u_short next0[32768], next1[32768], next2[32768], next3[32768], next4[32768],
	next5[32768], next6[32768], next7[32768], next8[8193];
#		endif
#	endif
#endif

u_short *next[parts] = {
next0, next1
#if parts > 2
,next2
#if parts > 3
,next3, next4
#if parts > 5
,next5, next6,
next7, next8
#endif
#endif
#endif
};
#endif

#ifdef GATHER_STAT
long node_steps, node_matches;
#endif

/* Initialize Tree */
void InitTree ()
{
	long i;
#ifdef GATHER_STAT
	node_steps = node_matches = 0;
#endif

	for (i = N + 1; i < array_size; i++ )
		nextof(i) = NIL;
	for (i = 0; i < sizeof(prev)/sizeof(*prev); i++ )
		prev[i] = NIL;
}

/* Get next match */
void Get_Next_Match (r)
	u_short r;
{
	register uchar  *key;
	register u_short        m, i, p;
#ifdef GATHER_STAT
	node_matches++;
#endif
	key = &text_buf[p = r];
	m = 0;
	while (m < _F) {
		if ((p = nextof(p)) == NIL) {
			match_length = m;
			return;
		}

/* This statement is due to ideas of Boyer and Moore: */

		if(key[m] != text_buf[p + m])
			continue;

/* This statement is due to my ideas: :-) */
/* It gives up to 8% speedup on files with big redundancy (text, etc.) */

		if(key[m >> 1] != text_buf[p + (m >> 1)])
			continue;

#ifdef GATHER_STAT
		node_steps++;
#endif

/* This statement doesn't take a lot of execution time -
	about 20% (in profiling we trust)
*/
		for (i = 0; i < _F && key[i] == text_buf[p + i];  i++);

		if (i > m) {
			match_position = ((r - p) & (N - 1)) - 1;
			m = i;
		}
	}
#ifdef DEBUG
	if (verbose)
		fprintf(stderr, "Replacing node: %d -> %d\n", p, r);
#endif
	nextof(prev[p]) = nextof(p);
	prev[nextof(p)] = prev[p];
	prev[p] = NIL;  /* remove p, it is further than r */
	match_length = _F;
}
