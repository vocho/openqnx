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





#include <stdlib.h>
#include "freeze.h"
#include "huf.h"

/*----------------------------------------------------------------------*/
/*									*/
/*		HUFFMAN ENCODING					*/
/*									*/
/*----------------------------------------------------------------------*/

#ifdef COMPAT
#undef  N
#undef  F
#define F               (new_flg ? _F : _FO)
#define N               (new_flg ? _NN : _NO)
#endif


/* TABLE OF ENCODE/DECODE for upper 6 bits position information */

/* The contents of this table are used for freezing only, so we use
 * this table freely when melting
 */

uchar Table[9] = { 0, 0, 1, 1, 1, 4, 10, 27, 18 };

uchar p_len[64];
uchar d_len[256];
uchar code[256];

#ifdef COMPAT
uchar table_old[9] = { 0, 0, 0, 1, 3, 8, 12, 24, 16 };
#endif

u_short freq[_T + 1];    /* frequency table */

short    prnt[_T + _NCHAR];    /* points to parent node */
/* notes :
   prnt[T .. T + N_CHAR - 1] used by
   indicates leaf position that corresponding to code */

short son[_T];           /* points to son node (son[i],son[i+]) */

u_short getbuf = 0;
uchar    getlen = 0;

uchar corrupt_flag = 0;         /* If a file is corrupt, use fcat */

u_short putbuf = 0;
uchar putlen = 0;

/* Initialize tree */

void StartHuff ()
{
	register short i, j;
#ifdef COMPAT
	if(do_melt == 0 || new_flg)
		init(Table);
	else
		init(table_old);
#else
	init(Table);
#endif
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
	freq[T] = 0xffff;
	prnt[R] = 0;
	in_count = 1;
	bytes_out = 5;
#ifdef DEBUG
	symbols_out = refers_out = 0;
#endif
	putlen = getlen = 0;
	putbuf = getbuf = 0;
	corrupt_flag = 0;
}


/* reconstruct tree */
void reconst ()
{
	register u_short i, j, k;
	register u_short f;
#ifdef DEBUG
	if (!quiet)
	  fprintf(stderr,
	    "Reconstructing Huffman tree: symbols: %ld, references: %ld\n",
	    symbols_out, refers_out);
#endif
	/* correct leaf node into of first half,
	   and set these freqency to (freq+1)/2       */
	j = 0;
	for (i = 0; i < T; i++) {
		if (son[i] >= T) {
			freq[j] = (freq[i] + 1) / 2;
			son[j] = son[i];
			j++;
		}
	}
	/* build tree.  Link sons first */
	for (i = 0, j = N_CHAR; j < T; i += 2, j++) {
		k = i + 1;
		f = freq[j] = freq[i] + freq[k];
		for (k = j - 1; f < freq[k]; k--);
		k++;
		{       register u_short *p, *e;
			for (p = &freq[j], e = &freq[k]; p > e; p--)
				p[0] = p[-1];
			freq[k] = f;
		}
		{       register short *p, *e;
			for (p = &son[j], e = &son[k]; p > e; p--)
				p[0] = p[-1];
			son[k] = i;
		}
	}
	/* link parents */
	for (i = 0; i < T; i++) {
		if ((k = son[i]) >= T) {
			prnt[k] = i;
		} else {
			prnt[k] = prnt[k + 1] = i;
		}
	}
}


/* update given code's frequency, and update tree */

void update (c)
	u_short c;
{
	register u_short *p;
	register u_short i, j, k, l;

	if (freq[_R] == MAX_FREQ) {
		reconst();
	}
	c = prnt[c + _T];
	do {
		k = ++freq[c];

		/* swap nodes when become wrong frequency order. */
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
	} while ((c = prnt[c]) != 0);	/* loop until reach to root */
}

void EncodeChar (c)
	u_short c;
{
	register short *p;
	unsigned long i;
	register u_short j, k;

	i = 0;
	j = 0;
	p = prnt;
	k = p[c + _T];

	/* trace links from leaf node to root */
	do {
		i >>= 1;

		/* if node index is odd, trace larger of sons */
		if (k & 1) i += 0x80000000;

		j++;
	} while ((k = p[k]) != _R) ;
	if (j > 16) {
		Putcode(16, (u_short)(i >> 16));
		Putcode(j - 16, (u_short)i);
	} else {
		Putcode(j, (u_short)(i >> 16));
	}
	update(c);
}

void EncodePosition (c)
	register u_short c;
{
	register u_short i;

	/* output upper 6bit from table */
	i = c >> 7;
	Putcode((u_short)(p_len[i]), (u_short)(code[i]) << 8);

	/* output lower 7bit */
	Putcode(7, (u_short)(c & 0x7f) << 9);
}

void EncodeEnd ()
{
	if (putlen) {
		putchar((int)(putbuf >> 8));
		bytes_out++;
		if (ferror(stdout))
			writeerr();
	}
}

short DecodeChar ()
{
	register u_short c;
	register u_short dx;
	register u_short cc;
	c = son[_R];

	/* trace from root to leaf,
	   got bit is 0 to small(son[]), 1 to large (son[]+1) son node */
	while (c < _T) {
		dx = getbuf;
		if (getlen <= 8) {
			if ((short)(cc = getchar()) < 0) {
				if (corrupt_flag) {
					corrupt_message();
					return ENDOF;
				}
				corrupt_flag = 1;
				cc = 0;
			}
			dx |= cc << (8 - getlen);
			getlen += 8;
		}
		getbuf = dx << 1;
		getlen--;
		c += (dx >> 15) & 1;
		c = son[c];
	}
	c -= _T;
	update(c);
	return c;
}

short DecodePosition ()
{
	register u_short i, j, c;

	/* decode upper 6 bits from table */
	i = GetByte();
	c = (u_short)code[i] << 7;
	j = d_len[i] - 1;

	/* get lower 7 bits */
	return c | (((i << j) | GetNBits (j)) & 0x7f);
}

#ifdef COMPAT
/* update given code's frequency, and update tree */

void updateO (c)
	u_short    c;
{
	register u_short *p;
	register u_short i, j, k, l;

	if (freq[_RO] == MAX_FREQ) {
		reconst();
	}
	c = prnt[c + _TO];
	do {
		k = ++freq[c];

		/* swap nodes when become wrong frequency order. */
		if (k > freq[l = c + 1]) {
			for (p = freq+l+1; k > *p++; ) ;
			l = p - freq - 2;
			freq[c] = p[-2];
			p[-2] = k;

			i = son[c];
			prnt[i] = l;
			if (i < _TO) prnt[i + 1] = l;

			j = son[l];
			son[l] = i;

			prnt[j] = c;
			if (j < _TO) prnt[j + 1] = c;
			son[c] = j;

			c = l;
		}
	} while ((c = prnt[c]) != 0);	/* loop until reach to root */
}

short DecodeCOld ()
{
	register u_short c;
	register u_short dx;
	register u_short cc;
	c = son[_RO];

	/* trace from root to leaf,
	   got bit is 0 to small(son[]), 1 to large (son[]+1) son node */
	while (c < _TO) {
		dx = getbuf;
		if (getlen <= 8) {
			if ((short)(cc = getchar()) < 0) {
				if (corrupt_flag) {
					corrupt_message();
					return ENDOF;
				}
				corrupt_flag = 1;
				cc = 0;
			}
			dx |= cc << (8 - getlen);
			getlen += 8;
		}
		getbuf = dx << 1;
		getlen--;
		c += (dx >> 15) & 1;
		c = son[c];
	}
	c -= _TO;
	updateO(c);
	return c;
}

short DecodePOld ()
{
	register u_short i, j, c;

	/* decode upper 6 bits from table */
	i = GetByte();
	c = (u_short)code[i] << 6;
	j = d_len[i] - 2;

	/* get lower 6 bits */
	return c | (((i << j) | GetNBits (j)) & 0x3f);
}

#endif

void init(table) uchar * table; {
	short i, j, k, num;
	num = 0;
	for(i = 1, j = 0; i <= 8; i++) {
		num += Table[i] << (8 - i);
		for(k = table[i]; k; j++, k--)
			p_len[j] = i;
	}
	if (num != 256) {
		fprintf(stderr, "Invalid position table\n");
		exit(1);
	}
	num = j;
	if (do_melt == 0)
		for(i = j = 0;;) {
			code[j] = i << (8 - p_len[j]);
			i++;
			j++;
			if(j == num) break;
			i <<= p_len[j] - p_len[j-1];
		}
	else {
		for(k = j = 0; j < num; j ++)
			for(i = 1 << (8 - p_len[j]); i--;)
				code[k++] = j;
		for(k = j = 0; j < num; j ++)
			for(i = 1 << (8 - p_len[j]); i--;)
				d_len[k++] =  p_len[j];
	}
}

void write_header() {
	u_short i;

	i = Table[5] & 0x1F; i <<= 4;
	i |= Table[4] & 0xF; i <<= 3;
	i |= Table[3] & 7;   i <<= 2;
	i |= Table[2] & 3;   i <<= 1;
	i |= Table[1] & 1;

	putchar((int)(i & 0xFF));
	putchar((int)((i >> 8) & 0x7F));
	putchar((int)(Table[6] & 0x3F));
	if (ferror(stdout))
		writeerr();
}

int read_header() {
	short i, j;
	i = getchar() & 0xFF;
	i |= (getchar() & 0xFF) << 8;
	Table[1] = i & 1; i >>= 1;
	Table[2] = i & 3; i >>= 2;
	Table[3] = i & 7; i >>= 3;
	Table[4] = i & 0xF; i >>= 4;
	Table[5] = i & 0x1F;
	Table[6] = getchar() & 0x3F;

	i = Table[1] + Table[2] + Table[3] + Table[4] +
	Table[5] + Table[6];

	i = 62 - i;     /* free variable length codes for 7 & 8 bits */

	j = 128 * Table[1] + 64 * Table[2] + 32 * Table[3] +
	16 * Table[4] + 8 * Table[5] + 4 * Table[6];

	j = 256 - j;    /* free byte images for these codes */

/*      Equation:
	    Table[7] + Table[8] = i
	2 * Table[7] + Table[8] = j
*/
	j -= i;
	if (j < 0 || i < j) {
		corrupt_message();
		return EOF;
	}
	Table[7] = j;
	Table[8] = i - j;

#ifdef DEBUG
	fprintf(stderr, "Codes: %d %d %d %d %d %d %d %d\n",
		Table[1], Table[2], Table[3], Table[4],
		Table[5], Table[6], Table[7], Table[8]);
#endif
	return 0;
}

void corrupt_message ( )        /* file too short or invalid header */
{
	fprintf ( stderr, "melt: corrupt input\n" );
}
