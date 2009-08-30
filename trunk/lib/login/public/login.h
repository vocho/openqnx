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


 login.h: library of routines for login family.
*/

#ifndef _LOGIN_H_INCLUDED
#define _LOGIN_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef _PWD_H_INCLUDED
#include <pwd.h>
#endif

#ifndef _SHADOW_H_INCLUDED
#include <shadow.h>
#endif

/* read only for all but super user */
#define PASSWD_UMASK	022
/* read/write for super user, non-accessable for others... */
#define SHADOW_UMASK	077

/* Return codes from check_files */
#define	NO_PASSWORD		001
#define	NO_SHADOW		002
#define	DIFFERENT_DEVS	004
#define	ON_RAMDISK		010
#define	NO_DRIVE		020

#define MAX_PWLEN       32
#define MIN_PWLEN       6
#define TOO_SIMPLE      2

#ifndef LOGINDIR
#define LOGINDIR     "/etc/"
#endif
#ifndef ADMDIR
#define ADMDIR       "/usr/adm/"
#endif
#define DEFDIR       LOGINDIR "default/"
#define DEFLOGIN     DEFDIR "login"
#define DEFPASSWD    DEFDIR "passwd"
#define DEFSU        DEFDIR "su"
#define	ENV_SAVE     DEFDIR "login"
#define	DEF_SHELL    "/bin/sh"
#define	LASTLOG_FILE ADMDIR "lastlog"
#define	LOGIN_LOG    ADMDIR "loginlog"
#define	LAST_LOGIN   ADMDIR "lastlogin"
#define	PASSFILE     LOGINDIR "passwd"
#define	STOP_LOGIN   LOGINDIR "nologin"
#define	ISSUE_FILE   LOGINDIR "issue"
#define	QNX_LOG      LOGINDIR "acclog"

#define	PW_LOCK      LOGINDIR ".pwlock"
#define	LOCK_TIMEOUT 60

#define LOGIN_STR	"login: "

#include <_pack64.h>

struct defobj {
	struct syment  *symtab;
	int             nsymtab;
};
typedef struct defobj default_t;

struct lastlog {
	time_t          ll_time;
	char            ll_line[8];
	char            ll_host[16];
};

enum pwdbstat_e {
	PdbOk,
	NoPasswd,
	NoShadow,
	NotSameDevice,
	PasswdBadType,
	ShadowBadType,
	InvalidOwner,
	BusyPasswd
};

enum retcodes {
	Success = 0,
	PermissionDenied,
	SyntaxError,
	BadArg,
	UidBusy,
	DBcorrupt,
	Failed,
	NoPasswdFile,
	PasswdBusy,
	NoSuchUser
};

#include <_packpop.h>

__BEGIN_DECLS

extern char    *chkfiles(void);
extern struct passwd *chklogin(int __pwf_stat, char *__logname);
extern char    *crypt(const char *__key, const char *__salt);
extern char    *def_find(default_t * __dp, char *__key);
extern char    *def_install(default_t * __dp, char *__key, char *__val);
extern default_t *def_open(char *__fname, int __nentries);
extern char    *getdef_str(char *__str);
extern char    *getpass(const char *__prompt);
extern struct passwd *getpwentry(char *__uname);
extern const char  *pwdb_errstr(enum pwdbstat_e);
extern int      addpwent(struct passwd * __pw);
extern int      addshent(struct spwd * __sp);
extern enum pwdbstat_e auth_pwdb(void);
extern int      base_64(short __x);
extern void     bsd_lastlog(struct passwd * __pw, char *__ttynam);
extern int      build_env(char *__fname, int __preserve);
extern int      catfile(char *__fname);
extern int      check_files(void);
extern int      chgpw_name(char *__name, struct passwd * __pw);
extern int      chgsh_name(char *__name, struct spwd * __sp);
extern int      def_close(default_t * __dp);
extern int      delpw_name(char *__name);
extern int      delsh_name(char *__name);
extern int      get_defaults(char *__fname);
extern int      get_string(char *__prompt, char *__bufp);
extern int      getdef_bool(char *__str);
extern long     getdef_int(char *__str);
extern int      getdef_range(char *__str, long *__low, long *__high);
extern pid_t    getspid(int __sid);
extern int      getty(char *__ttyname);
extern int      getyn(char *__prompt,...);
extern int      guess_gid(void);
extern int      guess_uid(void);
extern int      init_defaults(char *__prognam, char *__alt);
extern int      inputval_range(char *__prompt, long __low, long __hi, long __def, long *__res);
extern int      lock_passwd(void);
extern void     log_failure(char *__ttynam, char **__userids, time_t * __times, int __retries);
extern int      new_passwd(struct passwd * __pw, char *__npassword);
extern int      new_salt(char *__salt);
extern int      password(char *__prompt, struct passwd * __pw);
extern int      pseteuid(pid_t __pid, uid_t __euid);
extern int      psetgid(pid_t __pid, gid_t __gid);
extern int      psetuid(pid_t __pid, uid_t __uid);
extern void     qnx_pwlog(struct passwd * __pw, char *__logid, char *__ttynam);
extern int      read_noecho(int __fd, void *__bufp, int __buflen);
extern int      run_shell(struct passwd * __pw);
extern void     setkey(const char *__key);
extern int      sid_name(int __sid, char *__name);
extern int      sulog(int __failed, char *__to, char *__logfile);
extern void     unix_logging(struct passwd * __pw, char *__ttynam);
extern int      unlock_passwd(void);

__END_DECLS

#endif
