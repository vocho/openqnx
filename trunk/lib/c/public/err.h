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
 *  err.h
 *

 */
#ifndef _ERR_H_INCLUDED
#define	_ERR_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS
extern void err __P((int, const char *, ...))
		    __attribute__((__noreturn__, __format__(__printf__, 2, 3)));
extern void verr __P((int, const char *, __NTO_va_list))
		    __attribute__((__noreturn__, __format__(__printf__, 2, 0)));
extern void errx __P((int, const char *, ...))
		    __attribute__((__noreturn__, __format__(__printf__, 2, 3)));
extern void verrx __P((int, const char *, __NTO_va_list))
		    __attribute__((__noreturn__, __format__(__printf__, 2, 0)));
extern void warn __P((const char *, ...))
		    __attribute__((__format__(__printf__, 1, 2)));
extern void vwarn __P((const char *, __NTO_va_list))
		    __attribute__((__format__(__printf__, 1, 0)));
extern void warnx __P((const char *, ...))
		    __attribute__((__format__(__printf__, 1, 2)));
extern void vwarnx __P((const char *, __NTO_va_list))
		    __attribute__((__format__(__printf__, 1, 0)));
__END_DECLS
#endif

/* __SRCVERSION("err.h $Rev: 164949 $"); */
