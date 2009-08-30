/* _Stopfx function */
#include <ctype.h>
#include "xmath.h"
_STD_BEGIN

int _Stopfx(const char **ps, char **endptr)
	{	/* parse prefix of floating-point field */
	const char *s = *ps;
	int code = 0;

	for (; isspace((unsigned char)*s); ++s)
		;
	if (*s == '-')
		code = FL_NEG, ++s;
	else if (*s == '+')
		++s;
	if (*s == 'n' || *s == 'N')
		{	/* parse "nan" or fail */
		if ((*++s != 'a' && *s != 'A') || (*++s != 'n' && *s != 'N'))
			{	/* parse failed, roll back pointer */
			s = *ps;
			code = FL_ERR;
			}
		else
			{	/* parse optional (n-char-sequence) */
			const char *q = ++s;

			code = FL_NAN;
			if (*q == '(')
				{	/* got '(', skip through ')' */
				for (; isalnum((unsigned char)*++q) || *q == '_'; )
					;
				if (*q == ')')
					s = ++q;
				}
			}
		if (endptr != 0)
			*endptr = (char *)s;
		}
	else if (*s == 'i' || *s == 'I')
		{	/* parse "inf" or fail */
		if ((*++s != 'n' && *s != 'N') || (*++s != 'f' && *s != 'F'))
			{	/* parse failed, roll back pointer */
			s = *ps;
			code = FL_ERR;
			}
		else
			{	/* parse optional rest of  "infinity" */
			const char *q = ++s;
			code |= FL_INF;

			if ((*q == 'i' || *q == 'I')
				&& (*++q == 'n' || *q == 'N')
				&& (*++q == 'i' || *q == 'I')
				&& (*++q == 't' || *q == 'T')
				&& (*++q == 'y' || *q == 'Y'))
				s = ++q;
			}
		if (endptr != 0)
			*endptr = (char *)s;
		}

 #if _HAS_C9X
	else if (*s == '0' && (s[1] == 'x' || s[1] == 'X'))
		{	/* test for valid hex field following 0x or 0X */
		const char *s1 = s + 2;

		if (*s1 == '.')
			++s1;
		if (!isxdigit((unsigned char)*s1))
			code |= FL_DEC;
		else
			s += 2, code |= FL_HEX;
		}
 #endif /* _IS_C9X */

	else
		code |= FL_DEC;
	*ps = s;
	return (code);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xstopfx.c $Rev: 153052 $");
