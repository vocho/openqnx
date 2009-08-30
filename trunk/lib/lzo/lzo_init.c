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




/* lzo_init.c -- initialization of the LZO library

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#include "lzo_conf.h"
#include "lzo_util.h"

#include <stdio.h>


#if 0
#  define IS_SIGNED(type)		(((type) (1ul << (8 * sizeof(type) - 1))) < 0)
#  define IS_UNSIGNED(type)		(((type) (1ul << (8 * sizeof(type) - 1))) > 0)
#else
#  define IS_SIGNED(type)		(((type) (-1)) < ((type) 0))
#  define IS_UNSIGNED(type)		(((type) (-1)) > ((type) 0))
#endif


/***********************************************************************
// Runtime check of the assumptions about the size of builtin types,
// memory model, byte order and other low-level constructs.
//
// We are really paranoid here - LZO should either fail (or crash)
// at startup or not at all.
//
// Because of inlining much of these functions evaluates to nothing.
************************************************************************/

static lzo_bool schedule_insns_bug(void);	/* avoid inlining */
static lzo_bool strength_reduce_bug(int *);	/* avoid inlining */


#if 0 || defined(LZO_DEBUG)
static lzo_bool __lzo_assert_fail(const char *s, unsigned line)
{
#if defined(__palmos__)
	printf("LZO assertion failed in line %u: '%s'\n",line,s);
#else
	fprintf(stderr,"LZO assertion failed in line %u: '%s'\n",line,s);
#endif
	return 0;
}
#  define __lzo_assert(x)	((x) ? 1 : __lzo_assert_fail(#x,__LINE__))
#else
#  define __lzo_assert(x)	((x) ? 1 : 0)
#endif


/***********************************************************************
// The next two functions should get completely optimized out of existance.
// Some assertions are redundant - but included for clarity.
************************************************************************/

static lzo_bool basic_integral_check(void)
{
	lzo_bool r = 1;
	lzo_bool sanity;

	/* paranoia */
	r &= __lzo_assert(CHAR_BIT == 8);
	r &= __lzo_assert(sizeof(char) == 1);
	r &= __lzo_assert(sizeof(short) >= 2);
	r &= __lzo_assert(sizeof(long) >= 4);
	r &= __lzo_assert(sizeof(int) >= sizeof(short));
	r &= __lzo_assert(sizeof(long) >= sizeof(int));

	r &= __lzo_assert(sizeof(lzo_uint32) >= 4);
	r &= __lzo_assert(sizeof(lzo_uint32) >= sizeof(unsigned));
#if defined(__LZO_STRICT_16BIT)
	r &= __lzo_assert(sizeof(lzo_uint) == 2);
#else
	r &= __lzo_assert(sizeof(lzo_uint) >= 4);
	r &= __lzo_assert(sizeof(lzo_uint) >= sizeof(unsigned));
#endif

#if defined(SIZEOF_UNSIGNED)
	r &= __lzo_assert(SIZEOF_UNSIGNED == sizeof(unsigned));
#endif
#if defined(SIZEOF_UNSIGNED_LONG)
	r &= __lzo_assert(SIZEOF_UNSIGNED_LONG == sizeof(unsigned long));
#endif
#if defined(SIZEOF_UNSIGNED_SHORT)
	r &= __lzo_assert(SIZEOF_UNSIGNED_SHORT == sizeof(unsigned short));
#endif
#if !defined(__LZO_IN_MINILZO)
#if defined(SIZEOF_SIZE_T)
	r &= __lzo_assert(SIZEOF_SIZE_T == sizeof(size_t));
#endif
#endif

	/* assert the signedness of our integral types */
	sanity = IS_UNSIGNED(unsigned short) && IS_UNSIGNED(unsigned) &&
	         IS_UNSIGNED(unsigned long) &&
	         IS_SIGNED(short) && IS_SIGNED(int) && IS_SIGNED(long);
	if (sanity)
	{
		r &= __lzo_assert(IS_UNSIGNED(lzo_uint32));
		r &= __lzo_assert(IS_UNSIGNED(lzo_uint));
		r &= __lzo_assert(IS_SIGNED(lzo_int32));
		r &= __lzo_assert(IS_SIGNED(lzo_int));

		r &= __lzo_assert(INT_MAX    == LZO_STYPE_MAX(sizeof(int)));
		r &= __lzo_assert(UINT_MAX   == LZO_UTYPE_MAX(sizeof(unsigned)));
		r &= __lzo_assert(LONG_MAX   == LZO_STYPE_MAX(sizeof(long)));
		r &= __lzo_assert(ULONG_MAX  == LZO_UTYPE_MAX(sizeof(unsigned long)));
		r &= __lzo_assert(SHRT_MAX   == LZO_STYPE_MAX(sizeof(short)));
		r &= __lzo_assert(USHRT_MAX  == LZO_UTYPE_MAX(sizeof(unsigned short)));
		r &= __lzo_assert(LZO_UINT32_MAX == LZO_UTYPE_MAX(sizeof(lzo_uint32)));
		r &= __lzo_assert(LZO_UINT_MAX   == LZO_UTYPE_MAX(sizeof(lzo_uint)));
#if !defined(__LZO_IN_MINILZO)
		r &= __lzo_assert(SIZE_T_MAX     == LZO_UTYPE_MAX(sizeof(size_t)));
#endif
	}

#if 0
	/* for some reason this fails on a Cray ??? */
	r &= __lzo_assert(LZO_BYTE(257) == 1);
	r &= __lzo_assert(LZO_USHORT(65537L) == 1);
#endif

	return r;
}


static lzo_bool basic_ptr_check(void)
{
	lzo_bool r = 1;
	lzo_bool sanity;

	r &= __lzo_assert(sizeof(char *) >= sizeof(int));
	r &= __lzo_assert(sizeof(lzo_byte *) >= sizeof(char *));

	r &= __lzo_assert(sizeof(lzo_voidp) == sizeof(lzo_byte *));
	r &= __lzo_assert(sizeof(lzo_voidp) == sizeof(lzo_voidpp));
	r &= __lzo_assert(sizeof(lzo_voidp) == sizeof(lzo_bytepp));
	r &= __lzo_assert(sizeof(lzo_voidp) >= sizeof(lzo_uint));

	r &= __lzo_assert(sizeof(lzo_ptr_t) == sizeof(lzo_voidp));
	r &= __lzo_assert(sizeof(lzo_ptr_t) >= sizeof(lzo_uint));

	r &= __lzo_assert(sizeof(lzo_ptrdiff_t) >= 4);
	r &= __lzo_assert(sizeof(lzo_ptrdiff_t) >= sizeof(ptrdiff_t));

#if defined(SIZEOF_CHAR_P)
	r &= __lzo_assert(SIZEOF_CHAR_P == sizeof(char *));
#endif
#if defined(SIZEOF_PTRDIFF_T)
	r &= __lzo_assert(SIZEOF_PTRDIFF_T == sizeof(ptrdiff_t));
#endif

	/* assert the signedness of our integral types */
	sanity = IS_UNSIGNED(unsigned short) && IS_UNSIGNED(unsigned) &&
	         IS_UNSIGNED(unsigned long) &&
	         IS_SIGNED(short) && IS_SIGNED(int) && IS_SIGNED(long);
	if (sanity)
	{
		r &= __lzo_assert(IS_UNSIGNED(lzo_ptr_t));
		r &= __lzo_assert(IS_UNSIGNED(lzo_moff_t));
		r &= __lzo_assert(IS_SIGNED(lzo_ptrdiff_t));
		r &= __lzo_assert(IS_SIGNED(lzo_sptr_t));
	}

	return r;
}


/***********************************************************************
//
************************************************************************/

static lzo_bool ptr_check(void)
{
	lzo_bool r = 1;
	int i;
	char _wrkmem[10 * sizeof(lzo_byte *) + sizeof(lzo_align_t)];
	lzo_byte *wrkmem;
	const lzo_bytepp dict;
	unsigned char x[4 * sizeof(lzo_align_t)];
	long d;
	lzo_align_t a;

	for (i = 0; i < (int) sizeof(x); i++)
		x[i] = LZO_BYTE(i);

	wrkmem = (lzo_byte *) LZO_PTR_ALIGN_UP(_wrkmem,sizeof(lzo_align_t));
	dict = (const lzo_bytepp) wrkmem;

	d = (long) ((const lzo_bytep) dict - (const lzo_bytep) _wrkmem);
	r &= __lzo_assert(d >= 0);
	r &= __lzo_assert(d < (long) sizeof(lzo_align_t));

	memset(&a,0xff,sizeof(a));
	r &= __lzo_assert(a.a_ushort == USHRT_MAX);
	r &= __lzo_assert(a.a_uint == UINT_MAX);
	r &= __lzo_assert(a.a_ulong == ULONG_MAX);
	r &= __lzo_assert(a.a_lzo_uint == LZO_UINT_MAX);

	/* sanity check of the memory model */
	if (r == 1)
	{
		for (i = 0; i < 8; i++)
			r &= __lzo_assert((const lzo_voidp) (&dict[i]) == (const lzo_voidp) (&wrkmem[i * sizeof(lzo_byte *)]));
	}

	/* check BZERO8_PTR and that NULL == 0 */
	memset(&a,0,sizeof(a));
	r &= __lzo_assert(a.a_charp == NULL);
	r &= __lzo_assert(a.a_lzo_bytep == NULL);
	r &= __lzo_assert(NULL == 0);
	if (r == 1)
	{
		for (i = 0; i < 10; i++)
			dict[i] = wrkmem;
		BZERO8_PTR(dict+1,sizeof(dict[0]),8);
		r &= __lzo_assert(dict[0] == wrkmem);
		for (i = 1; i < 9; i++)
			r &= __lzo_assert(dict[i] == NULL);
		r &= __lzo_assert(dict[9] == wrkmem);
	}

	/* check that the pointer constructs work as expected */
	if (r == 1)
	{
		unsigned k = 1;
		const unsigned n = (unsigned) sizeof(lzo_uint32);
		lzo_byte *p0;
		lzo_byte *p1;

		k += __lzo_align_gap(&x[k],n);
		p0 = (lzo_bytep) &x[k];
#if defined(PTR_LINEAR)
		r &= __lzo_assert((PTR_LINEAR(p0) & (n-1)) == 0);
#else
		r &= __lzo_assert(n == 4);
		r &= __lzo_assert(PTR_ALIGNED_4(p0));
#endif

		r &= __lzo_assert(k >= 1);
		p1 = (lzo_bytep) &x[1];
		r &= __lzo_assert(PTR_GE(p0,p1));

		r &= __lzo_assert(k < 1+n);
		p1 = (lzo_bytep) &x[1+n];
		r &= __lzo_assert(PTR_LT(p0,p1));

		/* now check that aligned memory access doesn't core dump */
		if (r == 1)
		{
			lzo_uint32 v0 = * (lzo_uint32 *) &x[k];
			lzo_uint32 v1 = * (lzo_uint32 *) &x[k+n];

			r &= __lzo_assert(v0 > 0);
			r &= __lzo_assert(v1 > 0);
		}
	}

	return r;
}


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(int)
_lzo_config_check(void)
{
	lzo_bool r = 1;
	int i;
	union {
		lzo_uint32 a;
		unsigned short b;
		lzo_uint32 aa[4];
		unsigned char x[4*sizeof(lzo_align_t)];
	} u;

#if 0
	/* paranoia - the following is guaranteed by definition anyway */
	r &= __lzo_assert((const void *)&u == (const void *)&u.a);
	r &= __lzo_assert((const void *)&u == (const void *)&u.b);
	r &= __lzo_assert((const void *)&u == (const void *)&u.x[0]);
	r &= __lzo_assert((const void *)&u == (const void *)&u.aa[0]);
#endif

	r &= basic_integral_check();
	r &= basic_ptr_check();
	if (r != 1)
		return LZO_E_ERROR;

	for (i = 0; i < (int) sizeof(u.x); i++)
		u.x[i] = LZO_BYTE(i);

#if 0
	/* check if the compiler correctly casts signed to unsigned */
	r &= __lzo_assert( (int) (unsigned char) ((char) -1) == 255);
#endif

	/* check LZO_BYTE_ORDER */
#if defined(LZO_BYTE_ORDER)
	if (r == 1)
	{
#  if (LZO_BYTE_ORDER == LZO_LITTLE_ENDIAN)
		lzo_uint32 a = (lzo_uint32) (u.a & LZO_0xffffffffL);
		unsigned short b = (unsigned short) (u.b & 0xffff);
		r &= __lzo_assert(a == 0x03020100L);
		r &= __lzo_assert(b == 0x0100);
#  elif (LZO_BYTE_ORDER == LZO_BIG_ENDIAN)
		lzo_uint32 a = u.a >> (8 * sizeof(u.a) - 32);
		unsigned short b = u.b >> (8 * sizeof(u.b) - 16);
		r &= __lzo_assert(a == 0x00010203L);
		r &= __lzo_assert(b == 0x0001);
#  else
#    error invalid LZO_BYTE_ORDER
#  endif
	}
#endif

	/* check that unaligned memory access works as expected */
#if defined(LZO_UNALIGNED_OK_2)
	r &= __lzo_assert(sizeof(short) == 2);
	if (r == 1)
	{
		unsigned short b[4];

		for (i = 0; i < 4; i++)
			b[i] = * (const unsigned short *) &u.x[i];

#  if (LZO_BYTE_ORDER == LZO_LITTLE_ENDIAN)
		r &= __lzo_assert(b[0] == 0x0100);
		r &= __lzo_assert(b[1] == 0x0201);
		r &= __lzo_assert(b[2] == 0x0302);
		r &= __lzo_assert(b[3] == 0x0403);
#  elif (LZO_BYTE_ORDER == LZO_BIG_ENDIAN)
		r &= __lzo_assert(b[0] == 0x0001);
		r &= __lzo_assert(b[1] == 0x0102);
		r &= __lzo_assert(b[2] == 0x0203);
		r &= __lzo_assert(b[3] == 0x0304);
#  endif
	}
#endif

#if defined(LZO_UNALIGNED_OK_4)
	r &= __lzo_assert(sizeof(lzo_uint32) == 4);
	if (r == 1)
	{
		lzo_uint32 a[4];

		for (i = 0; i < 4; i++)
			a[i] = * (const lzo_uint32 *) &u.x[i];

#  if (LZO_BYTE_ORDER == LZO_LITTLE_ENDIAN)
		r &= __lzo_assert(a[0] == 0x03020100L);
		r &= __lzo_assert(a[1] == 0x04030201L);
		r &= __lzo_assert(a[2] == 0x05040302L);
		r &= __lzo_assert(a[3] == 0x06050403L);
#  elif (LZO_BYTE_ORDER == LZO_BIG_ENDIAN)
		r &= __lzo_assert(a[0] == 0x00010203L);
		r &= __lzo_assert(a[1] == 0x01020304L);
		r &= __lzo_assert(a[2] == 0x02030405L);
		r &= __lzo_assert(a[3] == 0x03040506L);
#  endif
	}
#endif

#if defined(LZO_ALIGNED_OK_4)
	r &= __lzo_assert(sizeof(lzo_uint32) == 4);
#endif

	r &= __lzo_assert(lzo_sizeof_dict_t == sizeof(lzo_dict_t));

	/* save space and don't require linking in the lzo_adler32() function */
#if defined(__LZO_IN_MINLZO)
	/* check the lzo_adler32() function */
	if (r == 1)
	{
		lzo_uint32 adler;
		adler = lzo_adler32(0, NULL, 0);
		adler = lzo_adler32(adler, lzo_copyright(), 200);
		r &= __lzo_assert(adler == 0x7ea34377L);
	}
#endif

	/* check for the gcc schedule-insns optimization bug */
	if (r == 1)
	{
		r &= __lzo_assert(!schedule_insns_bug());
	}

	/* check for the gcc strength-reduce optimization bug */
	if (r == 1)
	{
		static int x[3];
		static unsigned xn = 3;
		register unsigned j;

		for (j = 0; j < xn; j++)
			x[j] = (int)j - 3;
		r &= __lzo_assert(!strength_reduce_bug(x));
	}

	/* now for the low-level pointer checks */
	if (r == 1)
	{
		r &= ptr_check();
	}

	return r == 1 ? LZO_E_OK : LZO_E_ERROR;
}


static lzo_bool schedule_insns_bug(void)
{
#if defined(__BOUNDS_CHECKING_ON) || defined(__CHECKER__)
	/* for some reason checker complains about uninitialized memory access */
	return 0;
#else
	const int clone[] = {1, 2, 0};
	const int *q;
	q = clone;
	return (*q) ? 0 : 1;
#endif
}


static lzo_bool strength_reduce_bug(int *x)
{
	return x[0] != -3 || x[1] != -2 || x[2] != -1;
}


/***********************************************************************
//
************************************************************************/

int __lzo_init_done = 0;

LZO_PUBLIC(int)
__lzo_init2(unsigned v, int s1, int s2, int s3, int s4, int s5,
                        int s6, int s7, int s8, int s9)
{
	int r;

	__lzo_init_done = 1;

	if (v == 0)
		return LZO_E_ERROR;

	r = (s1 == -1 || s1 == (int) sizeof(short)) &&
	    (s2 == -1 || s2 == (int) sizeof(int)) &&
	    (s3 == -1 || s3 == (int) sizeof(long)) &&
	    (s4 == -1 || s4 == (int) sizeof(lzo_uint32)) &&
	    (s5 == -1 || s5 == (int) sizeof(lzo_uint)) &&
	    (s6 == -1 || s6 == (int) lzo_sizeof_dict_t) &&
	    (s7 == -1 || s7 == (int) sizeof(char *)) &&
	    (s8 == -1 || s8 == (int) sizeof(lzo_voidp)) &&
	    (s9 == -1 || s9 == (int) sizeof(lzo_compress_t));
	if (!r)
		return LZO_E_ERROR;

	r = _lzo_config_check();
	if (r != LZO_E_OK)
		return r;

	return r;
}


/***********************************************************************
// backward compatibility with v1.01
************************************************************************/

#if !defined(__LZO_IN_MINILZO)

LZO_EXTERN(int)
__lzo_init(unsigned v,int s1,int s2,int s3,int s4,int s5,int s6,int s7);

LZO_PUBLIC(int)
__lzo_init(unsigned v,int s1,int s2,int s3,int s4,int s5,int s6,int s7)
{
	if (v == 0 || v > 0x1010)
		return LZO_E_ERROR;
	return __lzo_init2(v,s1,s2,s3,s4,s5,-1,-1,s6,s7);
}

#endif


/*
vi:ts=4
*/
