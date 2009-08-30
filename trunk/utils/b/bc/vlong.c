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
 * vlong implements a suite of arithmetic operations an 'very long'
 * numbers.
 * The numbers are stored as an array of digits in {CALC_BASE=100}.
 *
 */

#include <stdlib.h>
#include <string.h>
#include "vlong.h"
#include "mem.h"

extern void no_mem(char *);

int vl_addl(digit_t *dest, long scalar, int n)
{
	register long   carry = 0;

	for (carry = scalar; carry && n--;) {
		carry += *dest;
		*dest++ = (digit_t) (carry % CALC_BASE);
		carry /= CALC_BASE;
	}
	return carry;
}

int vl_divl(digit_t *val, long scale, int n)
{
	register long   curval = 0;

	while (n--) {
		curval *= CALC_BASE;
		curval += val[n];
		if (curval < scale) {
			val[n] = 0;
		} else {
			val[n] = (digit_t) (curval / scale);	/* store it */
			curval %= scale;
		}
	}
	return curval;
}


int vl_subl(digit_t *dest, long scalar, int n)
{
	register long   borrow = 0;
	long            op1, op2;

	for (borrow = scalar; borrow && n--;) {
		op1 = *dest;
		op2 = borrow % CALC_BASE;
		if ((op1 -= op2) < 0) {
			op1 += CALC_BASE;
			borrow += CALC_BASE;
		}
		*dest++ = op1;
		borrow /= CALC_BASE;
	}
	return borrow;
}

int vl_mull(digit_t *dest, long scale, int n)
{
	long            carry = 0;
	register long   temp;

	while (n--) {
		temp = *dest * scale;
		temp += carry;
		*dest++ = (digit_t) (temp % CALC_BASE);
		carry = temp / CALC_BASE;
	}
	return (int) carry;
}

int vl_neg(digit_t *n, int len)
{
	while (len-- > 0) {
		if (!*n)
			continue;
		*n = CALC_BASE - *n;
		n++;
	}
	return 0;
}

int vl_iszero(digit_t *v, int n)
{

	while (n--)
		if (*v++)
			return 0;
	return 1;
}

int vl_cmp(digit_t *u, digit_t *v, int n)
{
	register int    t;

	v += n - 1;
	u += n - 1;
	while (n--) {
		if ((t = ((int) *u-- - (int) *v--)))
			return t < 0 ? -1 : 1;
	}
	return 0;
}


int ltov(long val, digit_t *v, int n)
{
	int             i;

	if (n < 0)
		n = -n;		/* ignore signs */

	for (i = 0; val && i < n; i++) {
		vl_mull(v + i, CALC_BASE, n - i);
		vl_addl(v + i, val % CALC_BASE, n - i);
		val /= CALC_BASE;
	}
	return 1;
}
/*
 * int    vl_addv(digit_t *dest, digit_t *src, int len)
 * 
 * adds "src" to "dest" in dest. returns: amount of carry if any.
 * 
 */


int vl_addv(digit_t *dest, digit_t *src, int len)
{
	register int    temp;
	int             carry = 0;
	while (len-- > 0) {
		*dest = (carry = ((temp = *dest + *src++ + carry) >= CALC_BASE)) ?
			temp - CALC_BASE : temp;
		dest++;
	}
	return carry;
}

int vl_subv(digit_t *dest, digit_t *src, int len)
{
	int             borrow = 0;

	while (len-- > 0) {
		register int    i1 = *dest;
		if ((borrow = ((i1 -= *src++ + borrow) < 0)))
			i1 = CALC_BASE + i1;
		*dest++ = i1;
	}
	return borrow;
}




int 
vl_mulv(digit_t *prod, digit_t  *op1, digit_t *op2, int len1, int len2)
{
	int             i, j;
	int             temp1;
	register int    carry;
	register digit_t *pp;
	register digit_t *p1;
	/* swap to reduce inner loop */
	if (len1 > len2) {
		i = len1;
		len1 = len2;
		len2 = i;
		pp = op1;
		op1 = op2;
		op2 = pp;
	}
	for (i = len2; i--;) {
		pp = prod++;
		if (!(temp1 = *op2++))
			continue;	/* skip it if zero */
		carry = 0;
		for (j = len1, p1 = op1; j--; pp++) {
			*pp = (carry += temp1 * *p1++ + *pp) % CALC_BASE;
			carry /= CALC_BASE;
		}
		*pp = carry;
	}
	return 0;		/* carry is not possible */
}





int vl_divv(digit_t *quo, digit_t *u, digit_t *v, int u_len, int v_len)
{
	register digit_t *pu;
	register digit_t *temp_num;
	int             dfactor;
	int             digit_1, digit_2;
	int             temp1;
	register int    qx;
	int             j;

	/*
	 * Create a temporary, one larger than the divisor. This will be used
	 * to perform the actual calculations.
	 */

	if ((temp_num = ALLOCMEM(digit_t, v_len + 1)) == 0)
		no_mem("div_vector");

	/*
	 * Knuth Step D1:	calculate an appropriate scaling factor. The
	 * value chosen is the largest factor which will not cause the
	 * divisor to change size.
	 */
	if ((dfactor = CALC_BASE / (v[v_len - 1] + 1)) > 1) {
		u[u_len - 1] = vl_mull(u, dfactor, u_len - 1);
		vl_mull(v, dfactor, v_len);
	}
	/*
	 * for convenience, remember the top two digits of the normalized
	 * divisor:
	 */
	digit_1 = v[v_len - 1];
	digit_2 = v_len > 1 ? v[v_len - 2] : 0;

	/*
	 * pu points at the current digit to be considered. j is the digit of
	 * the quotient being calculated.
	 */
	for (pu = u + u_len - 1, j = u_len - v_len; j > 0; j--, pu--) {
		/*
		 * This is simply the top two digits of the residual dividend
		 * being considered.
		 */
		temp1 = *pu * CALC_BASE + pu[-1];
		/*
		 * make a guess at the quotient. if the first two digits are
		 * the same, then start at the "maximum" possible quotient,
		 * and use the aproximation loop to get a correct guess. if
		 * they are different, divide the top digit of the divisor
		 * into the top 2 digits of the dividend.
		 */
		qx = *pu == digit_1 ?
			CALC_BASE - 1
			: temp1 / digit_1;

		/*
		 * refine the guess, by considering in the "third" digit of
		 * the divisor.
		 */
		while (1) {
			int             tval;
			tval = (temp1 - qx * digit_1) * CALC_BASE + pu[-2];
			if (qx * digit_2 <= tval)
				break;
			qx--;
		}
		/*
		 * The value of qx, is either correct, or too large by
		 * 1.(Proof is in Knuth.)
		 * 
		 * We simply scale the divisor by qx, and subtract it from the
		 * dividend.  If it causes a borrow (ie, was too large), then
		 * qx is decremented and the divisor is added back into the
		 * dividend.
		 */
		memcpy(temp_num, v, v_len);
		temp_num[v_len] = vl_mull(temp_num, qx, v_len);
		if (vl_subv(pu - v_len, temp_num, v_len + 1)) {
			qx--;
			memcpy(temp_num, v, v_len);
			temp_num[v_len] = 0;
			vl_addv(pu - v_len, temp_num, v_len + 1);
		}
		if (quo) {
			quo[j] = qx;
		}
	}

	/*
	 * Knuth Step D8: divide pu by dfactor to "denormal" the remainder.
	 */

	free(temp_num);
	vl_divl(u, dfactor, u_len);
	return 1;

}
