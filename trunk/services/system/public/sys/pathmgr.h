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
 *  sys/pathmgr.h
 *

 */
#ifndef __PATHMGR_H_INCLUDED
#define __PATHMGR_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
#pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __NEUTRINO_H_INCLUDED
#include _NTO_HDR_(sys/neutrino.h)
#endif

#ifndef __FTYPE_H_INCLUDED
#include _NTO_HDR_(sys/ftype.h)
#endif

#ifndef _UNISTD_H_INCLUDED
#include _NTO_HDR_(unistd.h)
#endif

#ifndef _FCNTL_H_INCLUDED
#include _NTO_HDR_(fcntl.h)
#endif

#if defined(__PID_T)
typedef __PID_T		pid_t;
#undef __PID_T
#endif

#define PATHMGR_FLAG_BEFORE		0x0001	/* Force path to be resolved before others at the same mountpoint. */
#define PATHMGR_FLAG_AFTER		0x0002	/* Force path to be resolved after others at the same mountpoint. */
#define PATHMGR_FLAG_OPAQUE		0x0004	/* Don't resolve to mountpoints with shorter pathname matches. */
#define PATHMGR_FLAG_FTYPEONLY  0x0008  /* Matching ftype is required on the path */
#define PATHMGR_FLAG_FTYPEALL	0x0010	/* Matching all ftypes (for redirecting servers) */
#define PATHMGR_FLAG_STICKY		0x0020	/* No ID is associated with pathname, must procmgr_unlink() to remove. */
#define PATHMGR_FLAG_DIR		0x0100	/* Allow resolving of longer pathnames. */
#define PATHMGR_FLAG_SELF		0x0200	/* Allow resolving names to itself. */

__BEGIN_DECLS

extern int pathmgr_link(const char *__path, _Uint32t __nd, pid_t __pid, int __chid, unsigned __handle, enum _file_type __file_type, unsigned __flags);
extern int pathmgr_symlink(const char *__path, const char *__symlink);
extern int pathmgr_unlink(const char *__path);

__END_DECLS

#endif

/* __SRCVERSION("pathmgr.h $Rev: 153052 $"); */
