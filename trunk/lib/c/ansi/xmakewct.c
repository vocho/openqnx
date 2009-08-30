/* _Makewct function */
#include <stddef.h>
#include <string.h>
#include "xlocale.h"
_STD_BEGIN

	/* macros */
#define MAXVALS	10	/* maximum number of comma-separated values */
#define MINADD	48	/* minimum wchar_ts to add to table of ranges */

int _Makewct(_Linfo *p, const _Locitem *q, const char *s)
	{	/* parse wctrans/wctype locale entry */
	char *s1;
	int nameonly;
	int wasnew = NEWADDR(p, q, _Wctab *);
	_Wctab *po = ADDR(p, q, _Wctab *);
	_Wctab *ps, *pt;

	if (*(s1 = (char *)s + strcspn(s, " \t")) == '\0')
		nameonly = 1;
	else
		{	/* nul-terminate name for comparisons */
		nameonly = 0;
		*s1 = '\0';
		}
	for (ps = 0, pt = &po[1]; pt->_Name != 0; ++pt)
		if (strcmp(s, pt->_Name) == 0)
			ps = pt;	/* found name in list, but run pt to end */

	if (ps == 0 || !nameonly && !wasnew)
		{	/* make new property table */
		_Wctab *pn = (_Wctab *)malloc(sizeof (_Wctab)
			* ((pt - po) + (ps == 0 ? 2 : 1)));	/* copy with room for new */

		if (pn == 0)
			return (0);
		memcpy(pn, po, sizeof (_Wctab) * ((pt - po) + 1));
		if (ps != 0)
			ps = pn + (ps - po), pt = pn + (pt - po);
		else
			{	/* add new property */
			ps = pn + (pt - po);
			if ((ps->_Name =
				(const char *)malloc(strlen(s) + 1)) == 0)
				{	/* not enough storage, tidy and quit */
				free(pn);
				return (0);
				}
			strcpy((char *)ps->_Name, s);
			pt = ps + 1;
			pt->_Name = 0;
			pt->_Off = ps->_Off;	/* new entry has empty sequence at end */
			}
		if (wasnew)
			free(po);
		ADDR(p, q, _Wctab *) = po = pn;
		}

	if (!nameonly)
		{	/* add values to ranges */
		size_t n;
		wchar_t vals[MAXVALS];
		wchar_t *px = (wchar_t *)po->_Name;

		*s1 = ' ';	/* restore whitespace at end of name */
		s = s1;
		for (n = 0; n < MAXVALS; ++n)
			{	/* gather comma-separated values */
			unsigned long val;

			if ((s = _Locsum(_Skip(s), &val)) == 0)
				return (0);
			vals[n] = (wchar_t)val;
			if (*s != ',')
				break;
			}
		if (*s != '\0')
			return (0);

		++ps;	/* point to place to add new triplet */
		if (po->_Off < pt->_Off + n)
			{	/* reallocate to make more space */
			size_t nw = pt->_Off + MINADD;

			if (po->_Off == 0)
				{	/* first growth, try to allocate space and copy */
				wchar_t *py = (wchar_t *)malloc(nw * sizeof (wchar_t));

				if (py == 0)
					return (0);
				memcpy(py, px, pt->_Off * sizeof (wchar_t));
				px = py;
				}
			else if ((px = (wchar_t *)realloc(px,
				nw * sizeof (wchar_t))) == 0)
				return (0);

			po->_Name = (char *)px;
			po->_Off = nw;
			}
		memmove(px + ps->_Off + n, px + ps->_Off,
			(pt->_Off - ps->_Off) * sizeof (wchar_t));	/* dig a hole */
		memcpy(px + ps->_Off, vals, n * sizeof (wchar_t));	/* and fill it */
		for (; ps <= pt; ++ps)
			ps->_Off += n;	/* adjust offsets */
		}
	return (1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xmakewct.c $Rev: 153052 $");
