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





#ifdef COMPILE_STATIST_PROGRAM
#include "freeze.h"
#include "lz.h"
#include "huf.h"

#undef _NCHAR
#define _NCHAR 62

long in_count, bytes_out;

unsigned long indicator_threshold = 4096, indicator_count;

u_short bits[9];

short   prnt[_T + _NCHAR];

hash_t  prev[N + 1];

#ifndef __XENIX__
u_short
#ifdef __TURBOC__
	huge
#endif
		next[array_size];
#else
#if parts == 2
u_short next0[32768], next1[8193];
#elif parts == 3
u_short next0[32768], next1[32768], next2[8193];
#elif parts == 5
u_short next0[32768], next1[32768], next2[32768], next3[32768], next4[8193];
#else
u_short next0[32768], next1[32768], next2[32768], next3[32768], next4[32768],
	next5[32768], next6[32768], next7[32768], next8[8193];
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

#ifndef INT_SIG
void
#endif
giveres();

main(argc) {
	if(argc != 1) {
		fprintf(stderr, "Usage: statist < sample_file\n");
		fprintf(stderr, "Press INTR to display current values\n");
		exit(0);
	}
	signal(SIGINT, giveres);
	freeze();
	giveres();
}

#ifndef INT_SIG
void
#endif
giveres() {
	u_short c;
	signal(SIGINT, giveres);
	for(c = 0; c < 62; c++) {
		register short *p;
		register int j, k;

		j = 0;
		p = prnt;
		k = p[c + T];

		do j++; while ((k = p[k]) != R) ;
		if (j <= 8)
			bits[j]++;
	}
	printf("%d %d %d %d %d %d %d %d\n",
		bits[1], bits[2], bits[3], bits[4],
		bits[5], bits[6], bits[7], bits[8]);
	fflush(stdout);
	for (c = 1; c <= 8; c++) bits[c] = 0;
}

freeze ()
{
	register u_short i, len, r, s;
	register short c;
	StartHuff();
	InitTree();
	s = 0;
	r = N - _F;
	for (i = s; i < r; i++)
		text_buf[i] = ' ';
	for (len = 0; len < _F && (c = getchar()) != EOF; len++)
		text_buf[r + len] = c;
	in_count = len;
	for (i = 0; i <= _F; i++)
		InsertNode(r + i - _F);
	while (len != 0) {
		Get_Next_Match(r);
		if (match_length > len)
			match_length = len;
		if (match_length <= THRESHOLD) {
			match_length = 1;
		} else {
			register u_short orig_length, orig_position;
			orig_length = match_length;
			orig_position = match_position;
			DeleteNode(s);
			Next_Char();
			Get_Next_Match(r);
			if (match_length > len) match_length = len;
			if (orig_length > match_length) {
				update((u_short)orig_position >> 7);
				match_length = orig_length - 1;
			} else
				update(match_position >> 7);
		}
		for (i = 0; i < match_length &&
				(c = getchar()) != EOF; i++) {
			DeleteNode(s);
			text_buf[s] = c;
			if (s < _F - 1)
				text_buf[s + N] = c;
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			InsertNode(r);
		}
		in_count += i;
		if ((in_count > indicator_count)) {
			fprintf(stderr, "%5dK\b\b\b\b\b\b", in_count / 1024);
			fflush (stderr);
			indicator_count += indicator_threshold;
		}
		while (i++ < match_length) {
			DeleteNode(s);
			s = (s + 1) & (N - 1);
			r = (r + 1) & (N - 1);
			if (--len) InsertNode(r);
		}
	}
}

uchar    text_buf[N + _F - 1];
u_short   match_position, match_length;

InitTree ()
{
	long i;
	for (i = N + 1; i < array_size; i++ )
		nextof(i) = NIL;
	for (i = 0; i < sizeof(prev)/sizeof(*prev); i++ )
		prev[i] = NIL;
}

Get_Next_Match (r)
	u_short r;
{
	register uchar  *key;
	register u_short        m, i, p;
	key = &text_buf[p = r];
	m = 0;
	while(m < _F) {
		if ((p = nextof(p)) == NIL) {
			match_length = m;
			return;
		}
		if(key[m] != text_buf[p + m])
			continue;
		if(key[m >> 1] != text_buf[p + (m >> 1)])
			continue;
		for (i = 0; i < _F && key[i] == text_buf[p + i];  i++);
		if (i > m) {
			match_position = ((r - p) & (N - 1)) - 1;
			m = i;
		}
	}
	nextof(prev[p]) = nextof(p);
	prev[nextof(p)] = prev[p];
	prev[p] = NIL;
	match_length = _F;
}

unsigned long freq[_T + 1];

short son[_T];

StartHuff ()
{
	register short i, j;
	for (i = 0; i < N_CHAR; i++) {
		freq[i] = 1;
		son[i] = i + T;
		prnt[i + T] = i;
	}
	i = 0; j = N_CHAR;
	while (j <= R) {
		freq[j] = freq[i] + freq[i + 1];
		son[j] = i;
		prnt[i] = prnt[i + 1] = j;
		i += 2; j++;
	}
	freq[T] = 0xffffffff;
	prnt[R] = 0;
	in_count = 1;
	bytes_out = 2;
}

update (c)
	u_short c;
{
	register unsigned long *p, k;
	register u_short i, j, l;
	c = prnt[c + _T];
	do {
		k = ++freq[c];
		if (k > freq[l = c + 1]) {
			for (p = freq+l+1; k > *p++; ) ;
			l = p - freq - 2;
			freq[c] = p[-2];
			p[-2] = k;
			i = son[c];
			prnt[i] = l;
			if (i < _T) prnt[i + 1] = l;
			j = son[l];
			son[l] = i;
			prnt[j] = c;
			if (j < _T) prnt[j + 1] = c;
			son[c] = j;
			c = l;
		}
	} while ((c = prnt[c]) != 0);
}

#endif

