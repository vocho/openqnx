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



/*-
 *  shadow.h       Shadow file operations.
 *

 */

#if !defined _SHADOW_H_INCLUDED
#define _SHADOW_H_INCLUDED

#ifndef _PWD_H_INCLUDED
#include <pwd.h>
#endif

#ifndef _STDIO_H_INCLUDED
#include <stdio.h>
#endif


#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

struct    spwd {
        char    *sp_namp;     /* name */
        char    *sp_pwdp;     /* encrypted password */
        long    sp_lstchg;    /* last changed */
        long    sp_max;       /* #days (min) to change */
        long    sp_min;       /* #days (max) to change */
        long    sp_warn;      /* #days to warn */
        long    sp_inact;     /* #days of inactivity */
        long    sp_expire;    /* date to auto-expire */
        long    sp_flag;      /* reserved */
};

#define SPFIELDS 9


#define SHADOW        "/etc/shadow"
#define OSHADOW       "/etc/oshadow"
#define NSHADOW       "/etc/nshadow"

#define PASSWD        "/etc/passwd"
#define OPASSWD        "/etc/opasswd"
#define NPASSWD        "/etc/npasswd"

__BEGIN_DECLS

extern struct spwd   *fgetspent(FILE *f);
extern struct spwd   *getspent(void);
extern struct spwd   *getspent_r(struct spwd *result, char *buffer, int buflen);
extern struct spwd   *getspnam(char *name);
extern struct spwd   *getspnam_r(const char *name, struct spwd *result, char *buffer, int buflen);

extern void  endspent(void);
extern int   putspent(const struct spwd *,FILE *);
extern void  setspent(void);


__END_DECLS

#endif

/* __SRCVERSION("shadow.h $Rev: 153052 $"); */
