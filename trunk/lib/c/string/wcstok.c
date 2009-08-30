/* wcstok function */
#include <wchar.h>
_STD_BEGIN

wchar_t *(wcstok)(wchar_t *_Restrict s1, const wchar_t *_Restrict s2,
	wchar_t **_Restrict ps)
	{	/* find next token in wchar_t s1[] delimited by s2[] */
	wchar_t *sbegin, *send;
	static const wchar_t nullstr[1] = {L'\0'};

	sbegin = s1 ? s1 : *ps;
	sbegin += wcsspn(sbegin, s2);
	if (*sbegin == L'\0')
		{	/* end of scan */
		*ps = (wchar_t *)nullstr;	/* for safety */
		return (0);
		}
	send = sbegin + wcscspn(sbegin, s2);
	if (*send != L'\0')
		*send++ = L'\0';
	*ps = send;
	return (sbegin);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcstok.c $Rev: 153052 $");
