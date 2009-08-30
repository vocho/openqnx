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

/* _WFwprep function */
#include <stdlib.h>
#ifdef __QNX__
#include <errno.h>
#endif
#include "xwstdio.h"
_STD_BEGIN

int _WFwprep(FILE *str)
	{	/* prepare wide stream for writing */
	if (str->_Next < str->_WWend)
		return (0);
	else if ((str->_Mode & (_MOPENW | _MREAD | _MBYTE)) != _MOPENW)
		{	/* can't write after read */
		str->_Mode |= str->_Mode & _MBYTE
			? _MERR : _MERR | _MWIDE;
#ifdef __QNX__
		/* PR 12857 con't */
		errno=EBADF;
#endif
		return (-1);
		}
	else if ((str->_Mode & (_MWRITE | _MWIDE)) != (_MWRITE | _MWIDE))
		;	/* haven't been writing */
	else if (str->_Next < str->_Bend)
		;	/* open up rest of existing buffer */
	else if (fflush(str))
		return (-1);	/* failed to flush full buffer */
	if ((str->_Mode & (_MNBF | _MLBF)) != 0
		|| str->_Buf != &str->_Cbuf)
		;
#ifdef __QNX__
	else
		{	/* allocate buffer */
		(void)_Fbuf(str);
		_Closreg();
		}
#else
	else if ((str->_Buf = (unsigned char *)malloc(BUFSIZ)) == 0)
		{	/* use 1-char _Cbuf */
		str->_Buf = &str->_Cbuf;
		str->_Next = str->_Buf;
		str->_Bend = str->_Buf + 1;
		_Closreg();
		}
	else
		{	/* use allocated buffer */
		str->_Mode |= _MALBUF;
		str->_Next = str->_Buf;
		str->_Bend = str->_Buf + BUFSIZ;
		str->_Rend = str->_Buf;
		str->_Wend = str->_Buf;
		_Closreg();
		}
#endif
	str->_WRend = str->_Buf;
	str->_WWend = str->_Bend;
	str->_Mode |= _MWRITE | _MWIDE;
	return (0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwfwprep.c $Rev: 153052 $");
