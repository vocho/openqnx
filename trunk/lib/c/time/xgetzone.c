/* _Getzone function */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "xmtx.h"
#include "xtime.h"
_STD_BEGIN

#ifdef __QNX__
const char *_Getzone(void)
	{	/* get time zone information */
	return (_Tzset());
	}
#else

		/* static data */
static const char *defzone = ":";
static char *tzone = 0;

static _Once_t getzone_o = _ONCE_T_INIT;

static char *reformat(const char *s)
	{	/* reformat TZ from EST-05EDT */
	int i;
	static char tzbuf[] = ":EST:EDT:-0500";

	for (i = 1; i <= 3; ++i)
		if (isalpha((unsigned char)*s))
			tzbuf[i] = *s, tzbuf[i + 4] = *s++;
		else
			return (0);

	tzbuf[9] = (char)(*s == '-' || *s == '+' ? *s++ : '+');
	if (!isdigit((unsigned char)*s))
		return (0);
	tzbuf[10] = *s++;
	if (!isdigit((unsigned char)*s))
		return (0);
	tzbuf[11] = *s++;

	if (isalpha((unsigned char)*s))
		for (i = 5; i <= 7; ++i)
			if (isalpha((unsigned char)*s))
				tzbuf[i] = *s++;
			else
				return (0);
	return (*s == '\0' ? tzbuf : 0);
	}

static void getzone(void)
	{	/* get time zone information */
	if ((tzone = getenv("TIMEZONE")) != 0)
		;
	else if ((tzone = getenv("TZ")) != 0)
		tzone = reformat(tzone);
	if (tzone == 0)
		tzone = (char *)defzone;
	}

const char *_Getzone(void)
	{	/* get time zone information */
	_Once(&getzone_o, getzone);
	return (tzone);
	}
#endif
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xgetzone.c $Rev: 153052 $");
