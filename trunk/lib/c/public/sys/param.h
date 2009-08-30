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
 *  sys/param.h
 *

 */
#ifndef PARAM_H_INCLUDED
#define PARAM_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

#include <time.h>
#include <limits.h>

#ifndef HZ
#define HZ				_sysconf(3)         /* 3 == _SC_CLK_TCK */
#endif

#define MAXHOSTNAMELEN	256		/* Should match _SYSNAME_SIZE in sys/utsname.h */

#ifndef MAXPATHLEN
#ifdef PATH_MAX
#define MAXPATHLEN		PATH_MAX
#else
#define MAXPATHLEN		_POSIX_PATH_MAX
#endif
#endif

#ifndef MAXSYMLINKS
#ifdef SYMLOOP_MAX
#define MAXSYMLINKS		SYMLOOP_MAX
#else
#define MAXSYMLINKS		_POSIX_SYMLOOP_MAX
#endif
#endif

#ifndef NBBY
#define NBBY			CHAR_BIT
#endif

#ifndef NCARGS
#ifdef ARG_MAX
#define	NCARGS			ARG_MAX
#else
#define NCARGS			_POSIX_ARG_MAX 	/* max bytes for an exec function */
#endif
#endif

#define LITTLE_ENDIAN 1234 /* LSB first */
#define BIG_ENDIAN    4321 /* MSB first */

#if defined(__BIGENDIAN__)
#define BYTE_ORDER BIG_ENDIAN
#else
#define BYTE_ORDER LITTLE_ENDIAN
#endif

__END_DECLS

#endif

/* __SRCVERSION("param.h $Rev: 173156 $"); */
