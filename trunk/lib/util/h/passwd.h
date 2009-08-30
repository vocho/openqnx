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






#ifndef __PASSWD_H_INCLUDED
#define	__PASSWD_H_INCLUDED

#ifndef _STDIO_H_INCLUDED
#include <stdio.h>
#endif

#ifdef __MINGW32__
#include <lib/compat.h>
#else
#ifndef __PWD_H_INCLUDED
#include <pwd.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
struct	spwd {
	char	*sp_namp;
	char	*sp_pwdp;
	long	sp_lstchg;	/* password last changed */
	long	sp_max;		/* days between changes */
	long	sp_min;
};


#define	NSHADOW	"/etc/nshadow"
#define	SHADOW	"/etc/shadow"
#define	OSHADOW	"/etc/oshadow"

#define	NPASSWD	"/etc/npasswd"
#define	PASSWD	"/etc/passwd"
#define	OPASSWD	"/etc/opasswd"

#define	PW_LOCK	"/etc/.pwlock"
#define	LOCK_TIMEOUT	60

/* Return codes from check_files */
#define	NO_PASSWORD		001
#define	NO_SHADOW		002
#define	DIFFERENT_DEVS	004
#define	ON_RAMDISK		010
#define	NO_DRIVE		020

#define MAX_PWLEN          32
char  *crypt(const char *pw, const char *salt);
int   lock_passwd(),
      unlock_passwd();
int   base_64(short x);
int   new_salt(char *salt);
int   check_files(void);

int   read_noecho(int fd, void *bufp, int buflen);
int   password(char *prompt,struct passwd *pw);
char *getpass(const char *prompt);
/* int   read_password(const char *prompt, char *pwbuf,int fd); */
int   new_passwd(struct passwd *pw, char *npassword);

void  setspent();
void  endspent();
int   putspent();
struct spwd   *getspent();
struct spwd   *getspnam(char *name);
struct spwd   *sgetspent(char *string);
struct spwd   *fgetspent(FILE *f);

#ifdef __cplusplus
};
#endif
#endif
