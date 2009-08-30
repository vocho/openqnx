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

/* _WFrprep function */
#include <stdlib.h>
#ifdef __QNX__
#include <errno.h>
#endif
#include "xwstdio.h"
#include "yfuns.h"
_STD_BEGIN

int _WFrprep(FILE *str)
	{	/* prepare stream for reading */
	if (str->_Next < str->_WRend)
		return (1);
	else if (str->_Mode & _MEOF)
		return (0);
	else if ((str->_Mode & (_MOPENR | _MWRITE | _MBYTE)) != _MOPENR)
		{	/* can't read after write */
		str->_Mode |= str->_Mode & _MBYTE
			? _MERR : _MERR | _MWIDE;
#ifdef __QNX__
		/* PR 12857 - con't */
		errno=EBADF; 
#endif
		return (-1);
		}
	if ((str->_Mode & (_MNBF | _MLBF)) != 0 || str->_Buf != &str->_Cbuf)
		;
#ifdef __QNX__
	else
		(void)_Fbuf(str);
#else
	else if ((str->_Buf = (unsigned char *)malloc(BUFSIZ)) == 0)
		{	/* use 1-char _Cbuf */
		str->_Buf = &str->_Cbuf;
		str->_Bend = str->_Buf + 1;
		}
	else
		{	/* use allocated buffer */
		str->_Mode |= _MALBUF;
		str->_Bend = str->_Buf + BUFSIZ;
		str->_Rend = str->_Buf;
		str->_Wend = str->_Buf;
		}
#endif
	str->_Next = str->_Buf;
	str->_WRend = str->_Buf;
	str->_WWend = str->_Buf;
	 {	/* try to read into buffer */
	int n = _Fread(str, str->_Buf, str->_Bend - str->_Buf);

	if (n < 0)
		{	/* report error and fail */
		str->_Mode |= _MERR | _MWIDE;
		return (-1);
		}
	else if (n == 0)
		{	/* report end of file */
		str->_Mode =
			(unsigned short)((str->_Mode & ~_MREAD) | _MEOF | _MWIDE);
		return (0);
		}
	else
		{	/* set up data read */
		str->_Mode |= _MREAD | _MWIDE;
		str->_WRend += n;
		return (1);
		}
	 }
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwfrprep.c $Rev: 153052 $");
