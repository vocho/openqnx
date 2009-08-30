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


/* _Feraise function */
/***********************************************************
 THIS FUNCTION IS ONLY FOR COMPATIBILITY WITH OLDER PROGRAMS
 IT SHOULD BE REMOVED WHEN THE LIBC VERSION MOVES PAST 2.

 THERE IS A C99 VERSION THAT EXISTS IN LIBM INSTEAD OF HERE
 ***********************************************************/
#if (_LIBC_SO_VERSION == 2 || _LIBC_SO_VERSION == 3)

#include <errno.h>
#include <ymath.h>
_STD_BEGIN

void (_Feraise)(int except)
        {        /* report floating-point exception */
        if ((except & (_FE_DIVBYZERO | _FE_INVALID)) != 0)
                errno = EDOM;
        else if ((except & (_FE_UNDERFLOW | _FE_OVERFLOW)) != 0)
                errno = ERANGE;
        }
_STD_END

#endif

/*
 * Copyright (c) 2000 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
V3.05:1296 */


__SRCVERSION("xferaise_compat.c $Rev: 171092 $");
