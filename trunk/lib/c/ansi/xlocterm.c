/* _Locterm and _Locvar functions */
#include <ctype.h>
#include <string.h>
#include "xlocale.h"
#include "xtls.h"
_STD_BEGIN

		/* static data */
static const char dollars[] = 	/* PLUS $@ and $$ */
	"^abfnrtv"		/* character codes */
	"01234567"		/* state values */
	"ABCDHLMPSUW"	/* ctype codes */
	"#FIOR";		/* state commands */
static const unsigned short dolvals[] = {
	CHAR_MAX, '\a', '\b', '\f', '\n', '\r', '\t', '\v',
	0x000, 0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700,
	_XA, _XB, _BB, _DI, _XD, _LO, _CN, _PU, _SP, _UP, _XS,
	UCHAR_MAX, _ST_FOLD, _ST_INPUT, _ST_OUTPUT, _ST_ROTATE};
static const char uppers[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

_TLS_ARR_DEF(static, unsigned long, vars, sizeof (uppers) - 1);

int _Locvar(char ch, unsigned long val)
	{	/* set a $ variable */
	const char *s = strchr(uppers, ch);
	unsigned long *pvars = _TLS_ARR(vars);

	if (s == 0)
		return (0);
	pvars[s - uppers] = val;
	return (1);
	}

int _Locterm(const char **ps, unsigned long *ans)
	{	/* evaluate a term on a locale file line */
	const char *s = *ps;
	const char *s1;
	int mi;
	unsigned long *pvars = _TLS_ARR(vars);

	for (mi = 0; *s == '+' || *s == '-'; s = _Skip(s))
		mi = *s == '-' ? !mi : mi;
	if (isdigit((unsigned char)s[0]))
		*ans = strtoul(s, (char **)&s, 0);
	else if (s[0] == '\'' && s[1] != '\0' && s[2] == '\'')
		*ans = ((unsigned char *)s)[1], s += 3;
	else if (s[0] && (s1 = strchr(uppers, s[0])) != 0)
		*ans = pvars[s1 - uppers], ++s;
	else if (s[0] == '$' && s[1]
		&& (s1 = strchr(dollars, s[1])) != 0)
		*ans = dolvals[s1 - dollars], s += 2;
	else
		return (0);
	if (mi)
		*ans = 0 - *ans;
	*ps = _Skip(s - 1);
	return (1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xlocterm.c $Rev: 153052 $");
