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





#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <paths.h>

#ifndef DONTUSESYSLOG
#include <syslog.h>
#endif

#ifdef USELIMITS
#include <sys/resources.h>
#endif

#ifdef _SC_GETGR_R_SIZE_MAX
#define GRMAX ((sysconf(_SC_GETGR_R_SIZE_MAX) == -1) ? 16 : sysconf(_SC_GETGR_R_SIZE_MAX))
#else
#define GRMAX 16
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

char* crypt(const char*, const char*);

#define PROGNAME "newgrp"

/*
 *  Print a usage message and terminate the process.
 */
void usage()
{
	fprintf(stderr, "usage: %s [-l | -] [group]\n", PROGNAME);
	exit(1);
}


#ifndef DONTUSESYSLOG
/*
 * Create a pretty string for the syslog message descriping the terminal.
 */
char* ontty()
{
	char *p;
	static char buf[32 + 4 + 1];

	buf[0] = 0;
	if (p = ttyname(STDERR_FILENO)) {
		snprintf(buf, sizeof buf, " on %s", p);
	}
	return buf;
}
#endif


#if (NGROUPS_MAX > 1)
/*
 *  Add the given group id to the array of group ids.
 */
int addtolist(gid_t newgrp, int gnum, gid_t *glist) {
	int x;

	/* if there is room, add a gid to list */
	if (gnum < NGROUPS_MAX) {
		for (x = 0; (x < gnum) && (glist[x] != newgrp); x++);
		if (x >= gnum) {
			glist[gnum++] = newgrp;
		}
	}
	return gnum;
}
#endif


int granted(const char* newgrpname, struct passwd** ppwd)
{
	uid_t	ruid;
	gid_t	newgrp;
	struct group  *grp;

	/*
	 * Initialization.
	 */
	*ppwd = getpwuid(ruid = getuid());

	/*
	 * The following are implementation defined security restrictions.
	 * If euid is not root then reject -- need a setuid-root exectuable.
 	 * If ruid is not root and not in /etc/passwd then reject.
	 */
	if (geteuid() || (ruid && (*ppwd == NULL))) {
		fprintf(stderr, "%s: Who are you?\n", PROGNAME);
		errno = EPERM;
		return -1;
	}

	/*
	 * No operands -- change back to /etc/passwd defaults.
	 */
	if (newgrpname == NULL) {
		/* change group(s) back to original "login fresh" values */
#if (NGROUPS_MAX > 1)
		initgroups((*ppwd)->pw_name, (*ppwd)->pw_gid);
#endif
		/* success */
		return (*ppwd)->pw_gid;
	}

	/*
	 * If given group name exists in /etc/group, then use it,
	 * else assume given group name is really the numeric group id.
	 */
	newgrp = -1;
	if ((grp = getgrnam(newgrpname)) == NULL) {
		char *temp;

		/* if a non-negative numeric group id was given, then use it */
		newgrp = (gid_t) strtol(newgrpname, &temp, 0);
		if ((newgrpname[0] == '\0') || (temp == NULL) ||
			(*temp != '\0') || (newgrp < 0) ||
			(((grp = getgrgid(newgrp)) == NULL) && ruid)) {
			fprintf(stderr, "%s: Unknown group %s\n", PROGNAME, newgrpname);
			errno = EINVAL;
			return -1;
		}
	}
	newgrp = (grp) ? grp->gr_gid : newgrp;

	/*
	 * If one of the following conditions are true, we can skip any
	 * additional security mechanisms:
	 *     - real user id is root
	 *     - new group id matches group id listed in /etc/passwd for user
	 */
	if ((ruid) && (newgrp != (*ppwd)->pw_gid)) {
		int rc;
		char **m, *temp;

		/* Check if the user listed as member of group */
		for (m = grp->gr_mem; *m && strcmp(*m, (*ppwd)->pw_name); m++);

		/* if not member, perform yet another security check */
		if (*m == NULL) {
			/* if no group passwd exists then reject */
			/* else prompt for passwd */
			if (grp->gr_passwd && (grp->gr_passwd[0] != '\0') &&
				(grp->gr_passwd[0] != '*')) {
				temp = getpass("Password:");
				rc = strcmp(crypt(temp, grp->gr_passwd), grp->gr_passwd);
				memset(temp, 0x00, strlen(temp));
				if (rc) {
					fprintf(stderr, "%s: Sorry\n", PROGNAME);
#ifndef DONTUSESYSLOG
					syslog(LOG_AUTH|LOG_WARNING, "BAD NEWGRP by %s to %s%s",
						(*ppwd)->pw_name, newgrpname, ontty());
#endif
					errno = EPERM;
					return -1;
				}
			} else {
				fprintf(stderr, "%s: Sorry\n", PROGNAME);
				errno = EINVAL;
				return -1;
			}
		}
	}

#if (NGROUPS_MAX > 1) /* Does this system support supplemental groups */
	{	
		int gnum;
		gid_t glist[NGROUPS_MAX+1];
#ifdef FRUGAL
		int x;
#endif

		if ((gnum = getgroups(NGROUPS_MAX, glist)) != -1) {

#ifdef FRUGAL /* egid should not be in the sup. group list */
			/* if new gid is in list, remove it */
			for (x = 0; x < gnum; x++) {
				if (glist[x] == newgrp) {
					for (gnum--; x < gnum; x++) {
						glist[x] = glist[x+1];
					}
					break;
				}
			}

#else /* This system normally stores the egid in the sup. group list */

			/* if there is room, add new gid to list */
			gnum = addtolist(newgrp, gnum, &glist[0]);
#endif
			/* if there is room, add old gid to list */
			gnum = addtolist(getegid(), gnum, &glist[0]);

			setgroups(gnum, glist);
		}
	}
#endif

	/* success */
	return newgrp;
}


int main(int argc, char* argv[])
{
	char *operand, *temp, *shell, tbuf[PATH_MAX];
	char *newenv[10], *token[] = { NULL, NULL };
	int aslogin, curropt;
	gid_t newgid;
	struct passwd *pwd;
#ifdef USELIMITS
	int prio;
#endif

#ifndef DONTUSESYSLOG
	openlog(PROGNAME, LOG_CONS, 0);
#endif

	/*
	 * Process command line options.
	 */
	aslogin = 0;
	while ((curropt = getopt(argc, argv, "-ls")) != EOF) {
		switch(curropt) {
		case '-':
		case 'l':
			aslogin = 1;
			break;

		case 's':
			aslogin = 0;
			break;

		case '?':
		default:
			usage();
			break;
		}
	}


	/*
	 * Process command line operands.
	 */
	operand = NULL;
	if ((optind+1) < argc) {
		/* too many operands */
		usage();
	} else if ((optind < argc) && ((operand =
		(strlen(argv[optind]) > GRMAX) ? NULL : argv[optind]) == NULL)) {
		/* potential overrun, so ignore operand */
		fprintf(stderr, "%s: Unknown group\n", PROGNAME);
#ifndef DONTUSESYSLOG
		/* assume we are attacked and log message */
		syslog(LOG_AUTH|LOG_ERR,
			"possible buffer overrun attack by uid %d%s\n",
			getuid(), ontty());
#endif
	}


#ifdef USELIMITS
	/*
	 * Be nice.
	 */
	errno = 0;
	prio = getpriority(PRIO_PROCESS, 0);
	if (errno) {
		prio = 0;
	}
	setpriority(PRIO_PROCESS, 0, -2);
#endif

	/*
	 * Do authorization, determine the new group id, attempt
	 * to get passwd struct for user, and change the group id.
	 */
	pwd = NULL;
	if ((newgid = granted(operand, &pwd)) != -1) {
		setgid(newgid);
	}

	/*
	 * Give up setuid-root status.  If this fails, we cannot create
	 * a new shell.
	 */
	if (setuid(getuid()) == -1) {
		/* must abort -- cannot create a superuser shell */
		fprintf(stderr, "%s: setuid: Cannot change user id\n", PROGNAME);
		exit(1);
	}

	/*
	 * Determine which shell to use.  If the shell in the pwd entry
	 * is illegible, then use default path.
	 */
	shell = (pwd && pwd->pw_shell && *pwd->pw_shell) ? pwd->pw_shell :
		_PATH_BSHELL;
	token[0] = (temp = strrchr(shell, '/')) ? (temp+1) : shell;

	/* Do any cleanup before the exec(). */
	fflush(NULL);
#ifdef USELIMITS
	setpriority(PRIO_PROCESS, 0, prio);
#endif

	/* At user's request, reset environment */
	if (aslogin) {
		extern char **environ;
		char *ndir, *nterm, *nuser;
		size_t len;

		/* reset environment variables */
		nterm = ((nterm = getenv("TERM")) && *nterm) ? nterm : "unknown";
		nuser = (pwd && pwd->pw_name && *pwd->pw_name) ? pwd->pw_name :
			(((nuser = getenv("USER")) && *nuser) ? nuser : "nobody");
		ndir = (pwd && pwd->pw_dir && *pwd->pw_dir) ? pwd->pw_dir : "/";
		newenv[0] = NULL;
		environ = newenv;
		setenv("TERM", nterm, 1);
		setenv("USER", nuser, 1);
		setenv("LOGNAME", nuser, 1);
		setenv("HOME", ndir, 1);
		setenv("SHELL", shell, 1);
		{
			char *newpath;
			int   newplen;
			newpath = alloca((newplen = confstr(_CS_PATH, NULL, 0)) + 1);
			if (newpath && confstr(_CS_PATH, newpath, newplen + 1) > 0) {
				setenv("PATH", newpath, 1);
			}
		}

		tbuf[0] = '-';
		if ((len = strlen(token[0])) > (sizeof tbuf -2)) {
			fprintf(stderr, "%s: You have no shell\n", PROGNAME);
			exit(1);
		}
		memcpy(tbuf + 1, token[0], len+1);
		token[0] = tbuf;
	}

	/* Start the new shell. */
	execvp(shell, token);

	/* could not exec a new shell */
	fprintf(stderr, "%s: You have no shell\n", PROGNAME);
	exit(1);
}
