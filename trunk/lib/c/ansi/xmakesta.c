/* _Makestab function */
#include <string.h>
#include "xlocale.h"
_STD_BEGIN

int _Makestab(_Linfo *p, const _Locitem *q, const char *s)
	{	/* process tab[#,lo:hi] $x expr */
	int inc = 0;
	unsigned long hi, lo, stno, val;
	unsigned short *usp, **uspp;

	if (*s != '[' || (s = _Locsum(_Skip(s), &stno)) == 0)
		return (0);
	if (*s != ',')
		lo = stno, stno = 0;
	else if (q->_Code != L_STATE || _NSTATE <= stno
		|| (s = _Locsum(_Skip(s), &lo)) == 0)
		return (0);
	lo = (unsigned char)lo;
	if (*s != ':')
		hi = lo;
	else if ((s = _Locsum(_Skip(s), &hi)) == 0)
		return (0);
	else
		hi = (unsigned char)hi;
	if (*s != ']')
		return (0);
	for (s = _Skip(s); s[0] == '$'; s = _Skip(s + 1))
		if (s[1] == '@' && (inc & 1) == 0)
			inc |= 1;
		else if (s[1] == '$' && (inc & 2) == 0)
			inc |= 2;
		else
			break;
	if ((s = _Locsum(s, &val)) == 0 && inc == 0
		|| s != 0 && *s != '\0')
		return (0);
	uspp = &ADDR(p, q, unsigned short *)
		+ (int)(stno & 0xf);
	if (q->_Code == L_TABLE)
		usp = NEWADDR(p, q, short *) ? *uspp : 0;
	else
		usp = *uspp != 0 && (*uspp)[-1] ? *uspp : 0;
	if (usp == 0)
		{	/* setup a new table */
		if ((usp = (unsigned short *)malloc(TABSIZ)) == 0)
			return (0);
		if (ADDR(p, q, short *) != 0)
			memcpy(usp, ADDR(p, q, short *) - 1, TABSIZ);
		else
			memset(usp, 0, TABSIZ);
		if (q->_Code == L_STATE)
			usp[0] = 1;	/* allocation flag */
		*uspp = ++usp;
		}
	for (; lo <= hi; ++lo)
		usp[lo] = (unsigned short)(val + (inc & 1 ? lo : 0)
			+ (inc & 2 ? usp[lo] : 0));
	return (1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xmakesta.c $Rev: 153052 $");
