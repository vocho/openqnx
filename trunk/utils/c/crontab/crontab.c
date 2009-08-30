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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <pwd.h>
#include <process.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef __WATCOMC__

#include <unix.h>
#include <sys/name.h>
#include <sys/kernel.h>

static char *cronsrv = "/qnx/cron";
#else

#include <sys/mman.h>
#include <sys/neutrino.h>
#include <fcntl.h>
#include <libgen.h>

#define SHM_CRON "/CRON" /* Must match with cron.c's define */
#endif

enum { CREATE, EDIT, LIST, REMOVE };

#ifdef __QNXNTO__
static char *crondir = "/var/spool/cron";
#else
#error poo
static char *crondir = "/usr/spool/cron";
#endif
static char *argv0;

static void fatal_message(char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "%s: ", argv0);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
    fflush(stderr);

    exit(EXIT_FAILURE); 
}

/*
 * Only allow users to create/replace their crontab if cron.allow
 * exists and their name is listed, or ...
 *
 * Don't allow users to create/replace their crontab if their
 * name appears in cron.deny, or if cron.deny does not exist and
 * their name does not appear in cron.allow
 */
static int listed(char *file, char *user) {
    char line[51]; /* 1 + the scan code value below */
    FILE * fp;

    if ((fp = fopen(file, "r")) == 0)
	return -1;  /* file does not exist */

    while (fscanf(fp, "%50s", line) != EOF) {
	if (strcmp(user, line) == 0) {
	    fclose(fp);
	    return 1;
	}
    }
    fclose(fp);
    return 0;
}

/* determine if crontab for user will be run */
static int not_authorized(char *user) {
    switch (listed("cron.allow", user)) {
      case -1:	/* no cron.allow, authorize if not denied */
	return listed("cron.deny", user) == 1;
      case  0:	/* not found, authorize if not denied */
	return listed("cron.deny", user) == 1;
      case  1:	/* allowed, authorize */
      return 0;
    }
    return 1;
}

int main(int argc, char **argv) {
    char crontab[11 + LOGIN_NAME_MAX] = "crontabs/";/* partial pathname */
    struct passwd *pwent;			/* passwd struct */
    int ch;					/* option character */
    FILE *tab, *tmp = NULL;			/* output/input files */
    int action = CREATE;

#ifdef __WATCOMC__
    nid_t nid = 0, qnx_strtonid();	/* server node */
    pid_t server;
    int local = 0;

    argv0 = basename(argv[0]);
    pwent = getpwuid(getuid());
    umask(066);

    while ((ch = getopt(argc, argv, "d:en:lLru:")) != EOF) {
	switch (ch) {
	  case 'L':
	    nid = getnid();
	    local = !local;
	    break;
	  case 'n':
	    nid = qnx_strtonid(optarg);
	    local = !local;
	    break;
#else
    void * server = NULL;
    int fd;					/* shm_open file handle */
    int euid, egid;

    argv0 = basename(argv[0]);
    pwent = getpwuid(getuid());
    umask(066);

    while ((ch = getopt(argc, argv, "d:elru:")) != EOF) {
	switch (ch) {
#endif
	  case 'd':
	    crondir = optarg;
	    break;
	  case 'e':
	    action = EDIT;
	    break;
	  case 'l':
	    if (action == REMOVE)
		fatal_message("conflict: -r/-l");
	    action = LIST;
	    break;
	  case 'r':
	    if (action == LIST) {
		fatal_message("conflict: -r/-l");
	    }
	    action = REMOVE;
	    break;
	  case 'u':
	    /* get name and uid/gid for user */
	    if ((pwent = getpwnam(optarg)) == 0) {
		long id = strtol(optarg, (char **) &pwent, 10);

		if (*(char *) pwent != 0) {
		    fatal_message("%s %s", "invalid user", optarg);
		}
		if ((pwent = getpwuid(id)) == 0) {
		    fatal_message("%s %s", "unknown user", optarg);
		}
	    }
	    break;
	  default:
	    exit(EXIT_FAILURE);
	}
    }

    /* make sure we're allowed to do anything with this crontab */
    if (getuid() != pwent->pw_uid && getuid() != 0) {
	fatal_message("access denied to crontab for %s", pwent->pw_name);
    }

    /* Locate the server */
#ifdef __WATCOMC__ 
    switch (nid) {
      case 0:
	if ((server = qnx_name_locate(0, cronsrv + 1, 0, 0)) != -1) {
	    nid = getnid();
	    local++;
	    break;
	}
	/* fallthru */
      default:
	server = qnx_name_locate(nid, cronsrv + local, 0, 0);
    }

    if (server == -1)
	fprintf(stderr, "warning: cron has not been started\n");

    if (local) {
	crondir = malloc(strlen(optarg = crondir) + 8);
	sprintf(crondir, "%s.%ld", optarg, nid);
    }

#else
    /* check if cron server is running. */
    if ((fd = shm_open(SHM_CRON, O_RDONLY, S_IRWXU )) == -1) {
	fprintf(stderr, "warning: cron has not been started\n");
    }
    else if(MAP_FAILED == (server = mmap(0, sizeof(pid_t), PROT_READ, MAP_SHARED, fd, 0))) {
	fatal_message("mmapp: %s", strerror(errno));
    }

    if(server && kill(*(pid_t *)server, 0) == -1) {
	fatal_message("cron stopped abnormally");
    }
    close(fd);
#endif

    strcat(crontab, pwent->pw_name);

    if (optind == argc) {
	if (action == CREATE || action == EDIT) {
	    char work[] = "/tmp/cronXXXX";

	    mktemp(work);

	    if (action == CREATE) {
		/* read replacement crontab from stdin */
		tmp = fopen(work, "w+");
		while ((ch = getchar()) != EOF)
		    fputc(ch, tmp);
	    } else {
		char *editor = getenv("EDITOR");
		int status;

		if (editor == NULL)
		    editor = "vi";

		if (chdir(crondir) == -1)
		    fatal_message("%s: %s", crondir, strerror(errno));

		if ((tab = fopen(crontab, "r")) == 0 && errno != ENOENT) {
		    fatal_message("%s/%s: %s", crondir, pwent->pw_name, strerror(errno));
		}

		egid = getegid();
		euid = geteuid();
		setegid(getgid());
		seteuid(getuid());

		tmp = fopen(work, "w+");
		if (tab) {
		    while ((ch = fgetc(tab)) != EOF)
			fputc(ch, tmp);
		    fflush(tmp);
		    fclose(tab);
		}
		if ((status = spawnlp(P_WAIT, editor, editor, work, 0))) {
		    remove(work);
		    fatal_message(status == -1 ? "unable to start %s" : "%s exited %d", editor, status);
		}
		setegid(egid);
		seteuid(euid);
	    }
	    remove(work);
	    rewind(tmp);
	}
    } else if (optind == argc - 1 && action == CREATE) {
	/* copy file onto crontab */
	if (access(argv[optind], R_OK) != 0 || (tmp = fopen(argv[optind], "r")) == 0) {
	    fatal_message("%s: %s", argv[optind], strerror(errno));
	}
    } else {
	fatal_message("too many arguments");
    }

    if(chdir(crondir) != 0)
	fatal_message("crontab %s: %s", strerror(errno), crondir);

    if (not_authorized(pwent->pw_name) && pwent->pw_uid != 0) {
	fprintf(stderr, "warning: no permission for cron: %s\n", pwent->pw_name);
    }

    if (action == LIST) {
	if ((tab = fopen(crontab, "r")) == 0) {
	    if(errno == ENOENT) 
		fatal_message("no crontab for %s", pwent->pw_name);
	    fatal_message("%s: %s", pwent->pw_name, strerror(errno));
	} else {
	    while ((ch = fgetc(tab)) != EOF)
		putchar(ch);
	    fclose(tab);
	}
    } else {
	/* drop and store euid/egid, use uid/gid */ 
	egid = getegid();
	euid = geteuid();
	setegid(getgid());
	seteuid(getuid());

	chmod(crontab, S_IRWXU);	/* -rw---- */
	if (action == REMOVE) {
	    if (remove(crontab) == -1 && errno != ENOENT) {
		fatal_message("%s: %s", strerror(errno), crontab);
	    }
	} else if (action == CREATE || action == EDIT) {
	    if ((tab = fopen(crontab, "w")) == NULL) {
		fatal_message("%s: %s", strerror(errno), crontab);
	    }

	    while ((ch = fgetc(tmp)) != EOF)
		fputc(ch, tab);

	    fflush(tab); /* Changing write permissions later! */

	    fchown(fileno(tab), pwent->pw_uid, pwent->pw_gid);
	    fchmod(fileno(tab), S_IRUSR);	/* -r----- */
	    fclose(tab);
	    fclose(tmp);

	}
	/* restore euid/egid for kill */
	setegid(egid);
	seteuid(euid);

	/* Notify server */
#ifdef __WATCOMC__
	if (server != -1 && Send(server, pwent->pw_name, 0, strlen(pwent->pw_name) + 1, 0) < 0) {
#else
	if(server && kill(*(pid_t *)server, SIGUSR1) == -1) {
#endif
		fatal_message("inform cron about update: %s", strerror(errno));
	}
    }
    return EXIT_SUCCESS;
}
