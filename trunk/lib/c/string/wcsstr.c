/* wcsstr function */
#include <wchar.h>
_STD_BEGIN

_WConst_return wchar_t *(wcsstr)(const wchar_t *s1,
	const wchar_t *s2)
	{	/* find first occurrence of wchar_t s2[] in s1[] */
	if (*s2 == L'\0')
		return ((wchar_t *)s1);
	for (; (s1 = wcschr(s1, *s2)) != 0; ++s1)
		{	/* match rest of prefix */
		const wchar_t *sc1, *sc2;

		for (sc1 = s1, sc2 = s2; ; )
			if (*++sc2 == L'\0')
				return ((wchar_t *)s1);
			else if (*++sc1 != *sc2)
				break;
		}
	return (0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcsstr.c $Rev: 153052 $");
