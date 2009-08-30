/* xwstdio.h internal header */
#ifndef _XWSTDIO
#define _XWSTDIO
#include <xstdio.h>
#include <xwchar.h>
_C_STD_BEGIN

		/* macros for _WScanf and friends */
#define WGET(px)	\
	(++(px)->nchar, (*(px)->pfn)((px)->arg, 0, 1))
#define WGETN(px)	(wint_t)(0 <= --(px)->nget ? WGET(px) \
	: (++(px)->nchar, WEOF))
#define WUNGET(px, ch)	\
	(--(px)->nchar, (*(px)->pfn)((px)->arg, ch, 0))
#define WUNGETN(px, ch)	do if ((int)(ch) != WEOF) WUNGET(px, ch); \
	else --(px)->nchar; while (0)

		/* type definitions */
#ifdef __QNX__
struct _WPargs {
	wchar_t *s;
	size_t max;
};
#endif
typedef struct
	{	/* print formatting information */
	union
		{	/* long or long double value */
		_Longlong li;
		long double ld;
		} v;
	void *(*pfn)(void *, const wchar_t *, size_t);
	void *arg;
	wchar_t *s;
	int n0, nz0, n1, nz1, n2, nz2, nchar, prec, width;
	unsigned short flags;
	wchar_t qual;
	wchar_t sep;	/* added for _HAS_FIXED_POINT %v */
	} _WPft;

typedef struct
	{	/* scan formatting information */
	wint_t (*pfn)(void *, wint_t, int);
	void *arg;
	va_list ap;
	const wchar_t *s;
	int nchar, nget, width;
	size_t prec;
	wchar_t qual;
	char noconv, stored;
	char secure;	/* added for _HAS_SECURE_LIB */
	wchar_t sep;	/* added for _HAS_FIXED_POINT %v */
	} _WSft;

		/* declarations */
_C_LIB_DECL
int _WFrprep(FILE *);
int _WFwprep(FILE *);
void _WGenld(_WPft *, wchar_t, wchar_t *, short, short);
int _WGetfld(_WSft *);
int _WGetfloat(_WSft *, void *);
int _WGetint(_WSft *, void *);
int _WGetstr(_WSft *, int);
void _WLdtob(_WPft *, wchar_t);
void _WLitob(_WPft *, wchar_t);
int _WPrintf(void *(*)(void *, const wchar_t *, size_t),
	void *, const wchar_t *, va_list);
int _WPutstr(_WPft *, const char *);
int _WPutfld(_WPft *, va_list *, wchar_t, wchar_t *);
int _WPuttxt(_WPft *, const wchar_t *);
int _WScanf(wint_t (*)(void *, wint_t, int),
	void *, const wchar_t *, va_list, int);
#ifdef __QNX__
void *_Wfprout(void *, const wchar_t *, size_t);        /* FILE */
//void *_Wsprout(void *, const wchar_t *, size_t);        /* struct _Wpargs */
#endif
_END_C_LIB_DECL
_C_STD_END
#endif /* _XWSTDIO */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("xwstdio.h $Rev: 153052 $"); */
