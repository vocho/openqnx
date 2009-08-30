/*
 * redirect.h --- definitions for functions that are OS specific.
 */

/* 
 * Copyright (C) 1986, 88, 89, 91-93, 1996, 1997 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

/* This file is included by custom.h for VMS-POSIX, or first
   by config.h (vms-conf.h) then again by awk.h for normal VMS.  */

#if defined(VMS_POSIX) || defined(IN_CONFIG_H)

#define DEFAULT_FILETYPE ".awk"

/* some macros to redirect some non-VMS-specific code */
#define getopt		gnu_getopt
#define opterr		gnu_opterr
#define optarg		gnu_optarg
#define optind		gnu_optind
#define optopt		gnu_optopt
#define regcomp		gnu_regcomp
#define regexec		gnu_regexec
#define regfree		gnu_regfree
#define regerror	gnu_regerror
#ifndef VMS_POSIX
#define strftime	gnu_strftime	/* always use missing/strftime.c */
#define strcasecmp	gnu_strcasecmp
#define strncasecmp	gnu_strncasecmp
#ifndef VMS_V7
#define tzset		fake_tzset
#define tzname		fake_tzname
#define daylight	fake_daylight
#define timezone	fake_timezone
#define altzone		fake_altzone
#endif
#endif

#ifdef STDC_HEADERS
/* This is for getopt.c and alloca.c (compiled with HAVE_CONFIG_H defined),
   to prevent diagnostics about various implicitly declared functions.  */
#include <stdlib.h>
#include <string.h>
#endif

#else	/* awk.h, not POSIX */

/* some macros to redirect to code in vms/vms_misc.c */
#ifndef bcopy
#define bcopy		vms_bcopy
#endif
#define exit		vms_exit
#define open		vms_open
#define popen		vms_popen
#define pclose		vms_pclose
#define strerror	vms_strerror
#define strdup		vms_strdup
#define unlink		vms_unlink
extern void  exit P((int));
extern int   open P((const char *,int,...));
extern char *strerror P((int));
extern char *strdup P((const char *str));
extern int   vms_devopen P((const char *,int));
# ifndef NO_TTY_FWRITE
#define fwrite		tty_fwrite
#define fclose		tty_fclose
extern size_t fwrite P((const void *,size_t,size_t,FILE *));
extern int    fclose P((FILE *));
# endif
extern FILE *popen P((const char *,const char *));
extern int   pclose P((FILE *));
extern void vms_arg_fixup P((int *,char ***));
/* some things not in STDC_HEADERS */
extern size_t gnu_strftime P((char *,size_t,const char *,const struct tm *));
extern int unlink P((const char *));
extern int getopt P((int,char **,char *));
extern int isatty P((int));
#ifndef fileno
extern int fileno P((FILE *));
#endif
extern int close P((int));
extern int dup P((int));
extern int dup2 P((int, int));
extern int read P((int, void *, int));
extern int getpgrp P((void));

#endif	/* not VMS_POSIX and not IN_CONFIG_H */

/*vms/redirect.h*/
