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





/* _WGetstr function */
#include <string.h>
#include "xwstdio.h"
_STD_BEGIN

static int ranmatch(const wchar_t *su, wchar_t uc, size_t n)
	{	/* look for c in [] with ranges */
	while (3 <= n)
		if (su[1] == L'-')
			if (su[0] <= uc && uc <= su[2])
				return (1);
			else
				su += 3, n -= 3;
		else if (su[0] == uc)
			return (1);
		else
			++su, --n;
	for (; 0 < n; ++su, --n)
		if (su[0] == uc)
			return (1);
	return (0);
	}

int _WGetstr(_WSft *px, int sfl)
	{	/* convert %[] (sfl < 0), %c (sfl == 0), else %s */
	char buf[MB_LEN_MAX], *s = NULL;
	char range = 0;
	char seen = 0;
	int n, nset = 0;
	int wfl = px->qual == L'l';
	wchar_t comp, *p = NULL;
	const wchar_t *pset, *t = NULL;
	wint_t ch;
	_Mbstinit(mbst);

	if (sfl < 0)
		{	/* parse [] in format */
		comp = (wchar_t)(*++px->s == L'^' ? *px->s++ : L'\0');
		t = wcschr(*px->s == L']'
			? px->s + 1 : px->s, L']');
		if (t == 0)
			return (0);
		nset = t - px->s;
		pset = px->s;
		px->s = t;
		if (3 <= nset && wmemchr(pset + 1, L'-', nset - 2))
			range = 1;
		}
	px->nget = 0 < px->width ? px->width :
		sfl != 0 ? INT_MAX : 1;

	if (px->noconv)
		;
	else if (wfl)
		p = va_arg(px->ap, wchar_t *);
	else
		s = va_arg(px->ap, char *);

	while ((ch = WGETN(px)) != WEOF)
		if (0 < sfl && iswspace(ch) || sfl < 0
			&& (!range && (!comp && !wmemchr(pset, ch, nset)
					|| comp && wmemchr(pset, ch, nset))
				|| range && (!comp && !ranmatch(pset, ch, nset)
					|| comp && ranmatch(pset, ch, nset))))
			break;
		else if (!wfl)
			{	/* build a multibyte sequence */
			if ((n = _Wctomb(buf, ch, &mbst)) < 0)
				{	/* bad conversion, give up */
				seen = 1;
				break;
				}
			else if (px->noconv)
				;
			else if (px->prec < (size_t)n)
				{	/* no room to store character, fail */
				seen = 1;
				break;
				}
			else
				{	/* enough room, store and count chars */
				memcpy(s, buf, n);
				s += n;
				px->stored = 1;
				px->prec -= n;
				}
			seen = 2;
			}
		else
			{	/* deliver a single wchar_t */
			if (px->noconv)
				;
			else if (px->prec == 0)
				{	/* no room to store character, fail */
				seen = 1;
				break;
				}
			else
				{	/* enough room, store and count char */
				*p++ = ch;
				px->stored = 1;
				--px->prec;
				}
			seen = 2;
			}

	if (wfl || seen != 2)
		;	/* no need to store homing sequence */
	else if ((n = _Wctomb(buf, L'\0', &mbst) - 1) < 0
		|| px->prec < (size_t)n)
		return (0);	/* bad homing sequence or not enough room */
	else if (!px->noconv)
		{	/* enough room, store and count chars */
		memcpy(s, buf, n);
		s += n;
		px->prec -= n;
		}

	WUNGETN(px, ch);
	if (sfl != 0 && seen == 2)
		;	/* store terminating nul */
	else if (seen == 2)
		return (1);	/* success, delivered a character */
	else if ((seen & 1) != 0 || ch == WEOF)
		return (EOF);	/* conversion or input failure */
	else
		return (0);	/* matching failure */

	if (px->noconv)
		;
	else if (px->prec == 0)
		return (0);	/* no room to store nul, fail */
	else if (wfl)
		*p = L'\0';
	else
		*s = '\0';
	return (1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwgetstr.c $Rev: 153052 $");
