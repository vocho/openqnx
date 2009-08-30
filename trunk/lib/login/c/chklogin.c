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




#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "login.h"



struct passwd  *
getpwentry(char *name)
{
	struct passwd  *rpw = NULL;
	struct passwd  *pw;

	/*
	 * owned by login, group login.
	 */
	setpwent();
	if ((rpw = getpwnam(name)) == NULL) {
		endpwent();
		return NULL;
	}
	pw = calloc(sizeof(struct passwd), 1);
	pw->pw_name = strdup(rpw->pw_name);
	pw->pw_passwd = strdup(rpw->pw_passwd);
	pw->pw_uid = rpw->pw_uid;
	pw->pw_gid = rpw->pw_gid;
	pw->pw_gecos = strdup(rpw->pw_gecos);
	pw->pw_dir = strdup(rpw->pw_dir);
	pw->pw_shell = strdup(rpw->pw_shell);
	endpwent();
	return pw;
}

int
get_login(char *login_name)
{
	while (1) {
		int             i, c;
		fputs(LOGIN_STR, stdout);
		for (i = 0; (c = getchar()) != EOF && c != '\n';) {
			if (i < LOGIN_NAME_MAX)
				login_name[i++] = c;
		}
		login_name[i] = '\0';
		if (i > 0)
			return i;
	}
}

/*
 * OK, so what if there is no filesystem?, no /etc?, no /etc/passwd ?
 */
struct passwd  *
chklogin(int passwd_stat, char *logname)
{
	int             t;
	struct passwd  *pw;
	static struct passwd pass;
	int             err[3];
	if (access("/etc/passwd", F_OK) == -1) {
		err[0] = errno;
		if (access("/etc", F_OK) == -1) {
			err[1] = errno;
			if (access("/", F_OK) == -1) {
				/* I have no filesystem */
				return 0;
			}
		}
	}
	if (*logname == 0) {
		if (get_login(logname) < 0)
			return 0;
	}
	if ((passwd_stat & NO_PASSWORD) == 0) {
		pw = getpwentry(logname);
		t = password("password:", pw);
		return (t && pw != NULL) ? pw : NULL;
	}
	pw = &pass;
	pw->pw_name = logname;
	pw->pw_passwd = "";
	pw->pw_uid = pw->pw_gid = 0;
	pw->pw_dir = "/";
	pw->pw_comment = "** no password file **";
	pw->pw_shell = DEF_SHELL;
	return pw;
}
