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




#include <libc.h>
#include <env.h>
#include <pwd.h>
#include <process.h>

#include <termios.h>
#include <sys/dev.h>
#include <sys/stat.h>
#include <sys/fsys.h>
#include <sys/disk.h>

#ifdef __QNXNTO__
#undef BROKEN_CHROOT
#endif

#include <login.h>

#define	TXT(s)		(s)
#define T_CNTRL_TERM 		"login: warning- no controlling terminal"
#define T_LOGIN_FAIL 		"login: login incorrect"
#define T_SESSION_LEADER	"login: not login shell"
#define T_MUST_TTY		"login: must be on a tty!"
#define	T_NO_MEMORY		"login: no memory!"
#define	T_NO_SHELL 		"login: no shell!"
#define	T_NO_PASSFILE		"login: no passwd file!"
#define	T_OWN_ME		"login: passwd file must be owned by %s\n"
#define	T_PASSWD_FTYPE		"login: passwd file must be regular file\n"




#ifndef UNIX_LOGGING
#define UNIX_LOGGING
#endif
#ifndef QNX_LOGGING
#define	QNX_LOGGING
#endif

#define	ROOT_UID	0
#define	DEF_UMASK	0022
#define	TTY_PERM	0622

static char rcsid[] = "$Id: login.c 153052 2008-08-13 01:17:50Z coreos $";
#define	MAX_RETRIES	5

struct passwd	*getpwentry(char *name);
static int preserve = 0;

static char nbuf[MAX_RETRIES * (L_cuserid+1)];
static char *lname[MAX_RETRIES];
static char *hostname = "qnx";

static time_t	ltimes[MAX_RETRIES];

static char	lattempt[L_cuserid+1];
static char	tty_name[L_ctermid+1];

static int	passwd_stat;

static struct passwd root_def = {
		"root",
		"",
		0,
		0,
		"",
		"",
		"",
		"/",
		"/bin/sh"
};


#define	LOGIN_TIMEOUT	0

void tsig(int signo)
{
	if (signo)
		;
	fprintf(stderr,"login timed out\n");
	exit(1);
}

login_error(char *fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	fprintf(stderr, "login error: ");
	vfprintf(stderr, fmt, v);
	putc('\n', stderr);
	sleep(15);
	exit(1);
}


chkdb()
{
	unsigned x,i;
	for (i=0; i < 5; i++) {
		switch (x=auth_pwdb()) {
		default:
			return x;
			break;
		case NoPasswd:
		case BusyPasswd:
			sleep(2);
			break;
		}
	}
	return x;
}

check_files()
{
	unsigned x;
	switch (x=chkdb()) {
	case PdbOk:
		break;
	case NoPasswd:
		fprintf(stderr, "login: no password file. logging in as root\n");
		login(&root_def);
		break;
	case NoShadow:
		fprintf(stderr, "login: Warning no shadow file, you may not be able to log in\n");
		break;
	case NotSameDevice:
		login_error("Invalid configuration of passwd/shadow files");
	case PasswdBadType:
		login_error("Password must be a 'regular' file");
	case ShadowBadType:
		login_error("Shadow must be a 'regular' file");
	case InvalidOwner:
		login_error("Password/Shadow must be owned by uid 0");
	case BusyPasswd:
		login_error("Password file update in progress");
	default:
		login_error("Database authentication failed (err=%u)", x);
	}
	return 0;
}

main(int argc, char **argv)
{               
	struct passwd *pw;
	int	retry = 0;
	int	c;
	int	i;
	int	nsecs = LOGIN_TIMEOUT;
	
	for (i=0; i < MAX_RETRIES; i++) {
		lname[i] = nbuf+(L_cuserid+1)*i;
	}
/*
 * qnx doesn't have a getty, so simulate it
 */
	if (getty(tty_name)) {
		login_error(T_MUST_TTY);
	}

	while ((c=getopt(argc,argv,"pt:h:f:")) != EOF) {
		switch (c) {
		case	'f':
			if (getuid() != 0) {
				fprintf(stderr,"login: must be root to run login force\n");
				break;
			}
			if (pw=getpwnam(optarg)) {
				check_files();
				login(pw);
				return EXIT_FAILURE;
			} else {
				fprintf(stderr,"login: no such user '%s'\n",optarg);
				return EXIT_FAILURE;
			}
			break;
		case	'p':
			preserve = 1;
			break;
		case	't':
			nsecs = strtol(optarg,(char **)0,0);
			break;
		case    'h':
			hostname = optarg;
			break;
		default:
			fprintf(stderr,"option '%c' ignored\n",c);
		}
	}
	check_files();
	if (optind == argc)
		cat_file(ISSUE_FILE);
	else
		strncpy(lname[0], argv[optind],L_cuserid);
		
	signal(SIGALRM,tsig);
	if (nsecs)
		alarm(nsecs);
		
	for (retry=0; retry < MAX_RETRIES; retry++) {
		pw = chklogin(passwd_stat,lname[retry]);
		ltimes[retry] = time(NULL);
		if (pw) {
			login(pw);
			return EXIT_FAILURE;
		}
	}
	fprintf(stderr,"%s\n",TXT(T_LOGIN_FAIL));
	log_failure(tty_name,lname,ltimes,MAX_RETRIES);
	return EXIT_FAILURE;
}

#ifdef BROKEN_CHROOT

#include <sys/magic.h>

chroot(char *path)
{
	char      buf[PATH_MAX];

//	buf[0] = '-';
//	qnx_fullpath(buf+1, path);
	strcpy(buf+0, path);
	__MAGIC.sptrs[4] = strdup(buf);
	return 0;
}

login_setroot(struct passwd *pw)
{
	char         buf[PATH_MAX];
	buf[0] = '-';
	qnx_fullpath(buf+1, pw->pw_dir);
	chdir(buf+1);
	__MAGIC.sptrs[4] = strdup(buf);
	/* don't check response -- chdir will fail */
	return 0;
}
#else
login_setroot(struct passwd *pw)
{
	if (chroot(pw->pw_dir) == -1) {
		fprintf(stderr, "login: change root to '%s' failed -- '%s'\n",
				pw->pw_dir, strerror(errno));
		return -1;
	}
	if (chdir("/") == -1) {
		fprintf(stderr, "login: cannot change to new root directory - '%s'\n",
			strerror(errno));
		return -1;
	}
	return 0;
}

#endif


login(struct passwd *pw)
{

	char utmpfile[32];

	if (pw->pw_uid != 0 && cat_file(STOP_LOGIN)) {
		fprintf(stderr,"login: login is currently disabled -- contact system administrator\n");
		return 1;
	}

#ifdef UNIX_LOGGING
	sprintf(utmpfile, "/usr/adm/utmp.%d", getnid());
        utmpname(utmpfile);
	unix_logging(pw,tty_name);
	bsd_lastlog(pw,tty_name);
#endif

#ifdef QNX_LOG
	qnx_pwlog(pw,"LO",tty_name);
#endif

	chmod(tty_name,TTY_PERM);
	chown(tty_name,pw->pw_uid,pw->pw_gid);

	if (sid_name(getsid(0), pw->pw_name)) {
		fprintf(stderr, "login: cannot find session name (%s)\n",
			strerror(errno));
		return 1;
	}		

	build_env(ENV_SAVE,preserve);
	
	if (strcmp(pw->pw_passwd, "*") == 0) {
		if (login_setroot(pw) == -1) {
			return 1;
		}
	} else if (chdir(pw->pw_dir) != 0) {
		fputs("No directory!  Logging in with home=/\n",stderr);
		pw->pw_dir = "/";
		if (chdir(pw->pw_dir) != 0) {
			fputs("Cannot set home to /, giving up\n", stderr);
			return 1;
		}
	}	
	setenv("HOME",pw->pw_dir,1);
	setenv("LOGNAME",pw->pw_name,1);
	setenv("SHELL",pw->pw_shell,1);
	umask(DEF_UMASK);

	setgid(pw->pw_gid);		setegid(pw->pw_gid);
	setuid(pw->pw_uid);		seteuid(pw->pw_uid);
	alarm(0);
	run_shell(pw);
	fprintf(stderr,"login: cannot start login shell '%s':%s\n",
			pw->pw_shell, strerror(errno));
	return 1;
}
