/* fscanf function */
#include "xstdio.h"
_STD_BEGIN

static int scin(void *str, int ch, int getfl)
	{	/* get or put a character */
	return (getfl ? fgetc((FILE *)str)
		: ungetc(ch, (FILE *)str));
	}

int (fscanf)(FILE *_Restrict str, const char *_Restrict fmt, ...)
	{	/* read formatted from stream */
	int ans;
	va_list ap;

	va_start(ap, fmt);
#ifdef __QNX__
    // fix a bug in the Dinkum code
	_Lockfileatomic(str);
#else
    _Lockfileatomic(stdin);
#endif    
	ans = _Scanf(&scin, str, fmt, ap, 0);
#ifdef __QNX__
    // fix a bug in the Dinkum code
	_Unlockfileatomic(str);
#else
    _Unlockfileatomic(stdin);
#endif    
	va_end(ap);
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fscanf.c $Rev: 153052 $");
