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

/* vsprintf function */
#include <string.h>
#include "xstdio.h"
_STD_BEGIN

#ifdef __QNX__
#define prout _Sprout
#else
static void *prout(void *s, const char *buf, size_t n)
	{	/* write to string */
	return ((char *)memcpy(s, buf, n) + n);
	}
#endif

int (vsprintf)(char *_Restrict s, const char *_Restrict fmt,
	va_list ap)
	{	/* print formatted to string from arg list */
	int ans = _Printf(&prout, s, fmt, ap);

	if (0 <= ans)
		s[ans] = '\0';
	return (ans);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("vsprintf.c $Rev: 153052 $");
