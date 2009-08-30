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





/* _Makeloc function */
#include <string.h>
#include "xlocale.h"
_STD_BEGIN

	/* MACROS */
#define FIXUP(p, field, subfield)	\
	if (p->_Times.field != _Clocale._Times.field \
		&& p->_Times.subfield == _Clocale._Times.subfield) \
		p->_Times.subfield = p->_Times.field

static int gripe(const char *buf)
	{	/* print offending line */
	fputs(buf, stderr);
	fputs("\n -- invalid locale file line\n", stderr);
	return (0);
	}

static char hexval(int ch)
	{	/* return hex value of char or large negative */
	static const char hexvals[] = {"0123456789abcdef"};
	const char *s = strchr(hexvals, ch);

	return ((char)(s == 0 ? -16 : s - hexvals));
	}

static int unhex(char *buf, char *last)
	{	/* expand hexadecimal escape sequences in place */
	char *first, *next;
	int byte;

	for (first = buf, next = buf; first != last; ++next,  ++first)
		if (++first == last
			|| ((byte = hexval(first[-1]) + hexval(first[0])) < 0))
			return (-1);
		else
			*next = (char)byte;
	*next = '\0';
	return ((next - buf) + 1);
	}

static int unescape(wchar_t *bufw, const char *buf, int len)
	{	/* convert multibyte to wchar_t string */
	const unsigned char *s = (unsigned char *)buf;
	int lenw;

	for (lenw = 0; 0 < len; --len, ++s, ++lenw)
		{	/* convert a wchar_t, count it, and maybe store it */
		int lenseq;
		unsigned long ch;

		if (*s < 0x80)
			ch = *s;
		else if (*s < 0xe0)
			return (-1);
		else
			for (ch = *s - 0xe0, lenseq = 1; ; )
				if (--len == 0 || *++s < 0x40 || 5 < ++lenseq)
					return (-1);
				else if (*s < 0x80)
					{	/* fold in last byte and quit */
					ch = ch << 6 | *s - 0x40;
					break;
					}
				else
					ch = ch << 7 | *s - 0x80;
		if ((unsigned long)(wchar_t)ch != ch)
			return (-1);
		if (bufw != 0)
			*bufw++ = (wchar_t)ch;
		}
	if (bufw != 0)
		*bufw = L'\0';
	return (lenw + 1);
	}

const char *_Locsum(const char *s, unsigned long *ans)
	{	/* accumulate terms */
	unsigned long val;

	*ans = 0;
	if (!_Locterm(&s, ans))
		return (0);
	while (_Locterm(&s, &val))
		*ans += val;
	return (s);
	}

int _Makeloc(FILE *lf, char *buf, _Linfo *p)
	{	/* construct locale from text file */
	char *s, *s1;
	const char *cs;
	const _Locitem *q;
	int ok = 1;
	int len;
	unsigned long val;
	static const char gmap[] = "0123456789abcdef^";

	while ((q = _Readloc(lf, buf, (const char **)&s)) != 0)
		switch (q->_Code)
			{	/* process a line */
		case L_GSTRING:	/* alter a grouping string */
		case L_WSTRING:	/* alter a wchar_t string */
		case L_STRING:	/* alter a char string */
			if (NEWADDR(p, q, char *))
				free(ADDR(p, q, char *));
			if (s[0] == '"'
				&& (s1 = strrchr(s + 1, '"')) != 0
				&& *_Skip(s1) == '\0')
				{	/* strip "" from ends of string */
				*s1 = '\0';
				++s;
				len = strlen(s) + 1;
				}
			else if (s[0] != '\''
				|| (s1 = strrchr(s + 1, '\'')) == 0
				|| *_Skip(s1) != '\0')
				len = strlen(s) + 1;
			else if ((len = unhex(++s, s1)) < 0)
				return (0);
			if (q->_Code == L_WSTRING)
				{	/* find length of wchar_t string, then expand it */
				int lenw = unescape(0, s, len);

				if (lenw < 0
					|| (s1 = (char *)malloc(lenw * sizeof (wchar_t))) == 0)
					return (0);
				(void)unescape((wchar_t *)s1, s, len);
				}
			else if ((s1 = (char *)malloc(len)) == 0)
				return (0);
			else
				memcpy(s1, s, len);
			ADDR(p, q, char *) = s1;
			if (q->_Code == L_GSTRING)
				for (; *s1; ++s1)
					if ((cs = strchr(&gmap[0], *s1)) != 0)
						*s1 = (char)(*cs == '^' ? CHAR_MAX : cs - &gmap[0]);
			break;
		case L_TABLE:	/* alter a translation table */
		case L_STATE:	/* alter a state table */
			if (_Makestab(p, q, s) == 0)
				ok = gripe(buf);
			break;
		case L_VALUE:	/* alter a numeric value */
			if ((cs = _Locsum(s, &val)) == 0 || *cs != '\0')
				ok = gripe(buf);
			ADDR(p, q, char) = (char)val;
			break;
		case L_SETVAL:	/* assign to uppercase variable */
			if (*(s1 = (char *)_Skip(s)) == '\0'
				|| (s1 = (char *)_Locsum(s1, &val)) == 0
				|| *s1 != '\0' || _Locvar(*s, val) == 0)
				ok = gripe(buf);
			break;
		case L_WCTYPE:	/* alter a wctype definition */
			if (_Makewct(p, q, s) == 0)
				ok = gripe(buf);
			break;
#ifdef __QNX__
		case L_NOTE:
			break;
#endif
		case L_NAME:	/* end happily with next LOCALE */
			FIXUP(p, _Days, _Abday);
			FIXUP(p, _Days, _Day);
			FIXUP(p, _Months, _Abmon);
			FIXUP(p, _Months, _Mon);
			FIXUP(p, _Formats, _D_t_fmt);
			FIXUP(p, _Formats, _D_fmt);
			FIXUP(p, _Formats, _T_fmt);
			FIXUP(p, _Formats, _T_fmt_ampm);
			FIXUP(p, _Era_Formats, _Era_D_t_fmt);
			FIXUP(p, _Era_Formats, _Era_D_fmt);
			FIXUP(p, _Era_Formats, _Era_T_fmt);
			FIXUP(p, _Era_Formats, _Era_T_fmt_ampm);
			return (ok);
			}
	return (0);	/* fail on EOF or unknown keyword */
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xmakeloc.c $Rev: 153052 $");
