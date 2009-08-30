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




/*-
 * 
 * numb_t *normal(numb_t *x)
 * return x with no leading/trailing zeroes.
 * 
 * digit_t *unnormal(numb_t *x, int len, int decpos)
 * generate a vector of given length and alignment from numb_t x.
 * 
 * int trim_num(numb_t *x)
 *  trim trailing zeros from a number. (use decpos).
 */


#include "number.h"
#include <stdlib.h>
#include <string.h>

#ifndef	NUM_FRSIZE
#define	NUM_FRSIZE	128
#endif

static FRAME   *num_pool;

static
void init_number()
{
	if ((num_pool = fr_create(NUM_FRSIZE, sizeof(numb_t))) == 0) {
		no_mem("init_number");
	}
}



int delete_num(numb_t *x)
{

	if (num_pool == 0) {
		init_number();
	}
	if (x == 0) {
		return -1;
	}
	if (x->digit) {
		FREEMEM(x->digit);
	}
	fr_free(num_pool, x);
	return 1;
}


numb_t         *
create_num()
{
	numb_t         *n;

	if (num_pool == 0)
		init_number();
	if ((n = fr_alloc(num_pool)) == 0) {
		no_mem("create_num");
	}
	memset(n, 0, sizeof(numb_t));
	return n;
}



numb_t         *
clear_num(numb_t *r)
{
	register int    i;
	if (r == 0)
		return r;
	for (i = 0; i < r->len; i++)
		r->digit[i] = 0;
	r->decpos = 0;
	r->sign = 0;
	return r;
}


numb_t         *
dup_num(r)
	register numb_t *r;
{
	register numb_t *s;
	if (r == 0) {
		error(0, "dup_num: warning, attempt to duplicate a 0 ptr\n");
		return NEWHDR();
	}
	if ((s = NEWHDR()) == 0)
		return 0;
	*s = *r;
	if (s->digit == 0)
		return s;
	if (s->len <= 0) {
		error(0, "dup_num, zero length segment not 0, length=%d\n", s->len);
		s->digit = 0;
		return s;
	} else if (!NEWNUM(s)) {
		no_mem("dup_num");
	}
	memcpy(s->digit, r->digit, s->len);
	return s;
}


int get_size(numb_t *n)
{
	if (n == 0)
		return 0;
	return max(n->decpos, n->len);
}

int get_precision(numb_t *n)
{
	if (n == 0)
		return 0;
	return n->decpos * 2;
}

int get_length(numb_t *n)
{
	return get_size(n) * 2;
}


int set_length(numb_t *n, int len)
{
	int             i;
	if (n == 0)
		return -1;
	if (n->len == 0)
		n->digit = ALLOCMEM(digit_t, len);
	else
		n->digit = MEMCHSIZE(n->digit, len * sizeof(digit_t));

	if (n->digit == 0) {
		no_mem("set_length");
		return -1;
	}
	if (len > n->len) {	/* ok, pad to "left" with 0 */
		for (i = n->len; i < len; i++)
			n->digit[i] = 0;
	}
	n->len = len;
	return 0;
}

int set_prec(numb_t *n, int len)
{
	int             i;
	digit_t          *p;
	int             diff = len - n->decpos;
	if (n == 0)
		return 0;
	/* get some fresh memory for it */
	p = ALLOCMEM(digit_t, n->len + diff);
	for (i = 0; i < diff; i++)
		p[i] = 0;
	memcpy(p + i, n->digit, n->len);
	FREEMEM(n->digit);
	n->digit = p;
	n->len += diff;
	n->decpos += diff;
	return 0;
}

int copy_num(numb_t *to, numb_t *from)
{
	if (to == 0)
		return 1;
	if (from == 0) {
		FREEMEM(to->digit);
		to->digit = 0;
		to->sign = to->decpos = 0;
		return 1;
	}
	if (to->len != from->len)
		set_length(to, from->len);

	memcpy(to->digit, from->digit, from->len);
	to->decpos = from->decpos;
	to->sign = from->sign;
	return 1;
}

numb_t         *
get_align(numb_t *o1, numb_t *o2)
{
	int             dp, len;
	int             sig1, sig2, sig;
	numb_t         *res;

	sig1 = o1->len - o1->decpos;
	sig2 = o2->len - o2->decpos;
	/* number of signficant digits to be held, left of dp. */
	if ((sig = max(sig1, sig2)) < 0) {
		sig = 0;
	}
	dp = max(o1->decpos, o2->decpos);
	len = dp + sig;
	if ((res = NEWHDR()) == 0) {
		error(1, "get_align: no memory");
	}
	res->sign = 0;
	res->decpos = dp;
	res->len = dp + sig + 1;
	return res;
}



digit_t        *
unnormal(numb_t *n, int nlen, int ndecpos)
{
	digit_t        *t;

	if (n == 0) {
		if ((t = ALLOCMEM(digit_t, nlen)) == 0)
			no_mem("unnormal");
		return t;
	}
	if (nlen < n->len || ndecpos < n->decpos || ndecpos > nlen) {
		error(0, "unnormal: invalid args (%d,%d) : (%d,%d)\n", nlen, ndecpos, n->len, n->decpos);
		return 0;
	}
	if ((t = ALLOCMEM(digit_t, nlen)) == 0)
		no_mem("unnormal");
	memmove(t + ndecpos - n->decpos, n->digit, n->len);
	return t;
}


int trim_num(numb_t *x)
{
	int             i;
	int             mval;

	if ((x == 0) || x->digit == 0)
		return 0;
	mval = min(x->decpos, x->len);	/* get whatever is shortest */

	for (i = 0; i < mval && x->digit[i] == 0; i++);
	if (i == 0)
		return 1;
	if (i == x->len) {
		FREEMEM(x->digit);
		x->digit = 0;
		x->len = x->decpos = x->sign = 0;
		return 0;
	}
	x->decpos -= i;
	memmove(x->digit, x->digit + i, x->len - i);
	CHGSIZ(x, x->len - i);
	return 1;
}




numb_t         *
normal(numb_t *x)
{
	register int    i;

	if (trim_num(x) == 0)
		return x;
	if (x->len == 0) {
		if (x->digit != 0)
			FREEMEM(x->digit);
		x->digit = 0;
		x->decpos = 0;
		x->sign = 0;
		return x;
	}
	for (i = x->len - 1; x->digit[i] == 0 && i >= 0; i--);

	if (i == x->len - 1)
		return x;	/* return number unmolested. */

	if (i < 0) {
		FREEMEM(x->digit);
		x->digit = 0;
		x->len = x->decpos = x->sign = 0;
		return x;
	} else {
		CHGSIZ(x, i + 1);
	}
	return x;
}
