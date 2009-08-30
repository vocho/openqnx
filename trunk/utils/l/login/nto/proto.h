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
 Local prototypes for stuff that is required to make
 the heavily modified and changed freebsd structured login work right
*/
#include <unix.h>
#include <strings.h>
#include <inttypes.h>
#include <sys/param.h>
#include <paths.h>
#include <pwd.h>

#define LOGIN_DEFCLASS		"default"
#define LOGIN_DEFROOTCLASS	"root"
#define LOGIN_MECLASS		"me"
#define LOGIN_DEFSTYLE		"passwd"
#define LOGIN_DEFSERVICE	"login"
#define	LOGIN_DEFUMASK		022
#define LOGIN_DEFPRI		0
#define	LOGIN_ENV_SAVE		"/etc/default/login"

#define LOGIN_SETGROUP		0x0001		/* set group */
#define LOGIN_SETLOGIN		0x0002		/* set login (via setlogin) */
#define LOGIN_SETPATH		0x0004		/* set path */
#define LOGIN_SETPRIORITY	0x0008		/* set priority */
#define LOGIN_SETRESOURCES	0x0010		/* set resources (cputime, etc.) */
#define LOGIN_SETUMASK		0x0020		/* set umask, obviously */
#define LOGIN_SETUSER		0x0040		/* set user (via setuid) */
#define LOGIN_SETENV		0x0080		/* set user environment */
#define	LOGIN_SETALL		0x00ff		/* set everything */

#define AUTH_OKAY			0x01		/* user authenticated */
#define AUTH_ROOTOKAY		0x02		/* root login okay */
#define AUTH_SECURE			0x04		/* secure login */
#define AUTH_SILENT			0x08		/* silent rejection */
#define AUTH_CHALLENGE		0x10		/* a chellenge was given */

#define	AUTH_ALLOW		(AUTH_OKAY | AUTH_ROOTOKAY | AUTH_SECURE)

struct lastlog {
	time_t          ll_time;
	char            ll_line[8];
	char            ll_host[16];
};

/* Missing definitions */
extern char        *crypt(const char *, const char*);
extern int			getdtablesize(void);

/* Functions we supplement */
int setusercontext(const struct passwd *pwd, uid_t uid, unsigned int flags);
char *save_list(char *fname, int *buflen, int preserve);
void add_env(char *str);

