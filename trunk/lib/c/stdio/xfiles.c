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



/*

 


Also copyright P.J. Plauger - see bottom of file for details.
*/

/* _Files data object */
#include "xstdio.h"
#ifdef __QNX__
#include <unistd.h>
#endif
_STD_BEGIN

/* standard error buffer */
#ifdef __QNX__
extern FILE _Stdin, _Stdout, _Stderr;

static unsigned char ebuf[80];
#else
static unsigned char ebuf[80] = {0};
#endif

 #ifdef _WIN32_WCE
/* the standard streams */
FILE _Stdin = {	/* standard input */
	_MOPENR, 0, 0,
	&_Stdin._Cbuf, &_Stdin._Cbuf + 1, &_Stdin._Cbuf,
	&_Stdin._Cbuf, &_Stdin._Cbuf,
	_Stdin._Back + sizeof (_Stdin._Back),
	_Stdin._WBack + sizeof (_Stdin._WBack) / sizeof (wchar_t)};

FILE _Stdout = {	/* standard output */
	_MOPENW | _MCREAT | _MOPENA, 1, 0,
	&_Stdout._Cbuf, &_Stdout._Cbuf + 1, &_Stdout._Cbuf,
	&_Stdout._Cbuf, &_Stdout._Cbuf,
	_Stdout._Back + sizeof (_Stdout._Back),
	_Stdout._WBack + sizeof (_Stdout._WBack) / sizeof (wchar_t)};

FILE _Stderr = {	/* standard error */
	_MOPENW | _MCREAT | _MOPENA | _MNBF, 2, 0,
	ebuf, ebuf + sizeof (ebuf), ebuf,
	ebuf, ebuf,
	_Stderr._Back + sizeof (_Stderr._Back),
	_Stderr._WBack + sizeof (_Stderr._WBack) / sizeof (wchar_t)};

/* the array of stream pointers */
FILE *_Files[FOPEN_MAX] = {&_Stdin, &_Stdout, &_Stderr};

 #else /* _WIN32_WCE */
/* the standard streams */
FILE _Stdin = {	/* standard input */
#ifdef __QNX__
	_MOPENR, 0,
#else
	_MOPENR, 0, 0,
#endif
	&_Stdin._Cbuf, &_Stdin._Cbuf + 1, &_Stdin._Cbuf,
	&_Stdin._Cbuf, &_Stdin._Cbuf,
	_Stdin._Back + sizeof (_Stdin._Back),
#ifdef __QNX__
	_Stdin._WBack + sizeof (_Stdin._WBack) / sizeof (wchar_t),
	{ 0 }, 0, 0, 0, { 0 }, &_Stdout,
#else
	_Stdin._WBack + sizeof (_Stdin._WBack) / sizeof (wchar_t)
#endif
	};

FILE _Stdout = {	/* standard output */
#ifdef __QNX__
	_MOPENW, 1,
#else
	_MOPENW, 1, 1,
#endif
	&_Stdout._Cbuf, &_Stdout._Cbuf + 1, &_Stdout._Cbuf,
	&_Stdout._Cbuf, &_Stdout._Cbuf,
	_Stdout._Back + sizeof (_Stdout._Back),
#ifdef __QNX__
	_Stdout._WBack + sizeof (_Stdout._WBack) / sizeof (wchar_t),
	{ 0 }, 0, 0, 0, { 0 }, &_Stderr,
#else
	_Stdout._WBack + sizeof (_Stdout._WBack) / sizeof (wchar_t)
#endif
	};

FILE _Stderr = {	/* standard error */
#ifdef __QNX__
	_MOPENW | _MNBF, 2,
#else
	_MOPENW | _MNBF, 2, 2,
#endif
	ebuf, ebuf + sizeof (ebuf), ebuf,
	ebuf, ebuf,
	_Stderr._Back + sizeof (_Stderr._Back),
	_Stderr._WBack + sizeof (_Stderr._WBack) / sizeof (wchar_t)
	};

/* the array of stream pointers */
FILE *_Files[FOPEN_MAX] = {&_Stdin, &_Stdout, &_Stderr};
 #endif /* _WIN32_WCE */

#ifdef __QNX__
const
#endif
char _PJP_C_Copyright[] =
	"Copyright (c) 1992-2006 by P.J. Plauger,"
	" licensed by Dinkumware, Ltd."
	" ALL RIGHTS RESERVED.";
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */
