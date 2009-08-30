/* _Gentime function */
#include <string.h>
#include <stdlib.h>
#include "xtime.h"
_STD_BEGIN

		/* macros */
#define SUNDAY		0	/* codes for tm_wday */
#define MONDAY		1
#define TUESDAY		2
#define WEDNESDAY	3
#define THURSDAY	4

static const char *getval(char *s, int val, int n,
	int *pn, char qual, const _Tinfo *tin)
	{	/* convert a decimal value */
	if (val < 0)
		val = 0;
	if (qual == 'O')
		{	/* try to convert to alternate digit form */
		const char *p = _Gettime(tin->_Alt_digits, val, pn);

		if (0 < *pn)
			return (p);
		}
	*pn = n;
	for (s += n, *s = '\0'; 0 <= --n; val /= 10)
		*--s = (char)(val % 10 + '0');
	return (s);
	}

static int isleapyr(int year)
	{	/* test for leap year */
	return (year % 4 == 0 && (year % 100 != 0
		|| 0 < year && year / 400 == 100
		|| year < 0 && -year / 400 == 300));
	}

static int wkyr(int wstart, int wday, int yday)
	{	/* find week of year */
	wday = (wday + 7 - wstart) % 7;
	return ((yday + 7 - wday) / 7);
	}

static int ISOwkyr(int year, int wday, int yday)
	{	/* find week/year, ISO 8601 -- added with C9X */
	int wkno = wkyr(MONDAY, wday, yday);
	int isleap = isleapyr(year);
	int yunleap = yday - isleap;
	int jan1 = ((371 - yday) + wday) % 7;
	int dec32 = (jan1 + isleap + 365) % 7;

	if (364 <= yunleap && dec32 == TUESDAY
		|| 363 <= yunleap && dec32 == WEDNESDAY
		|| 362 <= yunleap && dec32 == THURSDAY)
		wkno = -1;	/* push into next year */
	else if (jan1 == TUESDAY || jan1 == WEDNESDAY || jan1 == THURSDAY)
		++wkno;
	return (wkno);
	}

static int ISOweek(int year, int wday, int yday)
	{	/* find week of year, ISO 8601 -- added with C9X */
	int wkno = ISOwkyr(year, wday, yday);

	if (wkno == 0)
		return (ISOwkyr(year - 1, wday + 7 - yday,
			isleapyr(year - 1) ? 366 : 365));
	else if (0 < wkno)
		return (wkno);
	else
		return (1);
	}

static int ISOyear(int year, int wday, int yday)
	{	/* find year, ISO 8601 -- added with C9X */
	int wkno = ISOwkyr(year, wday, yday);

	if (wkno == 0)
		return (year - 1);
	else if (0 < wkno)
		return (year);
	else
		return (year + 1);
	}

static int cmp_era_date(const struct tm *t, const char *s)
	{	/* compare date in *tin against yyyy/mm/dd at s */
	char *eptr;
	long val;

	val = strtol(s, &eptr, 10);
	if (s == eptr || *eptr != '/')
		return (2);	/* fail */
	else if (val != t->tm_year + 1900)
		return (t->tm_year + 1900 < val ? -1 : +1);

	val = strtol(s = eptr + 1, &eptr, 10);
	if (s == eptr || *eptr != '/')
		return (2);
	else if (val != t->tm_mon + 1)
		return (t->tm_mon + 1 < val ? -1 : +1);

	val = strtol(eptr + 1, &eptr, 10);
	if (s == eptr)
		return (2);
	else if (val != t->tm_mday)
		return (t->tm_mday < val ? -1 : +1);
	else
		return (0);
	}

static const char *getera(const struct tm *t, const _Tinfo *tin)
	{	/* get era field, if any */
	const char *s;
	int i, len;

	for (i = 0; *(s = _Gettime(tin->_Era, i, &len)) != '\0'; ++i)
		{	/* see if date is in range */
		const char *s1 = _Gettime(s + 1, 1, &len);
		int ans = cmp_era_date(t, s1);

		if (*s == '-' && (ans == -1 || ans == 0)
			|| *s == '+' && (ans == 0 || ans == 1))
			{	/* start date okay, check end date */
			s1 = _Gettime(s + 1, 2, &len);
			if (s1[0] == *s && s1[1] == '*')
				return (s);
			ans = cmp_era_date(t, s1);
			if (*s == '-' && (ans == 0 || ans == 1)
				|| *s == '+' && (ans == -1 || ans == 0))
				return (s);
			}
		}
	return ("+:");
	}

const char *_Gentime(const struct tm *t, const _Tinfo *tin,
	char qual, char code, int *pn, char *ac)
	{	/* format a time field */
	const char *p;

	switch (code)
		{	/* switch on conversion specifier */
	case 'a':	/* put short weekday name */
		p = _Gettime(tin->_Abday, t->tm_wday << 1, pn);
		break;

	case 'A':	/* put full weekday name */
		p = _Gettime(tin->_Day, (t->tm_wday << 1) + 1, pn);
		break;

	case 'b':	/* put short month name */
		p = _Gettime(tin->_Abmon, t->tm_mon << 1, pn);
		break;

	case 'B':	/* put full month name */
		p = _Gettime(tin->_Mon, (t->tm_mon << 1) + 1, pn);
		break;

	case 'c':	/* put date and time */
		p = _Gettime(qual == 'E' ? tin->_Era_D_t_fmt : tin->_D_t_fmt, 0, pn);
		*pn = -*pn;
		break;

	case 'C':	/* put century, from 00  -- added with C9X */
		if (qual != 'E' || *(p = _Gettime(getera(t, tin) + 1, 3, pn)) == '\0')
			p = getval(ac, 19 + t->tm_year / 100, 2, pn, qual, tin);
		break;

	case 'd':	/* put day of month, from 01 */
		p = getval(ac, t->tm_mday, 2, pn, qual, tin);
		break;

	case 'D':	/* put month/day/year -- added with C9X */
		p = "%m/%d/%y";
		*pn = -8;
		break;

	case 'e':	/* put day of month, from 1 -- changed from 'D' for C9X */
		p = getval(ac, t->tm_mday, 2, pn, qual, tin);
		if (ac[0] == '0')
			ac[0] = ' ';
		break;

	case 'F':	/* put year-month-day, ISO 8601 -- added with C9X */
		p = "%Y-%m-%d";
		*pn = -8;
		break;

	case 'g':	/* put year of century, ISO 8601 -- added with C9X */
		 {	/* correct for negative years */
		int year = ISOyear(t->tm_year, t->tm_wday, t->tm_yday) % 100;

		if (year < 0)
			year += 100;
		p = getval(ac, year, 2, pn, qual, tin);
		 }
		break;

	case 'G':	/* put year, ISO 8601 -- added with C9X */
		p = getval(ac, ISOyear(t->tm_year, t->tm_wday, t->tm_yday) + 1900, 4,
			pn, qual, tin);
		break;

	case 'h':	/* put short month name, same as 'b' */
		p = _Gettime(tin->_Abmon, t->tm_mon << 1, pn);
		break;

	case 'H':	/* put hour of 24-hour day */
		p = getval(ac, t->tm_hour, 2, pn, qual, tin);
		break;

	case 'I':	/* put hour of 12-hour day */
		p = getval(ac, (t->tm_hour + 11) % 12 + 1, 2, pn, qual, tin);
		break;

	case 'j':	/* put day of year, from 001 */
		p = getval(ac, t->tm_yday + 1, 3, pn, qual, tin);
		break;

	case 'm':	/* put month of year, from 01 */
		p = getval(ac, t->tm_mon + 1, 2, pn, qual, tin);
		break;

	case 'M':	/* put minutes after the hour */
		p = getval(ac, t->tm_min, 2, pn, qual, tin);
		break;

	case 'n':	/* put newline -- added with C9X */
		p = "\n";
		*pn = 1;
		break;

	case 'p':	/* put AM/PM */
		p = _Gettime(tin->_Am_pm, 12 <= t->tm_hour, pn);
		break;

	case 'r':	/* put 12-hour time -- added with C9X */
		p = _Gettime(qual == 'E' ? tin->_Era_T_fmt_ampm : tin->_T_fmt_ampm,
			3, pn);
		*pn = -*pn;
		break;

	case 'R':	/* put hour:minute -- added with C9X */
		p = "%H:%M";
		*pn = -5;
		break;

	case 'S':	/* put seconds after the minute */
		p = getval(ac, t->tm_sec, 2, pn, qual, tin);
		break;

	case 't':	/* put horizontal tab -- added with C9X */
		p = "\t";
		*pn = 1;
		break;

	case 'T':	/* put hour:minute:second, ISO 8601 -- added with C9X */
		p = "%H:%M:%S";
		*pn = -8;
		break;

	case 'u':	/* put day of week, ISO 8601 -- added with C9X */
		p = getval(ac, t->tm_wday == 0 ? 7 : t->tm_wday, 1, pn, qual, tin);
		break;

	case 'U':	/* put Sunday week of the year */
		p = getval(ac, wkyr(SUNDAY, t->tm_wday, t->tm_yday), 2,
			pn, qual, tin);
		break;

	case 'V':	/* put week number, ISO 8601 -- added with C9X */
		p = getval(ac, ISOweek(t->tm_year, t->tm_wday, t->tm_yday), 2,
			pn, qual, tin);
		break;

	case 'w':	/* put day of week, from Sunday */
		p = getval(ac, t->tm_wday, 1, pn, qual, tin);
		break;

	case 'W':	/* put Monday week of the year */
		p = getval(ac, wkyr(MONDAY, t->tm_wday, t->tm_yday), 2,
			pn, qual, tin);
		break;

	case 'x':	/* put date */
		p = _Gettime(qual == 'E' ? tin->_Era_D_fmt : tin->_D_fmt, 1, pn);
		*pn = -*pn;
		break;

	case 'X':	/* put time */
		p = _Gettime(qual == 'E' ? tin->_Era_T_fmt : tin->_T_fmt, 2, pn);
		*pn = -*pn;
		break;

	case 'y':	/* put year of the century */
		 {	/* change to year in era if valid era present */
		int year = t->tm_year % 100;
		int digits = 2;

		if (year < 0)
			year += 100;
		if (qual == 'E' && (p = getera(t, tin))[2] != '\0')
			{	/* compute year in era */
			char *eptr;
			long val = 1900 + t->tm_year
				- strtol(_Gettime(p + 1, 1, pn), 0, 10);

			if (p[0] == '-')
				val = -val;
			val += strtol(p + 2, &eptr, 10);
			if (p + 2 != eptr && eptr[0] == p[1])
				{	/* accept era year */
				year = (int)val;
				for (digits = 1; 0 < (val /= 10); )
					++digits;
				}
			}
		p = getval(ac, year, digits, pn, qual, tin);
		 }
		break;

	case 'Y':	/* put year */
		if (qual != 'E'
			|| (*(p = _Gettime(getera(t, tin) + 1, 4, pn)) == ';'
				|| *pn == 0))
			p = getval(ac, t->tm_year + 1900, 4, pn, qual, tin);
		else
			{	/* stop format at semicolon */
			const char *p1 = strchr(p, ';');

			if (p1 != 0)
				*pn = p1 - p;
			*pn = -*pn;
			}
		break;

	case 'z':	/* put time zone offset, ISO 8601 -- added with C9X */
		/* pr43768 -- special case if t->tm_isdst<0 -- the spec says
			%z should output no characters.
		*/
		if ( 0 > t->tm_isdst )
			{
			ac[0] = '\0';
			p = ac;
			*pn = 0;
			break;
			}

		p = _Gettime(tin->_Tzone[0] == '\0' ? _Getzone() : tin->_Tzone,
			2, pn);
#ifdef __QNX__
		{
    		int val;
    		val = strtol(p, 0, 10);
    		if (0 < *pn && 0 < t->tm_isdst)
    		{   /* adjust time zone offset for DST */
        		val += (_Tzoff_dst()-_Tzoff());
    		}
    		val = (val/3600)*100+(val % 3600)/60;
    		if (0 <= val)
        		ac[0] = '+';
    		else
        		ac[0] = '-', val = -val;
    		p = getval(ac + 1, val, 4, pn, qual, tin) - 1;
    		++*pn;
		}
#else
		if (0 < *pn && 0 < t->tm_isdst)
			{	/* adjust time zone offset for DST */
			int val = strtol(p, 0, 10) + 100;

			if (0 <= val)
				ac[0] = '+';
			else
				ac[0] = '-', val = -val;
			p = getval(ac + 1, val, 4, pn, qual, tin) - 1;
			++*pn;
			}
#endif
		break;

	case 'Z':	/* put time zone name */
		p = _Gettime(tin->_Tzone[0] == '\0' ? _Getzone() : tin->_Tzone,
			0 < t->tm_isdst, pn);
		break;

	case '%':	/* put "%" */
		p = "%";
		*pn = 1;
		break;

	default:	/* unknown field, print it */
		ac[0] = '%';
		ac[1] = code;
		ac[2] = '\0';
		p = ac;
		*pn = 2;
		}
	return (p);
		}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xgentime.c $Rev: 153052 $");
