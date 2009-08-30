/* xxstod.h -- _[W]Sto[d f ld] common functionality */

#define D16TO7	FLIT(268435456.0)	/* 16^7 */
#define D10TO9	FLIT(1e9)			/* 10^9 */
#define NLONG	((FBITS + 27) / 28)	/* 7 * NLONG == max hexadecimal digits */

/*
FTYPE _Stodx(const CTYPE *s, CTYPE **endptr, long pten, int *perr)
 */
	{	/* convert string to FTYPE, with checking */
	FTYPE x;
	long lo[NLONG + 1];
	const CTYPE *s0 = s;
	int code = CNAME(Stopfx)(&s, endptr);
	const int neg = code & FL_NEG;

	extern FTYPE FNAME(Dtentox)(FTYPE, long, int *);

	if (perr != 0)
		*perr = 0;
	if ((code &= ~FL_NEG) == FL_DEC)
		{	/* parse decimal format */
		const int nlo = CNAME(Stoflt)(s0, s, endptr, lo, NLONG);
		int i;

		if (nlo == 0)
			x = FLIT(0.0);
		else
#ifdef __QNX__ 
			/* PR44053 - make sure we do the rounding correctly in all cases.
			 *   the original Dinkum code does all the intermediate calculations
			 *   with positive numbers, and then switches the sign at the end if
			 *   necessary.  However, if the rounding mode is FE_UPWARD or
			 *   FE_DOWNWARD, this leads to rounding in the wrong direction.  To
			 *   solve this problem, we do the intermediate calculations with the
			 *   correct sign right from the start.
			 */
			for (i = 1, x = (FTYPE)( neg ? -lo[1] : lo[1] ); i < nlo; )
				x = x * D10TO9 + (FTYPE)( neg ? -lo[++i] : lo[++i] );
#else
			for (i = 1, x = (FTYPE)lo[1]; i < nlo; )
				x = x * D10TO9 + (FTYPE)lo[++i];
#endif

		x = FNAME(Dtentox)(x, pten + lo[0], perr);
		}
	else if (code == FL_HEX)
		{	/* parse hexadecimal format */
		const int nlo = CNAME(Stoxflt)(s0, s, endptr, lo, NLONG);
		int i;

		if (nlo == 0)
			x = FLIT(0.0);
		else
#ifdef __QNX__ /* PR44053 */
			for (i = 1, x = (FTYPE)( neg ? -lo[1] : lo[1] ); i < nlo; )
				x = x * D16TO7 + (FTYPE)( neg ? -lo[++i] : lo[++i] );
#else
			for (i = 1, x = (FTYPE)lo[1]; i < nlo; )
				x = x * D16TO7 + (FTYPE)lo[++i];
#endif

		FNAME(Dscale)(&x, lo[0]);
		x = FNAME(Dtentox)(x, pten, perr);
		}
        
	else if (code == FL_INF)
        {
#ifdef __QNX__ /* PR44053 */
        x = ( neg ? -FCONST(Inf) : FCONST(Inf) );
#else
        x = FCONST(Inf);
#endif        
        }
        
	else if (code == FL_NAN)
        {
#ifdef __QNX__ /* PR44053 */
        x = ( neg ? -FCONST(Nan) : FCONST(Nan) );
#else
        x = FCONST(Nan);
#endif
        }
             
	else
		x = 0;	/* code == FL_ERR */
        
#ifdef __QNX__ /* PR44053 */
	if (neg && x == FLIT(0.0)) {
		x = -x;
	}
	return x;
#else
	return (neg != 0 ? -x : x);
#endif
	}

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("xxstod.h $Rev: 200565 $"); */
