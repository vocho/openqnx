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





/* fputwc function */
#include <stdlib.h>
#include <string.h>
#include "xwstdio.h"
_STD_BEGIN

wint_t (fputwc)(wchar_t c, FILE *str)
	{	/* put a wchar_t to wide stream */
	int n;

	_Lockfileatomic(str);
	for (n = -1; n != 0; )
		{	/* loop until all chars put */
		char buf[MB_LEN_MAX], *s = buf;
		int m;

		if (str->_Next < str->_WWend)
			;
		else if (_WFwprep(str) < 0)
			{	/* noplace to write */
			_Unlockfileatomic(str);
			return (WEOF);
			}

		m = str->_WWend - str->_Next;
		if (m == 0)
			m = 1;	/* _WFwprep supplies at least one place if successful */

		if (0 < n)
			;	/* chars leftover from last pass */
		else if ((int)MB_CUR_MAX <= m)
			if ((n = _Wctomb((char *)str->_Next, c, &str->_Wstate)) < 0)
				{	/* bad conversion */
				_Unlockfileatomic(str);
				return (WEOF);
				}
			else
				{	/* count delivered characters */
				str->_Next += n;
				break;
				}
		else if ((n = _Wctomb(s = buf, c, &str->_Wstate)) < 0)
			{	/* bad conversion */
			_Unlockfileatomic(str);
			return (WEOF);
			}

		if (n < m)	/* deliver leftover chars */
			m = n;
		memcpy(str->_Next, s, m);
		s += m, n -= m;
		str->_Next += m;
		}

	if (((str->_Mode & _MNBF) != 0
		|| (str->_Mode & _MLBF) != 0 && c == L'\n')
		&& fflush(str))
		{	/* write failed */
		_Unlockfileatomic(str);
		return (WEOF);
		}

 #if !_MULTI_THREAD || !_FILE_OP_LOCKS
	if ((str->_Mode & (_MNBF| _MLBF)) != 0)
		str->_WWend = str->_Next;	/* disable buffering */
 #endif /* !_MULTI_THREAD || !_FILE_OP_LOCKS */

	_Unlockfileatomic(str);
	return (c);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fputwc.c $Rev: 153052 $");
