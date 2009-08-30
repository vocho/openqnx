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

/* _Findloc and _Skip functions */
#include <string.h>
#include "xlocale.h"
_STD_BEGIN

const char *_Skip(const char *s)
	{	/* skip next char plus white-space */
	return (*s == '\0' ? s : s + 1 + strspn(s + 1, " \t"));
	}

_Linfo *_Findloc(const char *name, size_t nlen)
	{	/* look for locale in file */
	const char *s;
	_Linfo *p;
	char buf[MAXLIN], *s1;
	FILE *lf;
	const _Locitem *q;
	char *locfile = getenv("LOCFILE");

	if (locfile == 0)
		return (0);
	if ((lf = fopen(locfile, "r")) == 0)
#ifdef _DEFAULT_LOCFILE		/* __QNX__ */
		{	/* Try a standard locale file */
		static const char *default_locfile = _DEFAULT_LOCFILE;

		if ((lf = fopen(default_locfile, "r")) == 0)
			return (0);
		locfile = (char *)default_locfile;
		}
#else /* _DEFAULT_LOCFILE */
		return (0);
#endif /* _DEFAULT_LOCFILE */

	while ((q = _Readloc(lf, buf, &s)) != 0)
		if (q->_Code == L_NAME
			&& memcmp(s, name, nlen) == 0
			&& *_Skip(s + nlen - 1) == '\0')
			break;
	if (q == 0)
		p = 0;
	else if ((p = (_Linfo *)malloc(sizeof (_Linfo))) == 0)
		;
	else if ((s1 = (char *)malloc(nlen + 1)) == 0)
		free(p), p = 0;
	else
		{	/* build locale */
		*p = _Clocale;
		p->_Name = (char *)memcpy(s1, name, nlen);
		s1[nlen] = '\0';
		if (_Makeloc(lf, buf, p))
			{	/* add new locale to list */
			_Locksyslock(_LOCK_LOCALE);
			p->_Next = _Clocale._Next, _Clocale._Next = p;
			_Unlocksyslock(_LOCK_LOCALE);
			}
		else
			{	/* parsing error reading locale file */
			fputs(buf, stderr);
			fputs("\n-- invalid locale file line\n", stderr);
			_Freeloc(p);
			free(p), p = 0;
			}
		}
	fclose(lf);
	return (p);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xfindloc.c $Rev: 153052 $");
