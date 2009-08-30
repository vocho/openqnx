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
  conversion routines

numb_t *dtonum(int n)
	This routine converts a integer into a numb_t type.  It utilizes
	the storage layout of numbers, ie LSD to MSD to avoid having to
	"reverse" the number when it is finished the convertion (ie. itoa()).

int numtod(numb_t *n)
   	This routine converts a numb_t to it's integer representation.
	fractional portions are discarded.  If the number is too large
	to fit an integer, the integer overflows by the characteristics
	of the machine. ie NO RANGE CHECKING IS PERFORMED.


numb_t *strtonum(char *s,char **end, int base);
	This is modeled after the standard library routine "strtol()".
	It is a brute force method of conversion.

int numtos(numb_t *n, char *s, int lim, int base);
	Make an asci string, up to lim chars in 'base' from num.

*/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "number.h"

/*
 * This number is chosen to be longer than a 32-bit long. The number will be
 * resized before exiting to minimize length.
 */

#define	MAX_INT_LENGTH	6

numb_t         *
dtonum(int n)
{
	digit_t        *v;
	int             sign = 0;
	numb_t         *x;
	int             i;

	if (n < 0) {
		sign = 1;
		n = -n;
	}
	if ((v = ALLOCMEM(digit_t, MAX_INT_LENGTH)) == 0) {
		no_mem("dtonum");
	}
	for (i = 0; n; i++) {
		vl_mull(v + i, CALC_BASE, MAX_INT_LENGTH - i);
		vl_addl(v + i, n % CALC_BASE, MAX_INT_LENGTH - i);
		n /= CALC_BASE;
	}
	if ((x = NEWHDR()) == 0) {
		no_mem("dtonum");
	}
	x->sign = sign;
	x->decpos = 0;
	x->len = MAX_INT_LENGTH;
	x->digit = v;
	return normal(x);
}

int numtod(numb_t *n)
{
	register int    val = 0;
	int             i;
	if (n == 0)
		return 0;
	if (n->digit == 0)
		return 0;
	if (n->len < n->decpos)
		return 0;
	for (i = n->len - 1; i >= n->decpos; i--) {
		val *= CALC_BASE;
		val += n->digit[i];
	}
	return n->sign ? -val : val;
}







static char     input_set[] = "0123456789ABCDEF";


numb_t         *
strtonum(char *user_s, char **end, int base)
{
	digit_t        *mantissa, *fraction;
	digit_t        *scale_v;/* scale_vector : convert fraction
				 * appropriately */
	digit_t        *res;
	int             mant_len;
	int             frac_len;
	int             i;
	numb_t         *v;
	char           *s = user_s;
	int             scale;
	int             sign = 0;
	char           *t;
	if (base < 2) {
		if (end != 0)
			*end = s;
		return 0;	/* can't handle extremely small bases */
	}
	while (isspace(*s))
		s++;
	if (*s == '-') {
		sign = 1;
		s++;
		while (isspace(*s))
			s++;
	}
	mant_len = strlen(s) + 1;

	if (!(mantissa = ALLOCMEM(digit_t, mant_len))) {
		no_mem("string_to_number");
		return 0;
	}
	if (!(v = NEWHDR())) {
		no_mem("string_to_number");
		return 0;
	}
	v->decpos = 0;

	while (*s && (t = strchr(input_set, toupper(*s)))) {
		vl_mull(mantissa, base, mant_len);
		vl_addl(mantissa, t - input_set, mant_len);
		s++;		/* move pointer along */
	}
	/* have accumulated the mantissa, now accumulate the fractional part */
	if (*s == '.') {
		s++;
		/*
		 * to preserve the best of what the user wants, we must
		 * select the length for the fractional part to be the
		 * greater of the number of decimal places in the number and
		 * the internal scale quantity:
		 */
		i = strlen(user_s) - (s - user_s);
		scale = get_scale();
		i = roundew(i) / 2;
		if (i > scale)
			scale = i;

		frac_len = mant_len + scale + 1;
		if (!(fraction = ALLOCMEM(digit_t, frac_len))) {
			no_mem("string_to_number");
			return 0;
		}
		if (!(scale_v = ALLOCMEM(digit_t, mant_len))) {
			no_mem("string_to_number");
			return 0;
		}
		ltov(1, scale_v, mant_len);	/* initial base point */
		while (*s && (t = strchr(input_set, toupper(*s)))) {
			vl_mull(fraction + scale, base, frac_len - scale);	/* multiply by base */
			vl_addl(fraction + scale, t - input_set, frac_len - scale);
			vl_mull(scale_v, base, mant_len);
			s++;
		}
		if (!(res = ALLOCMEM(digit_t, frac_len))) {
			no_mem("string_to_number");
			return 0;
		}
		for (i = mant_len - 1; scale_v[i] == 0 && i > 0; i--)	/* find end */
			;
		vl_divv(res, fraction, scale_v, frac_len, i + 1);
		vl_addv(res + scale + 1, mantissa, mant_len);

		FREEMEM(fraction);
		FREEMEM(mantissa);
		FREEMEM(scale_v);
		mantissa = res;
		v->decpos = scale + 1;
		mant_len = frac_len;
	}
	v->len = mant_len;
	v->digit = mantissa;
	v->sign = sign;
	if (end)
		*end = s;
	return normal(v);
}


static
int int_pow(int base, int exp)
{
	int             res = 1;
	while (exp-- > 0)
		res *= base;
	return res;
}

static
void shift_strl(char *s, int n)
{
	char           *t = s + n;
	while ((*s++ = *t++));
}

int numtos_base10(numb_t * n, char *s, int lim)
{
	digit_t        *x;
	digit_t        *y;
	int             j;
	int             first = 1;
	if (!n || !n->digit) {
		*s++ = '0';
		*s = '\0';
		return 1;
	}
	if (n->len * 2 + (n->decpos != 0) > lim) {
		for (j = 0; j < lim - 1; j++)
			s[j] = '*';
		s[j] = '\0';
		return j;
	}
	j = 0;
	if (n->sign)
		s[j++] = '-';
	x = n->digit + n->len - 1;
	y = n->digit + n->decpos;
	for (; x >= y; x--) {
		s[j++] = (*x / 10) + '0';

		if (first) {
			if (s[j - 1] == '0')
				j--;
			first = 0;
		}
		s[j++] = (*x % 10) + '0';
	}
	if (n->decpos)
		s[j++] = '.';
	for (; x >= n->digit; x--) {
		s[j++] = (*x / 10) + '0';
		s[j++] = (*x % 10) + '0';
	}
	s[j] = '\0';
	return j + (n->decpos != 0);
}

/*-
	This is a little convoluted.  There are two main sections:
	1:	left of the decimal.
		This is quite straight-forward, and is just a matter of
		shifting the string down by the base, and collecting the
		remainders.  Notice that the string is built in reverse,
		because multi-digit bases are permitted, and are a trifle
		difficult to reverse.

	2:	right of the decimal.
		This is more difficult.  The problem is outlined as follows:

			x / base1 = y / base2.
			thus y = (x*base2)/base1.
		thus	0.5 base 10 == 0.8 base 16.
		The principal difficulty here results from determining the
		bases.  the value 0.54321 may be viewed as
			54321/100000, ie a fraction of the next order of magnitude
				of it's base.
		however, when we are attempting to convert this to some other
		base, perhaps 16, we must find a number, which is a power of
		16 JUST greater than 100000.
		this value is then multiplied by the original to generate a
		value which may be output by the same method as case 1.
		Fortunately, there is a simple method of determining how many
		digits of the resultant multiplication are valid:  It is exactly
		the number of digits to the right of the decimal.

	This is my n-th attempt at this problem.  An earlier attempt, relied
	upon logarithms to derive some of the required scale factors.  I am
	more confident that the generates the "proper" values than the previous
	attempt.

	I owe a debt of gratitude to Richard Jackson who pointed me in this
	direction.
*/

int numtos(numb_t *n, char *s, int lim, int base)
{
	digit_t        *p;
	int             nlen;
	int             t;
	char            temp_str[10];
	int             order;
	int             ofmt_type;
	char           *ustr = s + lim - 1;	/* point at last byte of
						 * string */
	int             templen;

	if (base < 2) {
		for (t = 0; t < lim - 1; t++)
			s[t] = '*';
		s[t] = '\0';
		return t;
	}
#if 0
	if (base == 10) {
		return numtos_base10(n, s, lim);
	}
#endif
	ofmt_type = base <= 16;
	for (order = 1; int_pow(10, order) < base; order++);
	templen = ofmt_type ? 1 : order + 1;

	if (n->digit == 0) {
		memset(s, '0', order);
		s[order] = '\0';
		return order;
	}
	*ustr = '\0';		/* the last byte must be null */


	/* handle fraction, if any */
	if (n->decpos > 0) {
		numb_t         *max_base, *new_base;
		numb_t         *temp1, *temp;
		numb_t         *y;
		int             i, out_digits;
		int             offset;

		y = dup_num(n);	/* make private copy */
		/*
		 * generate the least order of magnitude (base 100) which
		 * with more significant digits than y
		 */
		for (i = y->len - 1; i > 0 && i >= y->decpos; i--)
			y->digit[i] = 0;	/* clear out leading digits */
		temp = dtonum(CALC_BASE);
		temp1 = dtonum(y->decpos);
		max_base = raise_num(temp, temp1);
		delete_num(temp1);
		delete_num(temp);
		/*
		 * Now, generate the least order of magnitude, in the target
		 * base greater than max_base.
		 */
		temp = dtonum(base);
		new_base = dtonum(1);
		for (out_digits = 0; num_compare(new_base, max_base) < 0; out_digits++) {
			temp1 = mult_num(new_base, temp);
			delete_num(new_base);
			new_base = temp1;
		}
		delete_num(temp);
		/*
		 * we can safely determine here how many output digits we
		 * will generate
		 */
		if (ustr - out_digits * templen < s) {
			delete_num(new_base);
			delete_num(max_base);
			delete_num(y);
			*s = '\0';
			return -1;
		}
		/*
		 * We "scale" up the value y, such that the value to be
		 * output lies to the "left" of the decimal position.
		 * Anything to the right is to be ignored.
		 */
		temp1 = mult_num(y, new_base);
		offset = temp1->decpos;
		delete_num(new_base);
		delete_num(max_base);
		/*
		 * The remainder is simple, we just successively scale down
		 * the value until we run out of digits.
		 */
		for (i = 0; i < out_digits; i++) {
			t = vl_divl(temp1->digit + offset, base, temp1->len - offset);
			switch (ofmt_type) {
			case 0:
				sprintf(temp_str, "%*.*d ", order, order, t);
				break;
			case 1:
				temp_str[0] = t >= 10 ? 'A' + t - 10 : '0' + t;
				temp_str[1] = '\0';
			}
			ustr = strncpy(ustr - templen, temp_str, templen);
		}
		delete_num(temp1);
		delete_num(y);
		if (ustr > s)
			*--ustr = '.';
		else {
			*s = '\0';
			return -1;
		}
	}
	/* handle the non-fraction */
	if (n->len > n->decpos) {
		nlen = n->len - n->decpos;
		if ((p = ALLOCMEM(sizeof *p, nlen)) == 0)
			no_mem("numtos");
		memcpy(p, n->digit + n->decpos, nlen);
		while (!vl_iszero(p, nlen)) {
			t = vl_divl(p, base, nlen);	/* remove section by
							 * section */
			switch (ofmt_type) {
			case 0:
				sprintf(temp_str, "%*.*d ", order, order, t);
				break;
			case 1:
				temp_str[0] = t >= 10 ? 'A' + t - 10 : '0' + t;
				temp_str[1] = '\0';
			}
			if (ustr - templen < s) {
				FREEMEM(p);
				*s = '\0';
				return -1;
			}
			ustr = strncpy(ustr - templen, temp_str, templen);
		}
		FREEMEM(p);
	}
	/*
	 * trim leading zeros
	 */
	while (*ustr == '0' && isxdigit(ustr[1]))
		ustr++;
	/*
	 * insert sign char
	 */
	if (n->sign) {
		if (ustr > s)
			*--ustr = '-';
		else {
			*s = '\0';
			return -1;
		}
	}
	/*
	 * shift string into place, if necessary
	 */
	if (ustr > s) {
		shift_strl(s, ustr - s);
	}
	return strlen(s);
}
