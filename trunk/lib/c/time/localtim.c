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

/* localtime function */
#include <stdlib.h>
#include "xmtloc.h"
#include "xtime.h"
_STD_BEGIN

#define MAXTZ	(100 * 13)	/* biggest valid HHMM offset from UTC */

typedef const char *old_t;

_TLS_DATA_DEF(static, old_t, oldzone, 0);
_TLS_DATA_DEF(static, long, tzoff, 0);

#ifndef __QNX__
time_t _Tzoff(void)
#else
long _Tzoff(void)
#endif
	{	/* determine local time offset */
	old_t *poldzone = _TLS_DATA_PTR(oldzone);
	_Tinfo *ptimes = _TLS_DATA_PTR(_Times);
	long *ptzoff = _TLS_DATA_PTR(tzoff);

#ifndef __QNX__
	if (*poldzone != ptimes->_Tzone)
#endif
		{	/* determine time zone offset (East is +) */
		const char *p, *pe;
		int n;

		if (ptimes->_Tzone[0] == '\0')
			ptimes->_Tzone = _Getzone();
		p = _Gettime(ptimes->_Tzone, 2, &n);
		*ptzoff = strtol(p, (char **)&pe, 10);
#ifdef __QNX__
    if (pe - p != n)
			*ptzoff = 0;
#else
		if (pe - p != n
			|| *ptzoff <= -MAXTZ || MAXTZ <= *ptzoff)
			*ptzoff = 0;
		*ptzoff -= (*ptzoff / 100) * 40;	/* HHMM -- changed for C9X */
#endif
		*ptzoff = -*ptzoff;	/* also change sense of offset for C9X */
		*poldzone = ptimes->_Tzone;
		}
#ifdef __QNX__
	return (-*ptzoff);
#else
	return (-*ptzoff * 60);
#endif
	}

#ifdef __QNX__
long _Tzoff_dst(void)
{ /* determine dst time offset */
  static const char *oldzonep;
  static long tzoffl;

  /* determine time zone offset (East is +) */
  const char *p, *pe;
  int n;

  if (_Times._Tzone[0] == '\0')
    _Times._Tzone = _Getzone();
  p = _Gettime(_Times._Tzone, 3, &n);
  tzoffl = strtol(p, (char **)&pe, 10);
  if (pe - p != n)
    tzoffl = 0;
	tzoffl = -tzoffl;	/* also change sense of offset for C9X */
  oldzonep = _Times._Tzone;
  return (-tzoffl);
}
#endif

struct tm *(localtime)(const time_t *tod)
	{	/* convert to local time structure */
#ifdef __QNX__
	_Tzset();
	return (_Ttotm(0, *tod, -1, __LOCALTIME_CALL));
#else
	return (_Ttotm(0, *tod + _Tzoff(), -1));
#endif
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("localtim.c $Rev: 153052 $");
