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

/* _Defloc function */
#include <string.h>
#ifdef __QNX__		/* Needed for QNX to get access to confstr() */
#include <unistd.h>
#endif
#include "xlocale.h"
#include "xmtx.h"
_STD_BEGIN

#if _ADD_POSIX	/* __QNX__ */
static const char *const envars[_NCAT] = {
	0, "LC_COLLATE", "LC_CTYPE", "LC_MONETARY", "LC_NUMERIC", "LC_TIME",
	"LC_MESSAGES"};

const char *_Defloc(void)
	{	/* find name of default locale */
	char *s;
	int len;
	int i;
	static char *defname;
	static const char *posix = "POSIX";

	if ((s = getenv("LC_ALL")) != 0 && *s)
		return (s);

	for (len = 0, i = 0; ++i < _NCAT; )
		if ((s = getenv(envars[i])) != 0 && *s)
			len += strlen(envars[i]) + strlen(s) + 2;

	if (((s = getenv("LANG")) == 0 || *s == 0) && len == 0)
		{	/* Check system defaults */
		if ((s = getenv("LOCALE")) != 0 && *s)
			s = strdup(s);
#ifdef __QNX__	 /* In QNX look at a coniguration string */
		if (s == 0 || *s == 0)
			if ((len = confstr(_CS_LOCALE, 0, 0)) > 0 && (s = malloc(len + 1)))
				(void)confstr(_CS_LOCALE, s, len + 1);
#endif
		if (s == 0 || *s == 0)
			s = (char *)posix;
		}
	else
		{	/* add LC_* specific entries */
		char *buff;

		if(s == 0 || *s == 0)
			s = (char *)posix;
		if((buff = malloc(strlen(s) + len + 2)) != 0)
			{	/* Make the locale string */
			strcpy(buff, s);
			if(len)
				for (len = 0, i = 0; ++i < _NCAT; )
					if ((s = getenv(envars[i])) != 0 && *s)
						strcat(buff, ";"),
						strcat(buff, _Nmcats[i]),
						strcat(buff, s);
			}
			s = buff;
		}

	if(defname && defname != posix)
		free(defname);
		
	return (defname = s);
	}
#else /* _ADD_POSIX */
const char *_Defloc(void)
	{	/* find name of default locale */
	char *s;

	if ((s = getenv("LOCALE")) == 0)
		s = "C";
	return (s);
	}
#endif /* _ADD_POSIX */
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xdefloc.c $Rev: 153052 $");
