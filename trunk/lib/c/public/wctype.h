/* wctype.h standard header */
#ifndef _WCTYPE
#define _WCTYPE
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

_C_STD_BEGIN

		/* MACROS */
#define WEOF	((wint_t)(-1))

		/* TYPE DEFINITIONS */
typedef _Sizet wctrans_t;

 #if !defined(__BORLANDC__)
typedef _Sizet wctype_t;

 #elif !defined(_WCTYPE_T_DEFINED)
  #define _WCTYPE_T_DEFINED
typedef _Wchart wctype_t;
 #endif /* !defined(__BORLANDC__) */

 #ifndef _WINTT
typedef long _Wintt;
  #define _WINTT
typedef _Wintt wint_t;
 #endif /* _WINTT */

		/* ctype DECLARATIONS */
 #include <xwcc.h>

_C_LIB_DECL
wctrans_t wctrans(const char *);
wctype_t wctype(const char *);
_END_C_LIB_DECL

 #if defined(__cplusplus) && !defined(_NO_CPP_INLINES)
		/* INLINES, FOR C++ */
_C_LIB_DECL
inline wint_t (towctrans)(wint_t _Wc, wctrans_t _Off)
	{	// translate by category
	return (_Towctrans(_Wc, _Off));
	}
_END_C_LIB_DECL

 #else /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */
	/* DECLARATIONS AND MACRO OVERRIDES, FOR C */
_C_LIB_DECL
wint_t (towctrans)(wint_t, wctrans_t);
_END_C_LIB_DECL
 #endif /* defined(__cplusplus) && !defined(_NO_CPP_INLINES) */

_C_STD_END
#endif /* _WCTYPE */

#ifdef _STD_USING
using _CSTD wctrans_t; using _CSTD wctype_t; using _CSTD wint_t;

using _CSTD iswalnum; using _CSTD iswalpha; using _CSTD iswcntrl;
using _CSTD iswctype; using _CSTD iswdigit; using _CSTD iswgraph;
using _CSTD iswlower; using _CSTD iswprint; using _CSTD iswpunct;
using _CSTD iswspace; using _CSTD iswupper; using _CSTD iswxdigit;
using _CSTD towctrans; using _CSTD towlower; using _CSTD towupper;
using _CSTD wctrans; using _CSTD wctype;

 #if _HAS_C9X
using _CSTD iswblank;
 #endif /* _IS_C9X */

#endif /* _STD_USING */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("wctype.h $Rev: 153052 $"); */
