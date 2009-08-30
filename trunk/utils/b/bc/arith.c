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
#include <string.h>

/*- num: runtime numeric lib.

int num_compare(numb_t *op1, numb_t *op2)
    Description
        op2 is subtracted from op1 and the sign of the operation
        is returned to the caller.
    Returns:
        0    : numbers are equal
        < 0 : op2 > op1
        > 0 : op1 > op2

    int    num_iszero(numb_t *op1)
    Description:
        Determines if op1 is zero.
    Returns:
        1 if op1 is zero
        0 if op1 is not zero.



num_sqrt:	This implimentation of the square-root function, owes
		to Newtons Method of Successive Approximation:
			
			Xn+1 = 1/2 * (Xn + y/Xn).
		A number (y) is given.
		The number is expanded to the maximum of it's scale or the internal
		scale quantity.
		The above formula is tested until X is determined, such that
		x*x < y < (x+k)*(x+k), where k is the resolution.
		To simplify the implimentation, the scale of the number is "removed",
		thus the square root is calculated of an "integer" and k=1.

	This module contains two square-root functions:
	1.	integer square-roots.
		This is present to create the initial "guess"
		for Newtons Forumula.

	2.	Arbitrary Number square-roots.


	Notes:
		This module uses "high-level" number operations which require an
		excessive number of memory allocations etc.  It can easily be
		recoded to use the "kernel" routines which avoid this overhead,
		at the cost of rendering the square-root algorithm indecipherable.

		It is predicted that the current implimentation occupies > 40 % of it's
		time in perform memory allocation/normalizaion code.
	
    	Nov 9 1989:	faster version in place. about 35X faster than previous
			version.

raise:	raise a number to an integral exponent.

	This routine contains a little trick, accredited to Robert Morris,
	Linda Cherry used in DC.

	The relative bit positions of a binary number indicate a number of
	squares.
	for example:	 x**5 = (n**2)**2 * n**1.
						   = (n*n)*(n*n) * n
			may be performed by 3 multiplications.
					x*x*x*x*x
			would require 5 multiplications.
	This amounts to a substantial (logn) improvement with a relatively
	sparce number (ie. 20), although not so on a "dense" number (ie 15).



	Better performance may be derived from this routine by avoiding
	the use of the "high-level" numb_t library, and utilizing the
	low level lib directly.
*/


#include "number.h"


numb_t         *
add_num(register numb_t * o1, register numb_t * o2)
{
	digit_t        *dest, *opr;
	numb_t         *result;
	int             len;


	if ((o1 == 0) || o1->digit == 0)
		return result = dup_num(o2);

	if ((o2 == 0) || o2->digit == 0)
		return result = dup_num(o1);

	switch (is_negative(o1) * 2 + is_negative(o2)) {
	case 0:
		break;
	case 1:
		o2->sign = 0;
		result = sub_num(o1, o2);
		o2->sign = 1;
		return result;
	case 2:
		o1->sign = 0;
		result = sub_num(o2, o1);
		o1->sign = 1;
		return result;
	case 3:
		break;
	}
	/*
	 * ok, both must be positive [th: ??? I'd say same sign] if we got here
	 */

	result = get_align(o1, o2);
	len = result->len - 1;

	if ((dest = unnormal(o1, result->len, result->decpos)) == 0)
		error(1, "unable to generate unnormal");

	if ((opr = unnormal(o2, result->len, result->decpos)) == 0)
		error(1, "unable to generate unnormal");

	dest[len] = vl_addv(dest, opr, len);
	result->sign = o1->sign;
	result->digit = dest;
	FREEMEM(opr);
	return normal(result);
}




numb_t         *
sub_num(numb_t *o1, numb_t *o2)
{
	digit_t        *dest, *opr;
	int             len;
	int             sign;
	numb_t         *result;

	/*
	 * check for zeros. Note: must check o2 first to allow 0-0 to be
	 * +tive
	 */
	if ((o2 == 0) || o2->digit == 0)
		return result = dup_num(o1);
	if ((o1 == 0) || o1->digit == 0) {
		result = dup_num(o2);
		result->sign = !result->sign;
		return result;
	}
	switch (is_negative(o2) * 2 + is_negative(o1)) {
	case 0:		/* A - B */
		break;
	case 1:		/* (-A) - B == - (A + B) */
		o1->sign = 0;
		result = add_num(o1, o2);
		result->sign = !result->sign;
		o1->sign = 1;
		return result;
	case 2:		/* A - (-B) == (A + B) */
		o2->sign = 0;
		result = add_num(o1, o2);
		o2->sign = 1;
		return result;
		break;
	case 3:		/* (-A) - (-B) == - (A - B) */
		o1->sign = o2->sign = 0;
		result = sub_num(o1, o2);
		o1->sign = o2->sign = 1;
		result->sign = !result->sign;
		return result;
		break;
	}
	result = get_align(o1, o2);
	len = result->len - 1;

	if ((dest = unnormal(o1, len + 1, result->decpos)) == 0)
		error(1, "unable to generate unnormal");

	if ((opr = unnormal(o2, len + 1, result->decpos)) == 0)
		error(1, "unable to generate unnormal");

	if ((sign = vl_subv(dest, opr, len)) ) {
		digit_t        *t;
		memset(dest, 0, len);
		memcpy(dest + result->decpos - o1->decpos, o1->digit, o1->len);
		t = dest;
		dest = opr;
		opr = t;
		vl_subv(dest, opr, len);
		sign = 1;
	} else {
		sign = 0;
	}
	FREEMEM(opr);
	result->sign = sign;
	result->digit = dest;
	return normal(result);
}


int num_compare(numb_t *op1, numb_t *op2)
{
	digit_t        *x, *y;
	int             result;
	int             reverse = 1;

	if (!op1 || !op1->digit)
		return (op2 && op2->digit) ? ( op2->sign ? 1 : -1 ) : 0;

	if (!op2 || !op2->digit)
		return op1->sign ? -1 : 1;

	switch (is_negative(op1) + is_negative(op2) * 2) {
	case 0:		/* both positive */
		break;
	case 1:		/* op1 only is neg */
		return -1;
	case 2:		/* op2 only is neg */
		return 1;
	case 3:		/* both are neg   */
		reverse = -1;
		break;
	}

	if ((result = (op1->len - op1->decpos) - (op2->len - op2->decpos)))
		return reverse * (result > 0 ? 1 : -1);

	x = op1->digit + op1->len - 1;
	y = op2->digit + op2->len - 1;
	while (x >= op1->digit && y >= op2->digit) {
		if (*x > *y)
			return reverse;
		else if (*x < *y)
			return -reverse;
		x--;
		y--;
	}
	if (op1->len > op2->len)
		return reverse;
	return (op2->len == op1->len ? 0 : -1) * reverse;
}

int num_iszero(numb_t *op1)
{
	return (op1 == 0) || op1->digit == 0;
}


void
num_negate(numb_t *n)
{
	if (n != 0)
		n->sign = !n->sign;
}



numb_t         *
div_num(numb_t *dividend, numb_t *divisor)
{
	digit_t        *u_ptr;
	digit_t        *vptr;
	numb_t         *quo;

	int             ulen, vlen;
	int             scale, uoffs;
	int             u_trunc;
	int             decpos;

	if ((quo = NEWHDR()) == 0)
		no_mem("div_num1");

	if ((divisor == 0 || dividend == 0) ||
	    dividend->digit == 0 || divisor->digit == 0) {
		return quo;
	}
	/* copy the number such that there are "scale" digits in the quotient */

	scale = get_scale();

	u_trunc = 0;
	if ((decpos = dividend->decpos - divisor->decpos) < 0) {
		/* more decimal digits in denominator than numerator */
		uoffs = scale + divisor->decpos - dividend->decpos;
	} else if (scale < decpos) {
		uoffs = 0;	/* remove some of the digits from the
				 * dividend */
		u_trunc = decpos - scale;
	} else {
		uoffs = scale - decpos;	/* copy the whole thing */
	}
	ulen = uoffs + dividend->len - u_trunc + 1;
	if (ulen < divisor->len) {
		quo->len = 0;
		quo->decpos = 0;
		quo->sign = 0;
		quo->digit = 0;
		return quo;
	}
	vlen = divisor->len;

	if ((u_ptr = ALLOCMEM(digit_t, ulen)) == 0)
		error(1, "div_num: no memory");

	memcpy(u_ptr + uoffs, dividend->digit + u_trunc, dividend->len - u_trunc);

	if ((vptr = ALLOCMEM(digit_t, vlen)) == 0) {
		error(1, "div_num: no memory");
	}
	memcpy(vptr, divisor->digit, vlen);

	quo->len = ulen - vlen + 1;
	quo->decpos = scale + 1;/* ok cause last byte of divide allways 0 */

	/* either operand is negative but not both */
	quo->sign = (is_negative(dividend) != 0) ^ (is_negative(divisor) != 0);

	if ((quo->digit = ALLOCMEM(digit_t, quo->len)) == 0)
		no_mem("div_num3");

	vl_divv(quo->digit, u_ptr, vptr, ulen, vlen);

	FREEMEM(u_ptr);
	FREEMEM(vptr);

	return normal(quo);
}









numb_t         *
rem_num(numb_t *dividend, numb_t *divisor)
{
	digit_t        *u_ptr;
	digit_t        *vptr;
	numb_t         *rem;

	int             ulen, vlen;

	int             scale, uoffs;
	int             u_trunc;
	int             decpos;

	if ((rem = NEWHDR()) == 0) {
		error(1, "rem_num: no memory");
	}
	if ((dividend == 0) || dividend->digit == 0) {
		copy_num(rem, dividend);
		return rem;
	}
	if ((divisor == 0) || divisor->digit == 0) {
		error(0, "divide by 0\n");
		return rem;
	}
	/* copy the number such that there are "scale" digits in the quotient */

	scale = get_scale();

	u_trunc = 0;
	if ((decpos = dividend->decpos - divisor->decpos) < 0) {
		/* more decimal digits in denominator than numerator */
		uoffs = scale + divisor->decpos - dividend->decpos;
	} else if (scale < decpos) {
		uoffs = 0;	/* remove some of the digits from the
				 * dividend */
		u_trunc = decpos - scale;
	} else {
		uoffs = scale - decpos;	/* copy the whole thing */
	}
	ulen = uoffs + dividend->len - u_trunc + 1;
	vlen = divisor->len;


	if ((u_ptr = ALLOCMEM(digit_t, ulen)) == 0)
		error(1, "div_num: no memory");

	memcpy(u_ptr + uoffs, dividend->digit + u_trunc, dividend->len - u_trunc);

	if ((vptr = ALLOCMEM(digit_t, vlen)) == 0) {
		error(1, "div_num: no memory");
	}
	memcpy(vptr, divisor->digit, vlen);

	rem->len = ulen;
	rem->decpos = scale + max(dividend->decpos, divisor->decpos);
	rem->digit = u_ptr;
	/* the remainder takes the sign of the dividend */
	rem->sign = dividend->sign;

	vl_divv(0, u_ptr, vptr, ulen, vlen);

	FREEMEM(vptr);
	return normal(rem);
}


numb_t         *
mult_num(numb_t *op1, numb_t *op2)
{
	numb_t         *result;
	int             res_scale;

	if ((result = NEWHDR()) == 0)
		no_mem("mult_num");

	if ((op1 == 0 || op2 == 0) || op1->len == 0 || op2->len == 0) {
		result->digit = 0;
		result->len = result->decpos = result->sign = 0;
		return result;
	}
	res_scale = get_scale();
	if (res_scale < max(op1->decpos, op2->decpos))
		res_scale = max(op1->decpos, op2->decpos);

	result->len = op1->len + op2->len + 1;
	result->decpos = op1->decpos + op2->decpos;
	result->sign = (is_negative(op1) ^ is_negative(op2)) != 0;

	if (result->decpos > res_scale && result->len < res_scale) {
		/* pointless, it will generate too small of a value */
		result->digit = 0;
		result->len = result->decpos = 0;
		return result;
	}
	if ((result->digit = ALLOCMEM(digit_t, result->len)) == 0)
		no_mem("mult_num");

	vl_mulv(result->digit, op1->digit, op2->digit, op1->len, op2->len);
	if (result->decpos > res_scale) {	/* truncate digits to make
						 * reasonable */
		int             t = result->decpos - res_scale;
		memmove(result->digit, result->digit + t, result->len - t);
		memset(result->digit + result->len - t, 0, t);
		result->decpos = res_scale;
	}
	return normal(result);
}






numb_t         *
raise_num(numb_t *op1, numb_t *op2)
{
	numb_t         *y, *r0, *r1;	/* result and 2 temporary registers */
	int             exp;
	int             sign = 0;	/* for negative exponents */
	int             i, j;

	exp = numtod(op2);	/* only an integer is required */
	if (exp < 0) {
		sign = 1;
		exp = -exp;
	}
	y = dtonum(1);		/* get a value of 1 to start */
	if (exp == 0)
		return y;

	for (i = 0; exp != 0; i++, exp >>= 1) {
		if (exp & 01) {	/* if this bit is set */
			for (j = 0, r0 = 0; j < i; j++) {
				if (r0 != 0) {	/* don't accidently delete
						 * the operand */
					r1 = mult_num(r0, r0);
					delete_num(r0);
				} else {
					r1 = mult_num(op1, op1);
				}
				r0 = r1;
			}
			r1 = y;
			y = mult_num(r1, r0 == 0 ? op1 : r0);
			delete_num(r1);
			if (r0 != 0)
				delete_num(r0);
		}
	}
	if (sign) {
		r0 = dtonum(1);	/* generate a one */
		r1 = y;
		y = div_num(r0, r1);
		delete_num(r0);
		delete_num(r1);
	}
	return normal(y);
}








static int
int_sqrt(unsigned y)
{
	long            x;
	long            sqx;

	if (y == 0 || y == 1)
		return y;
	x = y / 2;
	while (1) {
		if ((sqx = x * x) == y) {
			return (int) x;
		}
		if (sqx < y && (sqx + (x << 1) + 1) > y) {
			return (int) x;
		}
		x = (x + y / x) / 2;
	}
}





numb_t         *
num_sqrt(numb_t *y)
{
	digit_t        *yval;	/* expanded version of y */
	digit_t        *r0, *r1, *r2;	/* temporary values for evaluation */
	digit_t        *x;	/* resulting square root */
	numb_t         *X;
	int             scale;
	int             xlen, ylen;
	int             i, j;

	if ((y == 0) || y->digit == 0)
		return 0;	/* clear out zeros */

	scale = get_scale();
	scale = max(scale, y->decpos);

	scale *= 2;		/* expand to allow scale digits result */

	ylen = scale + y->len - y->decpos;

	ylen = roundew(ylen);

	/* only do one alloc instead of 4 */

	if ((yval = ALLOCMEM(digit_t, (ylen + 1) * 4)) == 0)
		error(1, "sqrt: no memory");

	r0 = yval + ylen + 1;
	r1 = r0 + ylen + 1;
	r2 = r1 + ylen + 1;
	memcpy(yval + scale - y->decpos, y->digit, y->len);	/* initial yval */

	/* generate an initial guess at the square root */
	i = y->digit[y->len - 1];
	if (y->len > 1) {
		i *= CALC_BASE;
		i += y->digit[y->len - 2];
	}
	if (ylen <= 2) {
		FREEMEM(yval);
		j = int_sqrt(i);
		if (isodd(y->decpos))
			j *= 10;
		X = dtonum(j);	/* this is the sqrt */
		X->decpos = scale / 2;
		X->sign = 0;
		return normal(X);
	}
	xlen = ylen / 2;

	/* make big to easy compute */
	if ((x=ALLOCMEM(digit_t, ylen + 1)) == 0)
		error(1, "sqrt: no memory");
	ltov(int_sqrt(i), x + xlen - 1, 1);


	/* initial guess */
	while (1) {
		memset(r0, 0, ylen);
		vl_mulv(r0, x, x, xlen, xlen);	/* square x */
		/* are equal */
		if ((j = vl_cmp(r0, yval, ylen)) == 0) {
			break;
		}
		if (j < 0) {	/* less than, is (x+1)*(x+1) > y */
			memset(r0, 0, ylen);
			memset(r2, 0, ylen);
			r0[0] = 1;	/* resolution */
			vl_addv(r0, x, xlen);
			vl_mulv(r2, r0, r0, xlen, xlen);
			if ((j = vl_cmp(r2, yval, ylen)) > 0) {
				break;
			} else if (j == 0) {
				memcpy(x, r0, xlen);
				break;
			}
		}
		memset(r1, 0, ylen);
		memset(r2, 0, ylen);

		memcpy(r0, yval, ylen);	/* y / xn */
		memcpy(r1, x, xlen);
		for (i = ylen - 1; i >= 0 && r0[i] == 0; i--);
		for (j = xlen - 1; j >= 0 && r1[j] == 0; j--);
		if (j < 0)
			error(1, "sqrt: no resolution\n");
		/*
		 * approximation formulae:		x' = (x + y/x) / 2;
		 */
		vl_divv(r2, r0, r1, i + 2, j + 1);
		vl_addv(r2 + 1, x, ylen);
		vl_divl(r2 + 1, 2, ylen);
		memcpy(x, r2 + 1, xlen);	/* shave a bit off */
	}
	FREEMEM(yval);		/* note this deletes r0,r1,r2 as well */
	if ((X = NEWHDR()) == 0)
		no_mem("num_sqrt");
	X->sign = 0;
	X->digit = x;
	X->len = ylen + 1;
	X->decpos = scale / 2;
	return normal(X);
}
