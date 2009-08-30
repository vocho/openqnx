/* wcspbrk function */
#include <wchar.h>
_STD_BEGIN

_WConst_return wchar_t *(wcspbrk)(const wchar_t *s1,
	const wchar_t *s2)
	{	/* find index of first s1[i] that matches any s2[] */
	const wchar_t *sc1, *sc2;

	for (sc1 = s1; *sc1 != L'\0'; ++sc1)
		for (sc2 = s2; *sc2 != L'\0'; ++sc2)
			if (*sc1 == *sc2)
				return ((wchar_t *)sc1);
	return (0);	/* terminating nulls match */
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("wcspbrk.c $Rev: 153052 $");
