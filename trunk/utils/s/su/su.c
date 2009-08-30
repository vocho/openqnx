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
 * switch user: become another uid/gid, but keep your name ;-).
 *
 *		  This utility uses the standard routines "getpwnam" to extract
 *        the login information structure, sets a "few" environment vars
 *
 *        HOME = pw->pw_dir, SHELL = pw->pw_shell.
 *
 * 		  It sets the userid,euserid, groupid, egroupid to correspond to 
 *		  the entries in the password file.
 *		  it uses the system call "exec" to transform itself into the 
 *		  shell specified in the password file.
 *
 * $Log$
 * Revision 1.8  2006/04/11 16:16:29  rmansfield
 * PR: 23548
 * CI: kewarken
 *
 * Stage 1 of the warnings/errors cleanup.
 *
 * Revision 1.7  2005/06/03 01:38:01  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.6  2004/05/17 16:26:41  rcraig
 * PR 20236: Login and su have inconsistent user name length checking and passwd has none at all (but doesn't work when the user name exceeds 995 characters).  "chklogin.c" in lib/login/c also uses L_cuserid to check for the username length.  In order to provide consistency, the macro LOGIN_NAME_MAX in limits.h has been defined and set to 256 initially.
 *
 * For building, make sure that libc has been installed with the most up todate version since the old limits.h file won't have the macro defined.
 *
 * Revision 1.5  2003/08/29 21:06:52  martin
 * Add QSSL Copyright.
 *
 * Revision 1.4  2002/03/28 19:27:34  kewarken
 * added warning for non-existence of shadow file
 *
 * Revision 1.3  2002/02/26 20:10:11  kewarken
 * fix for PR:10678 - su fails if shadow file doesn't exist
 *
 * Revision 1.2  2000/09/11 17:12:21  jbaker
 * disabled unix logging pending fix of a bug in libc, pututline
 *
 * Revision 1.1.1.1  2000/05/23 14:21:00  thomasf
 * QNX4 su import
 *
 * Revision 1.8  1996/09/17 14:56:49  steve
 * changes in header files
 *
 * Revision 1.7  1994/11/17  17:02:43  steve
 * Check for argv[1] == '-' changed.  Also check for logname replaced by
 * getuid()  (should actual bye getuid() or geteuid())
 *
 * Revision 1.6  1993/10/27  16:39:56  steve
 * Released version...
 *
 */

#include <pwd.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <process.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#if !defined(__QNXNTO__)
#include <env.h>
#include <sys/dev.h>
#endif

#include "login.h"


static char rcsid[] = "$Id: su.c 171662 2008-06-28 22:16:48Z rmansfield $";


#ifdef UNIX_LOGGING
#undefine UNIX_LOGGING
#endif

#define	ROOT_UID	0
#define	DEF_SHELL	"/bin/sh"
#define PW_STR	  "password: "
#define	DEF_UMASK	0022
#define	DEF_UNAME	"root"

#define	TXT(s)		(s)
#define T_MUST_TTY "su: Must be on a tty!"
#define	T_NO_SHELL "su: No Shell!"
#define T_SU_FAIL "su: Sorry"
#define T_NO_MEMORY "su: no memory to su!"


#define SULOG "/usr/adm/sulog"

int	reload_env = 0;
char	login_name[LOGIN_NAME_MAX+1];
char	*sulogfile;
char	*path;
char	*supath;
char	*suconsole;

char	*
dupstring(char *x)
{
	char	*p = strdup(x);
	if (!x) {
		fprintf(stderr,"su: no memory\n");
		exit(1);
	}
	return p;
}

char *
joinstr(char **argv)
{
	int	i;
	int	len = 0;
	char	*s;
	char	*t;
	for (i=0; argv[i]; i++) {
		len += strlen(argv[i])+1;
	}
	if ((s = malloc(len))) {
		for (t=s,i=0; argv[i]; i++) {
			strcpy(t,argv[i]);
			t += strlen(t);
			*t++ = ' ';
		}
		t[-1] = '\0';
	}
	return s;
}

int
su(struct passwd *pw, char **argv)
{
	char shname[NAME_MAX+2];
	char	*term;

#ifdef UNIX_LOGGING
	unix_logging(pw,ctermid(NULL));
#else
	qnx_pwlog(pw,"SU",ctermid(NULL));
#endif

	if (setgid(pw->pw_gid)) {
		fprintf(stderr, "su: %s (%s)\n", strerror(errno), "setgid");
		return -1;
	}
	
	if (setuid(pw->pw_uid)) {
		fprintf(stderr, "su: %s (%s)\n", strerror(errno), "setuid");
		return -1;
	}

	initgroups(pw->pw_name, pw->pw_gid); 
	if ((term=getenv("TERM"))) {
		term = dupstring(term);
	}
	strtok(pw->pw_shell," ");		/* kill at first space */
	if (reload_env) {
		strcpy(shname,"-");
		strcat(shname, basename(pw->pw_shell));
		clearenv();
		if (chdir(pw->pw_dir) == -1) {
			fprintf(stderr,"su: cannot change directory to %s (%s)\n",
				pw->pw_dir, strerror(errno));
			return -1;
		}
		setenv("LOGNAME",pw->pw_name,1);
	} else {
		strcpy(shname,basename(pw->pw_shell));
	}
	setenv("HOME",pw->pw_dir,1);
	setenv("SHELL",pw->pw_shell,1);
	setenv("TERM", term, 1);
	argv[0] = shname;
#ifdef DEBUGGING
fprintf(stderr,"argv[0]=%s, [1]=%s, [2]=%s\n", argv[0], argv[1], argv[2] ? argv[2] : "<null>");
#endif
	execvp(pw->pw_shell, argv);
	fprintf(stderr,"%s (%s)\n",T_NO_SHELL, strerror(errno));
	return -1;
}

int
main(int argc, char **argv)
{               
	struct	passwd	*pw;
	/* NOT USED char	*p; */
	int				 pwstat;
	enum pwdbstat_e x;	
	strcpy(login_name, "root"); 

	init_defaults(argv[0], "su");

	sulogfile = getdef_str("SULOG");
	path      = getdef_str("PATH");
	supath    = getdef_str("SUPATH");
	suconsole = getdef_str("CONSOLE");
	pwstat     = 0;	/* Ask the user for a password (0) else NO_PASSWORD */

	if (!sulogfile) {
		sulogfile = SULOG;
	}

	if (argc > 1) {
		if (strcmp(argv[1],"-") == 0) {
			reload_env = 1;
			argv++; argc--;
		}
		if (argc > 1 && *argv[1] != '-') {
			/* Truncate username to maximum allowed
				length */
			if (strlen(argv[1]) > LOGIN_NAME_MAX) {
				argv[1][LOGIN_NAME_MAX] = '\0';
			}
			strncpy(login_name,argv[1], sizeof login_name);
			argv++; argc--;		
		} 
	}

	if (getuid() == 0) {
	/*-
	 * no password required
	 */
		if ((pw=getpwnam(login_name))) {
			sulog(0, login_name, sulogfile);
			pw = getpwnam(login_name);
			su(pw,argv);
		}
		return EXIT_FAILURE;
	}
	if ((x=auth_pwdb()) != PdbOk) {
		if(x != NoShadow){
			fprintf(stderr,"su error: %s\n",pwdb_errstr(x));
			return EXIT_FAILURE;
		}
		fprintf(stderr,"Warning: no shadow file\n");
	}
	if ((pw=chklogin(pwstat,login_name)) == NULL) {
		sulog(1, login_name, sulogfile);
		fprintf(stderr,"%s\n",TXT(T_SU_FAIL));
		return EXIT_FAILURE;
	} else {
		sulog(0, login_name, sulogfile);
		pw = getpwnam(login_name);
	}
	su(pw,argv);
	return EXIT_FAILURE;
}
