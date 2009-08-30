/* _Readloc function */
#include <string.h>
#include "xlocale.h"
_STD_BEGIN

		/* static data */
static const char kc[] =	/* keyword chars */
	"_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

const _Locitem *_Readloc(FILE *lf, char *buf, const char **ps)
	{	/*	get a line from locale file */
	for (; ; )
		{	/* loop until EOF or full line */
		size_t n;
		const char *s;

		for (buf[0] = ' ', n = 1; ; n -= 2)
			if (fgets(buf + n, MAXLIN - n, lf) == 0
				|| buf[(n += strlen(buf + n)) - 1] != '\n')
				return (0);	/* EOF or line too long */
			else if (n <= 1 || buf[n - 2] != '\\')
				break;	/* continue only if ends in \ */
		buf[n - 1] = '\0';	/* overwrite newline */

		s = _Skip(buf);
		if (*s != '%' && *s != '\0')
			{	/* not comment or empty line, look for keyword */
			const _Locitem *q;

			if (0 < (n = strspn(s, &kc[0])))
				for (q = _Loctab; q->_Name; ++q)
					if (strncmp(q->_Name, s, n) == 0
						&& strlen(q->_Name) == n)
						{	/* found a match */
						*ps = _Skip(s + n - 1);
						return (q);
						}
			return (0);	/* unknown or missing keyword */
			}
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xreadloc.c $Rev: 153052 $");
