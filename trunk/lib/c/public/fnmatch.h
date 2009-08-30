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
 *  fnmatch.h
 *

 */

#ifndef _FNMATCH_H_INCLUDED
#define _FNMATCH_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif


#define FNM_PATHNAME 001
#define FNM_PERIOD   002
#define FNM_NOESCAPE 004
#if defined(__EXT_QNX)
#define FNM_QUOTE    FNM_NOESCAPE
#endif

#define FNM_NOMATCH  1


__BEGIN_DECLS
extern int fnmatch(const char *__expr, const char *__str, int __flags);
__END_DECLS
#endif

/* __SRCVERSION("fnmatch.h $Rev: 153052 $"); */
