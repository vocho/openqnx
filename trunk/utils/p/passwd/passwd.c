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

	passwd:	change passwd or create initial passwd entry.
	
	24,july,91: steve
	- don't make invalid uid/gid root
	- more careful in creating home directory


$Log$
Revision 1.10  2006/05/29 16:11:43  rmansfield
PR: 38017
CI: kewarken

Fixed off-by-one error.

Revision 1.9  2006/05/04 18:52:35  rmansfield
PR: 23548
CI: kewarken

Last change reverted warnings cleanup of PR23548.

Revision 1.8  2006/05/04 18:20:26  rmansfield
PR: 38017
CI: kewarken

Use strncpy instead of strcpy.

Revision 1.7  2006/04/11 16:16:03  rmansfield

PR: 23548
CI: kewarken

Stage 1 of the warnings/errors cleanup.

Revision 1.6  2005/06/03 01:37:53  adanko

Replace existing QNX copyright licence headers with macros as specified by
the QNX Coding Standard. This is a change to source files in the head branch
only.

Note: only comments were changed.

PR25328

Revision 1.5  2004/05/17 16:26:41  rcraig
PR 20236: Login and su have inconsistent user name length checking and passwd has none at all (but doesn't work when the user name exceeds 995 characters).  "chklogin.c" in lib/login/c also uses L_cuserid to check for the username length.  In order to provide consistency, the macro LOGIN_NAME_MAX in limits.h has been defined and set to 256 initially.

For building, make sure that libc has been installed with the most up todate version since the old limits.h file won't have the macro defined.

Revision 1.4  2003/08/27 18:04:34  martin
Add QSSL Copyright.

Revision 1.3  1999/12/09 18:43:15  thomasf
Added the ability to use the /etc/default/passwd file
to specify either QNX4 type crypt of UNIX type crypt
rather than prompt the user.  By default it uses
the UNIX crypt (Neutrino only) if the QNXCRYPT specifier
is not set in the file.

Revision 1.2  1999/08/03 18:02:23  thomasf
Fixed the local declaration of qnx_crypt to be const char *

Revision 1.1.1.1  1999/07/14 14:47:34  thomasf
passwd migrated from QNX4

Revision 1.6  1997/03/07 20:11:16  eric
lock_passwd in login lib changed to return appropriate errno
instead of timing out when an error other than EEXIST is returned
when trying to open() the password lock file (/etc/.passwd).
When unable to lock password file, passwd now will print
the reason why (normally EBUSY, but could be something else).

Revision 1.5  1997/03/07 16:06:43  steve
*** empty log message ***

 * Revision 1.4  1993/11/30  21:34:55  steve
 * Changed error message for too simple of a password to match reality.
 *
 * Revision 1.3  1993/11/30  21:30:29  steve
 * *** empty log message ***
 *
*/

#include <libc.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>		
#include <sys/timeb.h>
#include <sys/stat.h>
#include <util/stdutil.h>

#include "login.h"

//extern char *crypt(char *key, char *salt);
//extern char *getpass(char *prompt);
//extern int new_salt(char *salt);
#if defined(__QNXNTO__)
extern int new_salt(char *salt);
#define USE_QNX_CRYPT		0x1
#define USE_UNIX_CRYPT		0x2

#else
#include <crypt.h>
#include <login.h>
#endif

#define NORMAL_PASSWD "x"
#define CHROOTED_PASSWD "*"

char     *passwd_stub = NORMAL_PASSWD;
char     *nil_passwd = "";


#define PWLEN 32

int enter_uid() {
	long            uid;
	long            low, hi;
	struct passwd  *pw;

	if (getdef_range("UIDRANGE", &low, &hi) < 2) {
		low = 0;
		hi = UINT_MAX;
	}

	while (1) {
		uid = guess_uid();
		if (inputval_range("User id #", low, hi, uid, &uid) == -1)
			return -1;
		if ((pw = getpwuid(uid)) != NULL) {
			if (getdef_bool("DUPUIDOK")) {
				fprintf(stderr, "warning, userid %ld is already in use\n", uid);
			} else {
				fprintf(stderr, "error, userid %ld is already in use\n", uid);
				fprintf(stderr, "you are not permitted to share userids\n");
			}
			continue;
		}
		return uid;
	}
}


int enter_gid() {
	long            gid;
	long		low, hi;
	struct group   *gr;

	if (getdef_range("GIDRANGE", &low, &hi) < 2) {
		low = 0;
		hi = UINT_MAX;
	}
	while (1) {
		gid = guess_gid();
		if (inputval_range("Group id #", low, hi, gid, &gid) == -1)
			return -1;
		if ((gr = getgrgid(gid)) == NULL) {
			printf("Warning! group %ld doesn't exist\n", gid);
			printf("please remember to update /etc/group\n");
		}
		return gid;
	}
}

int analyzepw(char *s, int lim) {
	int             i = 0;
	int             punct = 0, chars = 0, nums = 0, space = 0;

	for (i = 0; s[i] && i < lim; i++) {
		if (isalpha(s[i]))
			chars = 1;
		else if (isdigit(s[i]))
			nums = 1;
		else if (isspace(s[i]))
			space = 1;
		else if (ispunct(s[i]))
			punct = 1;
	}
	return punct + chars + nums + space;
}

#define MIN_PWLEN 6
#define TOO_SIMPLE 2
char *too_simple_fmt =
"Your installation requires passwords to be a minimum of %d characters.\n"
"The passwords must be chosen from at least %d of the character classes:\n"
"\tupper and lower case alphabetic\n"
"\tpunctuation\n"
"\tnumeric\n"
"\tspace\n";

int get_passwd(char *bufp, int type) {
	char		pwbuf[PWLEN + 1], pwbuf2[PWLEN + 1];
	char            salt[3];
	int             counter;
	int             insistant = getdef_int("INSISTANT");
	char           *p;
	int got_password = 0;
	counter = 0;
	if (insistant < 1)
		insistant = 1;
	while (counter <= insistant) {
		if (getuid() == 0 || getdef_bool("NOPASSWORDOK")) {
			counter++;
		}
		pwbuf[0] = '\0';
		pwbuf2[0] = '\0';
		if ((p = getpass("New password:"))) {
			strncpy(pwbuf, p, PWLEN);
			pwbuf[PWLEN] = '\0';
		}
		if (strlen(pwbuf) == 0 && getuid() != 0 && !getdef_bool("NOPASSWORDOK")) {
			fprintf(stderr, "You must enter a password, please try again\n");
			continue;
		}
		if (getuid() != 0 && getdef_bool("STRICTPASSWORD")
		    && (strlen(pwbuf) < MIN_PWLEN ||
			analyzepw(pwbuf, MIN_PWLEN) < TOO_SIMPLE)) {
			fprintf(stderr,too_simple_fmt, MIN_PWLEN, TOO_SIMPLE);
			continue;
		}
		pwbuf2[0] = '\0';
		if ((p = getpass("Retype new password:"))) {
			strncpy(pwbuf2, p, PWLEN);
			pwbuf2[PWLEN] = '\0';
		}
		if (strcmp(pwbuf, pwbuf2) != 0) {
			fprintf(stderr, "Mismatch - password not changed\n");
			counter = 0;
			continue;
		}
		got_password = 1;
		break;
	}
	if (!got_password) {
		fprintf(stderr, "No password was provided\n");
		return -1;
	}
	*bufp = 0;
	new_salt(salt);
	if (strlen(pwbuf) > 0) {
#if defined(__QNXNTO__)
		if (!(type & (USE_QNX_CRYPT | USE_UNIX_CRYPT))) {
			if (getdef_bool("QNXCRYPT")) {
				type |= USE_QNX_CRYPT;
			}
			else {
				type |= USE_UNIX_CRYPT;
			}
		}
		if (type & USE_QNX_CRYPT) {
			strncpy(bufp, qnx_crypt(pwbuf, salt), PWLEN);
			bufp[PWLEN] = '\0';
		}
		else if (type & USE_UNIX_CRYPT) {
			strncpy(bufp, crypt(pwbuf, salt), PWLEN);
			bufp[PWLEN] = '\0';
		}
#else
		strncpy(bufp, crypt(pwbuf, salt), PWLEN);
		bufp[PWLEN] = '\0';
#endif
	}

	memset(pwbuf, 0, sizeof pwbuf);
	memset(pwbuf2, 0, sizeof pwbuf2);
	return *bufp ? 1 : 0;
}
int
mkdirp(char *dirp, uid_t uid, gid_t gid)
{
	if (mkdir(dirp, 0755) == -1) {
		if (errno != EEXIST) {
			fprintf(stderr, "Cannot create directory '%s':%s\n",
				dirp, strerror(errno));
			return -1;
		}
		if (getdef_bool("DUPDIROK")) {
			fprintf(stderr, "\n*** WARNING ***\n\n%s already exists. If it is being used by another user as a home\n"
			"directory problems could result.\n\n", dirp);
			fprintf(stderr, "Ownership of %s will be changed to match the new user's.\n\n", dirp);
			if (!getyn("Use directory %s anyway? (y/N) ", dirp))
				return -1;
		} else {
			fprintf(stderr, "Directory %s already exists!\n", dirp);
			fprintf(stderr, "You are not permitted to share directories amongst users\n");
			return -1;
		}
	}
	if (chown(dirp, uid, gid) == -1) {
		fprintf(stderr, "cannot change access to home directory\n");
		rmdir(dirp);
		return -1;
	}
	return 0;
}

int
initdir(char *dirp, uid_t uid, gid_t gid)
{
	char    buff[PATH_MAX + 1];
	char	*p;

	strncpy(buff, dirp, PATH_MAX);
	buff[PATH_MAX] = '\0';
	if (buff[strlen(buff) - 1] != '/')
		strncat(buff, "/", (PATH_MAX - strlen(buff) - 1));
	if ((p=getdef_str("PROFILE")) == 0) {
		return 0;
	}
	strncat(buff, p, (PATH_MAX - strlen(buff) - 1));
	if ((p = getdef_str("DEFPROFILE")) == 0) {
		return 0;
	}
	/* file does not exist */
	if (access(p, R_OK) == 0 && access(buff, F_OK) == -1) {
		FILE *f;
		FILE *g;
		if ((f=fopen(p,"r")) == 0) {
			fprintf(stderr,"warning cannot read shell init file (%s):%s\n",
				p, strerror(errno));
		} else if ((g=fopen(buff,"w"))  == 0) {
			fprintf(stderr,"warning cannot create shell init file (%s):%s\n",
				buff, strerror(errno));
			fclose(f);
		} else {
			int	c;
			while ((c=getc(f)) != EOF) putc(c,g);
			fclose(f);
			fclose(g);
			if (chown(buff, uid, gid) == -1) {
				fprintf(stderr, "warning: cannot change owner of (%s):%s\n",
					buff, strerror(errno));
			} else if (chmod(buff, 0644) == -1) {
				fprintf(stderr, "warning: cannot change file permission of (%s):%s\n",
					buff, strerror(errno));
			}
		}
	}
	return 0;
}



struct passwd *
newacc(char *username)
{
	char	buf[PATH_MAX + 1];
	struct	passwd *pw;
	int             i;

	if ((pw=getpwnam(username)) != 0) {
		fprintf(stderr,"user %s already exists\n", username);
		return 0;
	}
	if ((pw=malloc(sizeof *pw)) == 0) {
		return 0;
	}
	pw->pw_name = username;
	if ((pw->pw_uid = enter_uid()) == -1) {
		return 0;
	}
	if ((pw->pw_gid = enter_gid()) == -1) {
		return 0;
	}
	buf[0] = '\0';
	if (get_string("Real name", buf) == -1) {
		return 0;
	}
	if ((pw->pw_comment=strdup(buf)) == 0) {
		return 0;
	}
	strncpy(buf, getdef_str("BASEDIR"), PATH_MAX);
	buf[PATH_MAX] = '\0';
	if ((i = strlen(buf)) > 0) {
		if (buf[i - 1] != '/') {
			buf[i++] = '/';
		}
		strncpy(buf + i, username, PATH_MAX - i);
	}
	if (get_string("Home directory", buf) == -1) {
		return 0;
	}
	if ((pw->pw_dir = strdup(buf)) == 0) {
		return 0;
	}
	strncpy(buf, getdef_str("SHELL"), PATH_MAX);
	buf[PATH_MAX] = '\0';
	if (get_string("Login shell", buf) == -1)
		return 0;
	if ((pw->pw_shell = strdup(buf)) == 0) {
		return 0;
	}
	pw->pw_passwd = passwd_stub;
	while (mkdirp(pw->pw_dir, pw->pw_uid, pw->pw_gid) == -1) {
		*buf = '\0';
		if (get_string("Enter a different home directory or enter to exit ", buf) == -1)
			return 0;
		if (*buf == '\0')
			return 0;
		free(pw->pw_dir);
		if ((pw->pw_dir=strdup(buf)) == 0)
			return 0;
	}

	return pw;
}

int
create_account(char *username)
{
	struct passwd *pw;
	int	   opt, type;
	char   pwbuf[PWLEN + 1];
	int ret;

	type = 0;
	if ((pw=newacc(username)) == 0) {
		fprintf(stderr, "Account not created\n");
		return EXIT_FAILURE;
	}
	initdir(pw->pw_dir, pw->pw_uid, pw->pw_gid);

	if ((ret = get_passwd(pwbuf, type)) == -1)
		return EXIT_FAILURE;
	pw->pw_passwd = ret ? passwd_stub : nil_passwd;

	if (!lock_passwd()) {
		fprintf(stderr,"unable to lock password database (%s)\n",strerror(errno));
		return PasswdBusy;
	}
	if ((opt = addpwent(pw)) == 0) {
		if (pw->pw_passwd && *pw->pw_passwd) {
			struct spwd	spw;
			spw.sp_namp	= pw->pw_name;
			spw.sp_pwdp	= pwbuf;
			spw.sp_lstchg	= time(0);
			spw.sp_max	= 0;
			spw.sp_min	= 0;
			spw.sp_warn	= 0;
			spw.sp_inact	= 0;
			spw.sp_expire	= 0;
			spw.sp_flag	= 0;
			if ((opt = addshent(&spw))) {
				fputs("cannot create shadow entry\n",stderr);
				delpw_name(pw->pw_name);
			}
		}
	} else {
		fputs("cannot create password entry\n", stderr);
	}
	unlock_passwd();
	return opt;
}

int chk_passwd(struct passwd *pw, struct spwd *sp) {
	char	*p;
	if (!pw->pw_passwd || !*pw->pw_passwd) {
		return (0);
	}
	if ((p=getpass("Current Password:")) == 0) {
		return -1;
	}
	if (!(sp && sp->sp_pwdp)) {
		return -1;
	}
#if defined(__QNXNTO__)
	//We really want to check both shadow and normal passwords
	if (strcmp(sp->sp_pwdp, crypt(p, sp->sp_pwdp)) == 0) {
		//We match and use a unix crypt
		return(USE_UNIX_CRYPT);
	}
	else if (strcmp(sp->sp_pwdp, qnx_crypt(p, sp->sp_pwdp)) == 0) {
		//We match and use a qnx4 crypt
		return(USE_QNX_CRYPT);
	}
#else
	if (strcmp(sp->sp_pwdp,crypt(p,sp->sp_pwdp)) == 0) {
		return(0);
	}
#endif
	return -1;
}


int change_passwd(struct passwd * pw) {
	struct	spwd *sp = NULL;
	struct  spwd	spw;
	char	pwbuf[PWLEN];
	int		opt, ret;

	printf("changing password for %s\n", pw->pw_name);

	if (pw->pw_passwd && *pw->pw_passwd) sp=getspnam(pw->pw_name);

	ret = 0;
	if (getuid() != 0) {
		if ((ret = chk_passwd(pw, sp)) == -1) {
			fputs("Sorry!\n",stderr);
			return EXIT_FAILURE;
		}
	}
	if (get_passwd(pwbuf, ret) == -1)
		return EXIT_FAILURE;
	if (!lock_passwd()) {
		fprintf(stderr,"unable to lock password database (%s)\n",strerror(errno));
		return PasswdBusy;
	}
/*-
 * there are 4 normal cases covered here:
 * 1: user did not previously have a password and now does.
 * 2: user did not previously have a password and still doesn't.
 * 3: user previously had a password and now doesn't.
 * 4: user previously had a password and still does.
 *
 * plus the additional user should have had a password but
 * there was none in /etc/shadow!
 *
 * #1 -- update password & shadow files.
 * #2 -- do nothing.
 * #3 -- update password & shadow files.
 * #4 -- update shadow only.
 * in #1, the shadow file must be updated first for consistency.
 * in #3, the password file must be updated first for consistency.
 */
 	opt = Success;
	if (*pwbuf) {	/* user put a password in */
		if (!sp) {
			spw.sp_namp	= pw->pw_name;
			spw.sp_pwdp	= pwbuf;
			spw.sp_lstchg	= time(0);
			spw.sp_max	= 0;
			spw.sp_min	= 0;
			spw.sp_warn	= 0;
			spw.sp_inact	= 0;
			spw.sp_expire	= 0;
			spw.sp_flag	= 0;
			sp = &spw;
		} else {
			sp->sp_pwdp = pwbuf;
			sp->sp_lstchg = time(0);
		}
		if (!(pw->pw_passwd && *pw->pw_passwd)) {
			if ((opt=addshent(&spw)) != Success) {
				unlock_passwd();
				return opt;
			}
			pw->pw_passwd = passwd_stub;
			opt = chgpw_name(pw->pw_name,pw);
		} else {
			sp->sp_pwdp = pwbuf;
			opt = addshent(sp);
		}
	} else if (pw->pw_passwd && *pw->pw_passwd) {
		if ((opt=delsh_name(pw->pw_name)) != Success) {
			fputs("Warning: password,shadow files may be inconsistant\n",stderr);
	//		unlock_passwd();
	//		return opt;
		}
		pw->pw_passwd = nil_passwd;
		opt=chgpw_name(pw->pw_name,pw);
	}
	unlock_passwd();
	return opt;
}

int main(int argc, char **argv) {
	struct passwd  *p;
	int             i, ecode;
	
	init_defaults(basename(argv[0]), "passwd");
	while ((i=getopt(argc,argv,"c")) != EOF) {
		switch (i) {
		case 'c':
			if (getuid() != 0) {
				fprintf(stderr,"%s: must be root to set chroot option\n", argv[0]);
				return EXIT_FAILURE;
			}
			passwd_stub = CHROOTED_PASSWD;
			break;
		default:
			fprintf(stderr,"%s: bad option %c\n", argv[0], i);
			return EXIT_FAILURE;
		}
	}
	if (optind == argc) {
		if (!(p = getpwuid(getuid()))) {
			fprintf(stderr, "%s: cannot locate your password entry\n", argv[0]);
			return EXIT_FAILURE;
		}
		return change_passwd(p);
	}
/* passwd should use getopt to support -- syntax */
	ecode = Success;
	for (i = optind; i < argc; i++) {
		if (!(p = getpwnam(argv[i]))) {
			if (getuid() != 0) {
				fprintf(stderr, "%s: must be root to create new accounts\n",
						argv[0]);
				return EXIT_FAILURE;
			}
			if (strlen(argv[i]) > LOGIN_NAME_MAX) {
				fprintf(stderr,"%s: username exceeds maximum length of %d characters\n", argv[0], LOGIN_NAME_MAX);
				return EXIT_FAILURE;
			}
			if (strchr(argv[i], ':')) {
				fprintf(stderr, "%s: username cannot contain ':' character\n",
						argv[0]);
				return EXIT_FAILURE;
			}
			if ((ecode=create_account(argv[i])) != Success) {
				return ecode;
			}
		} else if (p->pw_uid != getuid() && getuid() != 0) {
			fprintf(stderr, "%s: sorry, only root may change another user's password\n", argv[0]);
		} else if ((ecode=change_passwd(p)) != Success) {
			return ecode;
		}
	}
	return ecode;
}
