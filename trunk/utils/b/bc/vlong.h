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




#ifndef vlong_h
#define vlong_h

typedef unsigned char digit_t;

enum { CALC_BASE = 100 };

/*- old names */
#if 0
#define add_scalar vl_addl
#define add_vector vl_addv
#define sub_scalar vl_subl
#define sub_vector vl_subv
#define div_scalar vl_divl
#define div_vector vl_divv
#define mult_scalar vl_mull
#define mult_vector vl_mulv
#define dtovect     ltov
#define compare_vector vl_cmp
#define iszero_vector vl_iszero
#define negate_vector vl_neg
#endif

int vl_addl(digit_t *dest, long scalar, int n);
int vl_divl(digit_t *val, long scale, int n);
int vl_subl(digit_t *dest, long scalar, int n);
int vl_mull(digit_t *dest, long scale, int n);
int vl_neg(digit_t *n, int len);
int vl_addv(digit_t *dest, digit_t *src, int len);
int vl_subv(digit_t *dest, digit_t *src, int len);
int vl_mulv(digit_t *prod, digit_t *op1, digit_t *op2, int len1, int len2);
int vl_divv(digit_t *quo, digit_t *u, digit_t *v, int u_len, int v_len);
int vl_iszero(digit_t *v, int n);
int vl_cmp(digit_t *u, digit_t *v, int n);
int ltov(long val, digit_t *v, int n);

#endif
