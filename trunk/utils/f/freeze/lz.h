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





#ifndef BITS
#define BITS    14      /* for 16-bits machines */
#endif

#if BITS < 13
#undef BITS
#define BITS    13      /* 1:1 hash */
#endif

#if BITS > 21
#undef BITS
#define BITS    21      /* 4 MB hash table, if sizeof(u_short) == 2 */
#endif

#define LEN0    (BITS/3 + (BITS%3 != 0))
#define LEN1    (BITS/3 + (BITS%3 == 2))
#define LEN2    (BITS/3)

#define MASK0   ((1 << LEN0) - 1)
#define MASK1   ((1 << LEN1) - 1)
#define MASK2   ((1 << LEN2) - 1)

#define NIL     N

#if defined(M_XENIX) && defined(I_286) && (BITS > 14)
#define __XENIX__
#if BITS > 18
#undef BITS
#define BITS 18
#endif
#endif

#define array_size      (N + 1 + (1L << BITS))

extern hash_t prev[];

#ifndef __XENIX__
#define nextof(i)       next[i]
extern u_short
#ifdef __TURBOC__
		huge
#endif
			next[];
#else
#define parts (array_size/32768 + 1)
#define nextof(i)       next[(i) >> 15][(i) & 0x7fff]
#if parts == 2
extern u_short next0[], next1[];
#else
#	if parts == 3
extern u_short next0[], next1[], next2[];
#       else
#		if parts == 5
extern u_short next0[], next1[], next2[], next3[], next4[];
#		else
extern u_short next0[], next1[], next2[], next3[], next4[],
	next5[], next6[], next7[], next8[];
#		endif
#	endif
#endif
extern u_short *next[];
#endif

/* To eliminate function-call overhead */

#define DeleteNode(n) \
{\
       nextof(prev[n]) = NIL;\
       prev[n] = NIL;\
}

#define InsertNode(r)\
{\
	register hash_t p; register u_short first_son;\
	register uchar  *key;\
	key = &text_buf[r];\
	p = N + 1 + (key[0] & MASK0) |\
		    ((key[1] & MASK1) << LEN0) |\
		    ((key[2] & MASK2) << (LEN0 + LEN1));\
	first_son = nextof(p);\
	nextof(r) = first_son;\
	nextof(p) = r;\
	prev[r] = p;\
	prev[first_son] = r;\
}

#define Next_Char()\
if ((c = getchar()) != EOF) {\
	text_buf[s] = c;\
	if (s < _F - 1)\
		text_buf[s + N] = c;\
	s = (s + 1) & (N - 1);\
	r = (r + 1) & (N - 1);\
	InsertNode(r);\
	in_count++;\
} else {\
	s = (s + 1) & (N - 1);\
	r = (r + 1) & (N - 1);\
	if (--len) InsertNode(r);\
}
