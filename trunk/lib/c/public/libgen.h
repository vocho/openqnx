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
 *  libgen.h: General file functions
 *

 */
#ifndef _LIBGEN_H_INCLUDED
#define _LIBGEN_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

#if defined(__EXT_POSIX2)
extern char     *basename( char * __fname );
extern char     *dirname( char * __fname );
#endif

#if defined(__EXT_QNX)
_C_STD_BEGIN
#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif
_C_STD_END
extern char *pathfind_r(__const char *__path, __const char *__name, __const char *__mode, char *__buff, _CSTD size_t __buff_size);

#define WAITFOR_CHECK_CONTINUE	-1
#define WAITFOR_CHECK_ABORT		-2
extern int waitfor( __const char *__path, int __delay_ms, int __poll_ms );
extern int _waitfor( __const char *__path, int __delay_ms, int __poll_ms, int (*__checkfunc)(__const char *, void *), void *__handle );
#endif

#if defined(__EXT_UNIX_MISC)
extern int  eaccess(__const char *__path, int __mode);
extern char *pathfind(__const char *__path, __const char *__name, __const char *__mode);
#endif

__END_DECLS

#endif

/* __SRCVERSION("libgen.h $Rev: 159968 $"); */
