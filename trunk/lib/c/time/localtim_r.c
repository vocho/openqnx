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

/* localtime_r function */
#include "xtime.h"
_STD_BEGIN

struct tm *(localtime_r)(const time_t *tod, struct tm *t)
	{	/* convert to local time structure */
#ifdef __QNX__
	_Tzset();
	return (_Ttotm(t, *tod, -1, __LOCALTIME_CALL));
#else
	return (_Ttotm(t, *tod + _Tzoff(), -1));
#endif
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("localtim_r.c $Rev: 153052 $");
