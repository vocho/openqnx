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





/* strtok function */
#include <string.h>
#include "xtls.h"
_STD_BEGIN

#ifndef __QNX__
typedef char *pchar_t;

_TLS_DATA_DEF(static, pchar_t, ssave, "");
#endif

char *(strtok)(char *_Restrict s1, const char *_Restrict s2)
	{	/* find next token in s1[] delimited by s2[] */
#ifdef __QNX__
	static char *ssave;

	return (strtok_r(s1, s2, &ssave));
#else
	char *sbegin, *send;
	pchar_t *pssave = _TLS_DATA_PTR(ssave);

	sbegin = s1 ? s1 : *pssave;
	sbegin += strspn(sbegin, s2);
	if (*sbegin == '\0')
		{	/* end of scan */
		*pssave = "";	/* for safety */
		return (0);
		}
	send = sbegin + strcspn(sbegin, s2);
	if (*send != '\0')
		*send++ = '\0';
	*pssave = send;
	return (sbegin);
#endif
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("strtok.c $Rev: 153052 $");
