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
 * dlfcn.h
 *

 */

#ifndef _DLFCN_H_INCLUDED
#define _DLFCN_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifdef __EXT_UNIX_MISC
#include <_pack64.h>
typedef struct dl_info {
	const char				*dli_fname;
	void					*dli_fbase;
	const char				*dli_sname;
	void					*dli_saddr;
}						Dl_info;
#include <_packpop.h>
#endif

__BEGIN_DECLS

#define RTLD_LAZY		0x0001
#define RTLD_NOW		0x0002
#define RTLD_NOLOAD		0x0004
#define RTLD_GLOBAL		0x0100
#define RTLD_LOCAL		0x0200
#define RTLD_GROUP		0x0400
#define RTLD_WORLD		0x0800
#define RTLD_NODELETE	0x1000
#ifdef __EXT_QNX
#define RTLD_NOSHARE	0x2000
#endif

#define RTLD_DEFAULT	((void *) -2)
#define RTLD_NEXT		((void *) -3)

extern char	*dlerror(void);
extern void	*dlopen(const char *__pathname, int __mode);
extern void	*dlsym(void *__handle, const char *__name);
#ifdef __EXT_UNIX_MISC
extern int	dladdr(void *__addr, Dl_info *__info);
#endif
#ifdef __EXT_QNX
extern int	_dladdr(void *__addr, Dl_info *__info);
#endif
extern int	dlclose(void *__handle);

__END_DECLS

#endif

/* __SRCVERSION("dlfcn.h $Rev: 200851 $"); */
