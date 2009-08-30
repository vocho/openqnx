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
 Once upon a time this source was related to the FreeBSD login, but
 since that time it has been quite munged, and cleaned up of 
 extraneous functionality:

 I've left the stubs in for functionality that I think that we
 might be interested in the future with (ie user account expiry,
 user denial, PAM authentication), but removed functionality 
 that doesn't belong in login (ie display motd).

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>				
#include <signal.h>
#include <limits.h>

#include <err.h>
#include <errno.h>
#include <grp.h>

#include <syslog.h>					
#include <utmp.h>

#include <pwd.h>

#include "pathnames.h"
#include <libgen.h>

#if defined(__QNXNTO__)
#define NO_PAM
#define TRYSHADOW
extern char *qnx_crypt(const char *, const char *);
#include "proto.h"
#endif

#ifndef NO_PAM
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#endif

#if defined(TRYSHADOW)
#include <shadow.h>
#endif

void	badlogin __P((char *));
void	checknologin __P((void));
void	dolastlog __P((int, char *, char *));
void	getloginname __P((void));
int		rootterm __P((char *));
void	sigint __P((int));
void	sleepexit __P((int));
void	refused __P((char *,char *,int));
void	timedout __P((int));

#ifndef NO_PAM
static int auth_pam __P((void));
#endif
int auth_traditional __P((int qnxcrypt));

#define	TTYGRPNAME	"tty"		/* name of group to own ttys */
#define	DEFAULT_BACKOFF	3
#define	DEFAULT_RETRIES	10
#define	DEFAULT_TIMEOUT	0


/* Flags which are used in the life of the program */
#define SET_FLAG(x, flag)	((x) |= (flag))
#define CLEAR_FLAG(x, flag)	((x) &= ~(flag))
#define FL_CHANGE_PASSWD	(1<<0)		//Force a change of passwds
#define FL_QNX_CRYPT		(1<<1)		//Use QNX4 crypt only
#define FL_ROOT_OK			(1<<2)		//Allow root to login
#define FL_ROOT_LOGIN		(1<<4)		//The user is root
#define FL_ASK_LOGIN		(1<<5)		//Prompt the user for login
#define FL_QUIET_LOGIN		(1<<6)		//Perform a quiet login
#define FL_FORCE_LOGIN		(1<<7)		//Force login, don't authenticate
#define FL_PRESERVE_ENV		(1<<8)		//Preserve existing environment
#define FL_UNIX_CRYPT		(1<<9)		//Use Unix crypt only

jmp_buf timeout_buf;
struct	passwd *pwd;
int		failures, g_flags = 0;
char	*hostname, *username, *tty;

int main(int argc, char **argv) {
	struct	group *gr;
	int		retries, backoff, ch, cnt, rval;
	char	tbuf[MAXPATHLEN + 2];
	char	tname[sizeof(_PATH_TTY) + 10];
	char	localhost[MAXHOSTNAMELEN];
	char	*ttyn, *term, *shell;
	char	*saved_env;
	int		saved_env_len, timeout;
	int i;
	int spaces;
	int cindex;
	int carg;
	int slen;
	char **args;
	char *temp;

	signal(SIGQUIT, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	if (setjmp(timeout_buf)) {
		if (failures)
			badlogin(tbuf);
		fprintf(stderr, "Login timed out \n");
		exit(0);
	}
	signal(SIGALRM, timedout);

	// Close all but three of the process descriptors
	for (cnt = getdtablesize(); cnt > 2; cnt--) {
		close(cnt);
	}

	openlog("login", LOG_ODELAY, LOG_AUTH);

	/*
	 * -p is used by getty to tell login not to destroy the environment (preserve)
	 * -f is used to skip a second login authentication (force)
	 * -q is used to disable output message (quiet)
	 * -c is used to indicate that only QNX4 crypt should be used
	 * -u is used to indicate that only Unix crypt should be used
	 * -t is used to specify a new timeout value (0 (default) disables timeouts)
	 */
	g_flags = FL_ASK_LOGIN;
	term = shell = saved_env = NULL;
	timeout = DEFAULT_TIMEOUT;
	retries = DEFAULT_RETRIES;
	backoff = DEFAULT_BACKOFF;
	failures = 0;

	if (gethostname(localhost, sizeof(localhost)) < 0) {
		syslog(LOG_ERR, "couldn't get local hostname: %m");
	}
	hostname = localhost;

	while ((ch = getopt(argc, argv, "t:fpcuqh:")) != -1) {
		switch (ch) {
		case 'c':
			g_flags &= ~(FL_QNX_CRYPT|FL_UNIX_CRYPT);
			g_flags |= FL_QNX_CRYPT;
			break;
		case 'u':
			g_flags &= ~(FL_QNX_CRYPT|FL_UNIX_CRYPT);
			g_flags |= FL_UNIX_CRYPT;
			break;
		case 't':
			timeout = strtoul(optarg, NULL, 10);
			break;
		case 'q':
			g_flags |= FL_QUIET_LOGIN;
			break;
		case 'f':
			g_flags |= FL_FORCE_LOGIN;
			break;
		case 'p':
			g_flags |= FL_PRESERVE_ENV;
			break;
		case 'h':
			//I need to take the hostname here
			hostname = optarg;	
			break;
		case '?':
		default:
			fprintf(stderr, "Usage: login [-fpcq] [-t timeout] [[username] [env ...]]\n");
			exit(EXIT_FAILURE);	
		}
	}
	argc -= optind;
	argv += optind;

	if (*argv) {
		username = *argv;
		g_flags &= ~FL_ASK_LOGIN;
	}

	//Get the terminal name we are loggin in from
	ttyn = ttyname(STDIN_FILENO);
	if (ttyn == NULL || *ttyn == '\0') {
		(void)snprintf(tname, sizeof(tname), "%s??", _PATH_TTY);
		ttyn = tname;
	}
	if ((tty = strrchr(ttyn, '/')) != NULL)
		tty++;
	else
		tty = ttyn;

	//Set the timeout alarm
	alarm(timeout);

	for (cnt = 0;; g_flags |= FL_ASK_LOGIN) {
		if (g_flags & FL_ASK_LOGIN) {
			g_flags &= ~FL_FORCE_LOGIN;
			getloginname();
		}

		g_flags &= ~FL_ROOT_LOGIN;
		g_flags |= (rootterm(tty)) ? FL_ROOT_OK : 0;
		if (strlen(username) > LOGIN_NAME_MAX) {
			username[LOGIN_NAME_MAX] = '\0';
		}

		/*
		 * Note if trying multiple user names; log failures for
		 * previous user name, but don't bother logging one failure
		 * for nonexistent name (mistyped username).
		 */
		if (failures && strcmp(tbuf, username)) {
			if (failures > (pwd ? 0 : 1))
				badlogin(tbuf);
		}
		strncpy(tbuf, username, sizeof tbuf-1);
		tbuf[sizeof tbuf-1] = '\0';

		/*
		 * if we have a valid account name, and it doesn't have a
		 * password, or the -f option was specified and the caller
		 * is root or the caller isn't changing their uid, don't
		 * authenticate.
		 */
		rval = 1;
		if ((pwd = getpwnam(username)) != NULL) {
			if (pwd->pw_uid == 0) {
				g_flags |= FL_ROOT_LOGIN;
			}

			if (g_flags & FL_FORCE_LOGIN && 
			    (getuid() == 0 || getuid() == pwd->pw_uid)) {
				break;
			} else if (!pwd->pw_passwd || pwd->pw_passwd[0] == '\0') {
				if (!(g_flags & FL_ROOT_LOGIN) || (g_flags & FL_ROOT_OK)) {
					/* pretend password okay */
					rval = 0;
					goto ttycheck;
				}
			}
		}
		g_flags &= ~FL_FORCE_LOGIN;

#ifndef NO_PAM
		/*
		 * Try to authenticate using PAM.  If a PAM system error
		 * occurs, perhaps because of a botched configuration,
		 * then fall back to using traditional Unix authentication.
		 *
		 * Skip PAM authentication if not needed
		 */
		if (rval && (rval = auth_pam()) == -1)
#endif /* NO_PAM */
			rval = (rval != 0) ? auth_traditional(g_flags) : rval;


#ifndef NO_PAM
		/*
		 * PAM authentication may have changed "pwd" to the
		 * entry for the template user.  Check again to see if
		 * this is a root login after all.
		 */
		if (pwd != NULL && pwd->pw_uid == 0)
			g_flags |= FL_ROOT_LOGIN;
#endif /* NO_PAM */

	ttycheck:
		/*
		 * If trying to log in as root without Kerberos,
		 * but with insecure terminal, refuse the login attempt.
		 */
		if (pwd && !rval) {
			if (g_flags & FL_ROOT_LOGIN && !(g_flags & FL_ROOT_OK)) {
				refused(NULL, "NOROOT", 0);
			}
			else { 
				break;
			}
		}
		printf("Login incorrect\n");
		failures++;

		/*
		 * we allow up to 'retry' (10) tries,
		 * but after 'backoff' (3) we start backing off
		 */
		if (++cnt > backoff) {
			if (cnt >= retries) {
				badlogin(username);
				sleepexit(1);
			}
			sleep((cnt - backoff) * 5);
		}
	}

	/* committed to login -- turn off timeout */
	alarm(0);
	endpwent();

	/* Set ourselves up in a home directory */
	if (!*pwd->pw_dir || chdir(pwd->pw_dir) < 0) {
		if (chdir("/") < 0) 
			refused("Cannot find root directory", "ROOTDIR", 1);
		pwd->pw_dir = "/";
		if (!(g_flags & FL_QUIET_LOGIN) || *pwd->pw_dir)
			printf("No home directory.\nLogging in with home = \"/\".\n");
	}

//At some point in time we may want to have passwd expiry ability
#if !defined(__QNXNTO__)
	if (pwd->pw_change || pwd->pw_expire)
		(void)gettimeofday(&tp, (struct timezone *)NULL);
#define DEFAULT_WARN  (2L * 7L * 86400L)  /* Two weeks */

	warntime = login_getcaptime(lc, "warnpassword", DEFAULT_WARN, DEFAULT_WARN);

	if (pwd->pw_change) {
		if (tp.tv_sec >= pwd->pw_change) {
			(void)printf("Sorry -- your password has expired.\n");
			changepass=1;
			syslog(LOG_INFO, "%s Password expired - forcing change", pwd->pw_name);
		} else if (pwd->pw_change - tp.tv_sec < warntime && !quietlog)
		    printf("Warning: your password expires on %s", ctime(&pwd->pw_change));
	}

	warntime = login_getcaptime(lc, "warnexpire", DEFAULT_WARN, DEFAULT_WARN);

	if (pwd->pw_expire) {
		if (tp.tv_sec >= pwd->pw_expire) {
			refused("Sorry -- your account has expired", "EXPIRED", 1);
		} else if (pwd->pw_expire - tp.tv_sec < warntime && !quietlog)
		    printf("Warning: your account expires on %s", ctime(&pwd->pw_expire));
	}

	//You can also put host, time, tty authentication in
	//here as well to see if they match what we want 
	if (login_access(pwd->pw_name, hostname ? hostname : tty) == 0)
		refused("Permission denied", "ACCESS", 1);
#endif

	/* Set up the user shell properly, default to /bin/sh */
	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = _PATH_BSHELL;
	shell = pwd->pw_shell;
	if ((shell = strdup(shell)) == NULL) {
		syslog(LOG_NOTICE, "memory allocation error");
		sleepexit(1);
	}

	/* Fill in the wtmp/utmp entry */
	dolastlog(g_flags & FL_QUIET_LOGIN, tty, hostname);

	/* Make us the owner of this tty: Security hole here?? */
	chown(ttyn, pwd->pw_uid, (gr = getgrnam(TTYGRPNAME)) ? gr->gr_gid : pwd->pw_gid);
	//chown(ttyn, pwd->pw_uid, pwd->pw_gid);

	/*
	 * Syslog each successful login, so we don't have to watch hundreds
	 * of wtmp or lastlogin files.
	 */
#ifdef LOGALL
	if (hostname)
		syslog(LOG_INFO, "login from %s on %s as %s", hostname, tty, pwd->pw_name);
	else
		syslog(LOG_INFO, "login on %s as %s", tty, pwd->pw_name);
#endif
	/*
	 * Log a message when we have a root login
	 */
	if (g_flags & FL_ROOT_LOGIN) {
		if (hostname)
			syslog(LOG_NOTICE, "ROOT LOGIN (%s) ON %s FROM %s", username, tty, hostname);
		else
			syslog(LOG_NOTICE, "ROOT LOGIN (%s) ON %s", username, tty);
	}

	/*
	 * Destroy environment unless user has requested its preservation.
	 * We need to do this before setusercontext() because that may
	 * set or reset some environment variables.
	 *
	 * Preserve TERM if it happens to be already set.
	 * Preserve any flags that live according to the /etc/default/login
	 * 
	 */
	if ((term = getenv("TERM")) != NULL)
		term = strdup(term);

	saved_env = save_list(LOGIN_ENV_SAVE, &saved_env_len, g_flags & FL_PRESERVE_ENV);
	if (!(g_flags & FL_PRESERVE_ENV)) {
		clearenv();
	}

	/*
	 * We don't need to be root anymore, so
	 * set the user and session context
	 */
#if !defined(__QNXNTO__)		/* This isn't defined for NTO yet */
	if (setlogin(username) != 0) {
                syslog(LOG_ERR, "setlogin(%s): %m - exiting", username);
		exit(1);
	}
#endif
	if (setusercontext(pwd, pwd->pw_uid, LOGIN_SETALL & ~LOGIN_SETLOGIN) != 0) {
        syslog(LOG_ERR, "setusercontext() failed - exiting");
		exit(1);
	}

	/* If we have environment to restore do it */ 
	if (saved_env) {
		while (saved_env_len > 0 && strlen(saved_env) != 0) {
			int env_length;
			add_env(saved_env);
			env_length = strlen(saved_env)+1;
			saved_env_len -= env_length;
			saved_env += env_length;
		}	
	}
	/* 
	 * Add to the environment first from the command line 
	 * Note that argv, argc have been munged from above 
	 */
	for (cnt = 1; cnt < argc; cnt++) {
		add_env(argv[cnt]);
	}

	setenv("SHELL", pwd->pw_shell, 1);
	setenv("HOME", pwd->pw_dir, 1);
	if (term != NULL && *term != '\0')
		setenv("TERM", term, 1);			/* Preset overrides */
	else {
		setenv("TERM", "qansi-m", 0);		/* Fallback doesn't */
	}
	setenv("LOGNAME", username, 1);
	setenv("USER", username, 1);
	//If this is already set (preserved) then don't mess with it
	if(getenv("PATH") == NULL) {
		char *newpath;
		int   newplen;
		newpath = alloca((newplen = confstr(_CS_PATH, NULL, 0)) + 1);
		if (newpath && confstr(_CS_PATH, newpath, newplen + 1) > 0) {
			setenv("PATH", newpath, 1);
		}
	}

	/* Reset all of our signals to their default values */
	signal(SIGALRM, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGTSTP, SIG_IGN);

	/* Force a change of password?  Could be handy in the future */
	if (g_flags & FL_CHANGE_PASSWD) {
		pid_t pid = fork();
		int status;
		switch(pid) {
			case -1:
				perror("fork:");
				exit(EXIT_FAILURE);
			case 0:
				execlp(_PATH_CHPASS, _PATH_CHPASS, (char*)0);
				perror("execlp:");
				exit(EXIT_FAILURE);
			default:
				waitpid(pid, &status, 0);
				if (status){
					exit(EXIT_FAILURE);
				}
		}
	}


	closelog();
	

	spaces = 0;
	cindex = 0;
	carg = 0;
	slen = strlen(shell);
	for(i = 0; i < strlen(shell); i++){
		if (shell[i] == ' '){
			spaces++;
		}
	}
	args = malloc(sizeof(char*) * (spaces+2));
	if (!args){
		err(1, "malloc");
	}
	args[0] = &shell[0];
	for(i = 0; i < slen;i++){
		if (shell[i] == ' '){
			args[carg] = &shell[cindex];
			shell[i] = 0;
			cindex = i + 1;
			carg++;
		}
	}
	args[carg] = &shell[cindex];
	args[carg+1] = 0;
	
	/*
		* Login shells have a leading '-' in front of argv[0]
		* We also need tbuf to be the actual shell path, and args[0]
		* to be the executable name plus a -.
		*/
	temp = strdup(args[0]);
	if (!temp){
		err(1, "strdup");
	}
	tbuf[0] = '-';
	strcpy(tbuf + 1, (term = strrchr(args[0], '/')) ? term + 1 : args[0]);
	
	args[0] = strdup(tbuf);
	if (!args[0]){
		err(1, "strdup");
	}
	
	snprintf(tbuf, sizeof(tbuf), "%s", temp);
	
	execvp(tbuf, args);
	err(1, "%s", args[0]);



	
}

int auth_traditional(int flags) {
	int rval;
	char *p, *qnxp, *unixp, *salt, *op;
#if defined(TRYSHADOW)
	struct spwd *sp;
#endif
	rval = 1;

	p = getpass("Password:");

#if defined(TRYSHADOW)
	{
        //Here we should start by looking at the shadow
        //password and if required go to the user password
        if ((pwd == NULL) || ((sp = getspnam(pwd->pw_name)) == NULL)) {
				op = (pwd) ? pwd->pw_passwd : NULL;
                salt = (pwd) ? pwd->pw_passwd : "xx";
		}
        else {
				op = sp->sp_pwdp;
                salt = sp->sp_pwdp;
		}
		endspent();
	}
#else
	op = (pwd) ? pwd->pw_passwd : NULL;
	salt = pwd != NULL ? pwd->pw_passwd : "xx";
#endif

	//We do this to passify people living in both qnx/unix worlds
	if (flags & (FL_QNX_CRYPT|FL_UNIX_CRYPT)) {
		unixp = qnxp = (flags & FL_QNX_CRYPT) ? qnx_crypt(p, salt)
											  : crypt(p, salt);
	}
	else {
		qnxp = qnx_crypt(p, salt);
		unixp = crypt(p, salt);
	}

	if (op) {
		if (!p[0] && op[0])
			qnxp = unixp = ":";
		if (strcmp(qnxp, op) == 0 || strcmp(unixp, op) == 0) 
			rval = 0;
	}

	/* clear entered password */
	memset(p, 0, strlen(p));
	return rval;
}

#ifndef NO_PAM
/*
 * Attempt to authenticate the user using PAM.  Returns 0 if the user is
 * authenticated, or 1 if not authenticated.  If some sort of PAM system
 * error occurs (e.g., the "/etc/pam.conf" file is missing) then this
 * function returns -1.  This can be used as an indication that we should
 * fall back to a different authentication mechanism.
 */
static int auth_pam() {
	pam_handle_t *pamh = NULL;
	const char *tmpl_user;
	const void *item;
	int rval;
	int e;
	static struct pam_conv conv = { misc_conv, NULL };

	if ((e = pam_start("login", username, &conv, &pamh)) != PAM_SUCCESS) {
		syslog(LOG_ERR, "pam_start: %s", pam_strerror(pamh, e));
		return -1;
	}
	if ((e = pam_set_item(pamh, PAM_TTY, tty)) != PAM_SUCCESS) {
		syslog(LOG_ERR, "pam_set_item(PAM_TTY): %s",
		    pam_strerror(pamh, e));
		return -1;
	}
	if (hostname != NULL &&
	    (e = pam_set_item(pamh, PAM_RHOST, hostname)) != PAM_SUCCESS) {
		syslog(LOG_ERR, "pam_set_item(PAM_RHOST): %s",
		    pam_strerror(pamh, e));
		return -1;
	}
	e = pam_authenticate(pamh, 0);
	switch (e) {

	case PAM_SUCCESS:
		/*
		 * With PAM we support the concept of a "template"
		 * user.  The user enters a login name which is
		 * authenticated by PAM, usually via a remote service
		 * such as RADIUS or TACACS+.  If authentication
		 * succeeds, a different but related "template" name
		 * is used for setting the credentials, shell, and
		 * home directory.  The name the user enters need only
		 * exist on the remote authentication server, but the
		 * template name must be present in the local password
		 * database.
		 *
		 * This is supported by two various mechanisms in the
		 * individual modules.  However, from the application's
		 * point of view, the template user is always passed
		 * back as a changed value of the PAM_USER item.
		 */
		if ((e = pam_get_item(pamh, PAM_USER, &item)) ==
		    PAM_SUCCESS) {
			tmpl_user = (const char *) item;
			if (strcmp(username, tmpl_user) != 0)
				pwd = getpwnam(tmpl_user);
		} else
			syslog(LOG_ERR, "Couldn't get PAM_USER: %s",
			    pam_strerror(pamh, e));
		rval = 0;
		break;

	case PAM_AUTH_ERR:
	case PAM_USER_UNKNOWN:
	case PAM_MAXTRIES:
		rval = 1;
		break;

	default:
		syslog(LOG_ERR, "auth_pam: %s", pam_strerror(pamh, e));
		rval = -1;
		break;
	}
	if ((e = pam_end(pamh, e)) != PAM_SUCCESS) {
		syslog(LOG_ERR, "pam_end: %s", pam_strerror(pamh, e));
		rval = -1;
	}
	return rval;
}
#endif /* NO_PAM */


/*
 * Allow for authentication style and/or kerberos instance
 * 
 */
#define	NBUFSIZ		UT_NAMESIZE + 64
void getloginname() {
	int ch;
	char *p;
	static char nbuf[NBUFSIZ];

	for (;;) {
		(void)printf("login: ");
		for (p = nbuf; (ch = getchar()) != '\n'; ) {
			if (ch == EOF) {
				badlogin(username);
				exit(0);
			}
			if (p < nbuf + (NBUFSIZ - 1))
				*p++ = ch;
		}
		if (p > nbuf) {
			if (nbuf[0] == '-')
				(void)fprintf(stderr, "login names may not start with '-'.\n");
			else {
				*p = '\0';
				username = nbuf;
				break;
			}
		}
	}
}


/* 
 This writes out the lastlog, utmp and wtmp information.
 It follows the NetBSD format of writing out a LASTLOG,
 then putting the same information in the UTMP and WTMP
 assuming that init (or some similar program) will write
 out a cleared entry when the user logs out.
*/
void dolastlog(int quiet, char *tty, char *hostname) {
	struct lastlog ll;
	struct utmp utmp;
	int lastfd;

	/* Fill in parts of our utmp structure.  This utmp
	   structure is from SUN, so it isn't exactly compatible */
	memset((void *)&utmp, 0, sizeof(utmp));
	time(&utmp.ut_time);
	strncpy(utmp.ut_name, pwd->pw_name, sizeof(utmp.ut_name));
	strncpy(utmp.ut_line, tty, sizeof(utmp.ut_line));

	/* Write out to the last log file, and print the last log if not quiet */
	lastfd = open(_PATH_LASTLOG, O_RDWR, 0);
	if (!quiet && lastfd != -1) {
		lseek(lastfd, (off_t)pwd->pw_uid * sizeof(ll), L_SET);
		if (read(lastfd, (char *)&ll, sizeof(ll)) == sizeof(ll) && ll.ll_time != 0) {
			printf("Last login: %.*s ", 24-5, (char *)ctime(&ll.ll_time));
			if (*ll.ll_host != '\0')
				printf("from %.*s\n", (int)sizeof(ll.ll_host), ll.ll_host);
			else
				printf("on %.*s\n", (int)sizeof(ll.ll_line), ll.ll_line);
		}
		lseek(lastfd, (off_t)pwd->pw_uid * sizeof(ll), L_SET);
	}
	if (lastfd != -1) {
		memset((void *)&ll, 0, sizeof(ll));
		time(&ll.ll_time);
		strncpy(ll.ll_line, tty, sizeof(ll.ll_line));
		if (hostname) 
			strncpy(ll.ll_host, hostname, sizeof(ll.ll_host));
	
		write(lastfd, (char *)&ll, sizeof(ll));
		close(lastfd);
	}

#if 0
	/* Write out to the utmp file */
	utmpname(UTMP_FILE);
	setutent();
	utmp.ut_type = LOGIN_PROCESS;
	pututline(&utmp);	
	endutent();

	/* Write out the the wtmp file */
	utmpname(WTMP_FILE);
	setutent();
	utmp.ut_type = ACCOUNTING;
	pututline(&utmp);	
	endutent();
#else
	login(&utmp);
#endif
}

/*
 If we have had a bad login, then log a message to 
 the syslog if possible.  If no syslog then we just ignore
*/
void badlogin(char *name) {
	if (failures == 0)
		return;
	if (hostname) {
		syslog(LOG_NOTICE, "%d LOGIN FAILURE%s FROM %s",
		    failures, failures > 1 ? "S" : "", hostname);
		syslog(LOG_AUTHPRIV|LOG_NOTICE,
		    "%d LOGIN FAILURE%s FROM %s, %s",
		    failures, failures > 1 ? "S" : "", hostname, name);
	} else {
		syslog(LOG_NOTICE, "%d LOGIN FAILURE%s ON %s",
		    failures, failures > 1 ? "S" : "", tty);
		syslog(LOG_AUTHPRIV|LOG_NOTICE,
		    "%d LOGIN FAILURE%s ON %s, %s",
		    failures, failures > 1 ? "S" : "", tty, name);
	}
	failures = 0;
}

/*
 This is used to log a message to the syslog when
 we are refusing a user for one reason or another.
 Could be rolled with the badlogin message perhaps
*/
void refused(char *msg, char *rtype, int lout) {
	if (msg != NULL)
	    printf("%s.\n", msg);
	if (hostname)
		syslog(LOG_NOTICE, "LOGIN %s REFUSED (%s) FROM %s ON TTY %s",
		       pwd->pw_name, rtype, hostname, tty);
	else
		syslog(LOG_NOTICE, "LOGIN %s REFUSED (%s) ON TTY %s",
		       pwd->pw_name, rtype, tty);
	if (lout)
		sleepexit(1);
}

/*
 Determine if this terminal is OK for root to log in on
 At some point in the future we may want to allow root
 only on some terminals so leave this in. 
 Return 1 ok to log root on this terminal.
*/
int rootterm(char *ttyn) {
	return(1);
}

/*
 Delayed exit routine
*/
void sleepexit(int eval) {
	sleep(5);
	exit(eval);
}

/* ARGSUSED */
void timedout(int  signo) {
	longjmp(timeout_buf, signo);
}
