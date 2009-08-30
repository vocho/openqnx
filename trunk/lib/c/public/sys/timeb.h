/*
 * $QNXLicenseC:
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
 *  sys/timeb.h     timeb structure used with ftime()
 *

 */
#ifndef __TIMEB_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define __TIMEB_H_INCLUDED
#endif

#ifndef __TIMEB_H_DECLARED
#define __TIMEB_H_DECLARED

_C_STD_BEGIN

#if defined(__TIME_T)
typedef __TIME_T	time_t;
#undef __TIME_T
#endif

_C_STD_END

#include <_pack64.h>

struct timeb {
    _CSTD time_t        time;           /* seconds since Jan 1, 1970 UTC */
    _Uint16t            millitm;        /* milliseconds */
    _Int16t             timezone;       /* difference in minutes from UTC */
    _Int16t             dstflag;        /* nonzero if daylight savings time */
	_Uint16t			zero;
};

#include <_packpop.h>

__BEGIN_DECLS

extern int ftime( struct timeb *__timeptr );

__END_DECLS

#endif

#ifdef _STD_USING
using std::time_t;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("timeb.h $Rev: 153052 $"); */
