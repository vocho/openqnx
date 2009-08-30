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



/*




Also copyright P.J. Plauger - see bottom of file for details.
*/

/* _Getloc function */
#include <string.h>
#include "xlocale.h"
#include "xmtx.h"
_STD_BEGIN

#if _TERMINATION_CLEANUP	/* __QNX__ */
static _Once_t freeallreg_o = _ONCE_T_INIT;

static void freeall(void)
	{	/* free all constructed locales */
	_Linfo *p, *q;

	for (p = _Clocale._Next; p != 0; p = q)
		{	/* free a locale */
		q = p->_Next;
		_Freeloc(p);
		free(p);
		}
	_Freeloc(&_Clocale);
	_Clocale._Next = 0;
	}

static void freeallreg(void)
	{	/* register freeall with _Atexit */
	_Atexit(&freeall);
	}
#endif

_Linfo *_Getloc(const char *nmcat, const char *lname)
	{	/* get locale pointer, given category and name */
	const char *ns, *s;
	size_t nl = 0;
	_Linfo *p;

	 {	/* find category component of name */
	size_t n;

	for (ns = 0, s = lname; ; s += n + 1)
		{	/* look for exact match or LC_ALL */
		if (s[n = strcspn(s, ":;")] == '\0' || s[n] == ';')
			{	/* memorize first LC_ALL */
			if (ns == 0 && 0 < n)
				ns = s, nl = n;
			if (s[n] == '\0')
				break;
			}
		else if (memcmp(nmcat, s, ++n) == 0)
			{	/* found exact category match */
			ns = s + n, nl = strcspn(ns, ";");
			break;
			}
		else if (s[n += strcspn(s + n, ";")] == '\0')
			break;
		}
	if (ns == 0)
		return (0);	/* invalid name */
	 }

	_Locksyslock(_LOCK_LOCALE);
	for (p = &_Clocale; p != 0; p = p->_Next)
		if (memcmp(p->_Name, ns, nl) == 0
			&& p->_Name[nl] == '\0')
			break;
	_Unlocksyslock(_LOCK_LOCALE);

	if (p == 0)
		{	/* look for locale in file */
		p = _Findloc(ns, nl);
#if _TERMINATION_CLEANUP	/* __QNX__ */
		_Once(&freeallreg_o, freeallreg);
#endif
		}
	return (p);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xgetloc.c $Rev: 153052 $");
