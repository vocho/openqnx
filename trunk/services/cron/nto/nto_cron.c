/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <syslog.h>
#include "cron.h"

#define SHM_CRON		"/CRON" /* Must match with crontab.c's */

/* These are the OS dependant routines: */

/* int main(int *, char **) */
void cleanup(int signo);
void trigger(cron_job * job);

/*  Extra parameters to the message() function */
extern int is_logging;
extern char * argv0;

/*  Location of crontabs */
char * crondir = "/var/spool/cron";

void cleanup(int signo) 
{
	shm_unlink(SHM_CRON);
	message(EVENT, "shutdown");
	exit(!signo); /* Called with a 0 when an error has occured */
}

/* The event described in the cron_job is triggered */
/* Note:  this forks once for each event, so
 *        the forks better not take more than
 *        one minute or we may miss the next event
 */
#define STDERR2 4 /* fcloseall() is called in main, so 3 should be lowest */

void trigger(cron_job * job) {
	char * shell = "SHELL=/bin/sh"; 
	pid_t pid = pid;
	char *inputstr;
	char command[TAB_BUFFER_SIZE];
	int attempt, result;
	FILE *output;
	struct passwd *pwent;

	strcpy(command, job->command);
	if (inputstr = strchr(command, '\n'))
		*inputstr++ = '\0';

	/* Disable jobs belonging to deleted users */
	if((pwent = getpwuid(job->user)) == NULL) {
		return; /* Will be reported at next ctab reload */
	}
	if(fork() != 0) {
		message(LOG, "command %s: ", command);
		return;
	}

	setgid(pwent->pw_gid);
	setuid(pwent->pw_uid);

	chdir(pwent->pw_dir);
	setenv("LOGNAME", pwent->pw_name, 1);
	setenv("HOME", pwent->pw_dir, 1);

	putenv(shell);
	putenv("PATH=/bin:/usr/bin");

	umask(022);
	signal(SIGCHLD, SIG_DFL);

	/* provide specified stdin for process */
	close(STDIN_FILENO);
	if (inputstr) {
		FILE *input;

		*inputstr++ = '\0';
		if ((input = tmpfile()) == 0) { /* input is implicitly stdin */
			message(FATAL_FORKED, "%s: %s: %s", strerror(errno), "tmpfile", command);
		}
		fputs(inputstr, input);
		fputc('\n', input);
		rewind(input);
	} else {
		if(fopen("/dev/null", "r") == NULL)	
			message(FATAL_FORKED, "%s: %s: %s", strerror(errno), "/dev/null", command);
	}

	/* Trap output to a tempfile, and then log it with a pid prefix */
	if ((output = tmpfile()) == NULL)
		message(FATAL_FORKED, "%s: %s: %s", "tmpfile", strerror(errno), command);

	/* Make a copy of stderr for reporting to the log file */
	dup2(STDERR_FILENO, STDERR2);
	fcntl(STDERR2, F_SETFD, fcntl(STDERR2, F_GETFD) | FD_CLOEXEC);

	dup2(fileno(output), STDOUT_FILENO);
	dup2(fileno(output), STDERR_FILENO);
	fcntl(fileno(output), F_SETFD, fcntl(fileno(output), F_GETFD) | FD_CLOEXEC);

	for(attempt = 5; 
	  attempt > 0 && (pid = spawnl(P_NOWAIT, shell+6, shell+11, "-c", command, NULL)) == -1;
	  --attempt) {
		sleep(10);
	}

	dup2(STDERR2, STDERR_FILENO); /* Reset stderr to be the logfile */

	if(attempt) {
		message(LOG, "> pid %d: %s %s", pid, pwent->pw_name, command);
		waitpid(pid, &result, 0);

		/* MAIL SUPPORT: would mail output instead */ 
		rewind(output);
		while(fgets(command, IO_WIDTH, output)) {
			if(inputstr = strchr(command, '\n')) /* Aprox 75 char line wrap */
				*inputstr = '\0';
			message(attempt ? LOG : EVENT, "  pid %d: %s", pid, command);
		}

		message(LOG, "< pid %d: %s", pid, pwent->pw_name);
	} else
		message(EVENT, "couldn't spawn shell for 1 min: %s", strerror(errno));

	exit(0); /* Status ignored */
}
#undef STDERR2

extern int main(int argc, char **argv) {
    char * tz = getenv("TZ");
    int ch;                   /* getopt return */
    int is_already_shmem = 0; /* flag */
    void * advertisement;     /* shm pointer */
    sigset_t updatesig;
    long int next_time;
    int fd;

	if(0 != geteuid()){
		fprintf(stderr, "%s: must be run as root\n", basename(argv[0]));
		exit(1);
	}

    clearenv();
    fcloseall();

	openlog("cron", LOG_CONS | LOG_PERROR, LOG_CRON);

    if(tz)
	setenv("TZ", tz, 1);
    argv0 = basename(argv[0]);

    while ((ch = getopt(argc, argv, "d:v")) != EOF) {
	switch (ch) {
	  case 'd': /* crondir */
	    crondir = optarg;
	    break;

	  case 'v': /* verbose */
	    is_logging = 1;
	    break;

	  default:
	    exit(-1); /* getopt issues an error message */
	}
    }

    message(EVENT, "started");

//@@@    if (getuid() != 0)
//	message(FATAL, "requires root to run");

    if(chdir(crondir) != 0)
	message(FATAL, "%s: %s", strerror(errno), crondir);

    /* check if cron server already running and set up shared memory 
     * area that advertises our pid.
     */
    if ((fd = shm_open(SHM_CRON, O_RDWR|O_CREAT|O_EXCL, S_IRWXU)) == -1) {
	is_already_shmem = 1;
	if ((fd = shm_open(SHM_CRON, O_RDWR, S_IRWXU)) == -1) {
		message(FATAL_FORKED, "shmem_open(" SHM_CRON "): %s", strerror(errno));
	}
    }
    /* set size of memory area */
    if (ftruncate(fd, sizeof(pid_t)) != -1 &&
      MAP_FAILED != (advertisement = mmap(0, sizeof(pid_t), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0))) {
	if(is_already_shmem && (kill(*(pid_t *)advertisement, 0) != -1 || errno != ESRCH)) { 
		message(FATAL_FORKED, "already running");
	} /* If the kill fails, assume cron died without removing "Cron" */

	/* set pid in shared memory region */
	*(pid_t *)advertisement = getpid();
    } else {
	message(FATAL_FORKED, "mmapp: %s", strerror(errno));
    }
    close(fd);

    /* Ignore hangup in case started with ontty because parent
     * (session leader) forks then exits causing child to be HUP'd
     */
    signal(SIGHUP,  SIG_IGN);	/* survive session leader death */
    signal(SIGCHLD, SIG_IGN);	/* ignore child deaths */
    signal(SIGTERM, cleanup);
    signal(SIGPWR,  cleanup);
    signal(SIGINT,  SIG_IGN);   /* would recive BRK even if '&'d */
    signal(SIGUSR1, ctab_load);

    ctab_load(0);
    sigemptyset(&updatesig);
    sigaddset(&updatesig, SIGUSR1);

    for(;;) {
	sigprocmask(SIG_BLOCK, &updatesig, NULL); /* put off crontab reloads */
	next_time = do_jobs();
	sigprocmask(SIG_UNBLOCK, &updatesig, NULL);

	sleep(next_time);
    }
}

