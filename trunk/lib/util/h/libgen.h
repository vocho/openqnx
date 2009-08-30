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



#ifndef LIBGEN_H
#define LIBGEN_H 1

#include <libc.h>

extern char    *bgets(char *, size_t, FILE *, char *);
extern size_t   bufsplit(char *, size_t, char *);
extern char    *copylist(const char *, off_t *);
extern char    *dirname(char *);
extern int      eaccess(const char *, int);
extern int      gmatch(const char *, const char *);
extern int      isencrypt(const char *, size_t);
extern int      mkdirp(const char *, mode_t);
extern int      rmdirp(char *, char *);
extern int      p2open(const char *, FILE *[2]);
extern int      p2close(FILE *[2]);
extern char    *pathfind(const char *, const char *, const char *);
extern char    *regcmp(const char *, ...);
extern char    *regex(const char *, const char *, ...);

extern char    *strcadd(char *, const char *);
extern char    *strccpy(char *, const char *);
extern char    *streadd(char *, const char *, const char *);
extern char    *strecpy(char *, const char *, const char *);
extern int      strfind(const char *, const char *);
extern char    *strrspn(const char *, const char *);
extern char    *strtrns(const char *, const char *, const char *, char *);

#endif
