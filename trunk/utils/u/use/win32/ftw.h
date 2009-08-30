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






#ifndef _FTW_H_INCLUDED
#define _FTW_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __STAT_H_INCLUDED
 #include <sys/stat.h>
#endif

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include _NTO_HDR_(_pack64.h)


struct FTW {
	int					quit;
	int					base;
	int					level;
};

enum {
	FTW_F,				/* file type */
	FTW_D,				/* directory type */
	FTW_DNR,			/* directory type, no read perm */
	FTW_DP,				/* directory type, subdirs visited */
	FTW_NS,				/* cannot stat */
	FTW_SL,				/* symbolic link */
	FTW_SLN				/* symbolic link naming non-existant file */
};

enum {
	FTW_PHYS = 0x0001,	/* physical walk (don't follow symlinks) */
	FTW_MOUNT = 0x0002,	/* do not cross mount point */
	FTW_DEPTH = 0x0004,	/* visit subdirs before dir itself */
	FTW_CHDIR = 0x0008	/* walk will chdir before reading dir */
};

enum {
	FTW_SKD = 0x0001,
	FTW_FOLLOW = 0x0002,
	FTW_PRUNE = 0x0004
};
#include _NTO_HDR_(_packpop.h)


__BEGIN_DECLS

extern int ftw(const char *__path, int (*__fn)(const char *__fname, const struct stat *__sbuf, int __flags), int __depth);
extern int nftw(const char *__path, int (*__fn)(const char *__fname, const struct stat *__sbuf, struct FTW *__ftw), int __depth, int);

__END_DECLS

#endif

/* __SRCVERSION("win32/ftw.h $Rev: 153052 $"); */


