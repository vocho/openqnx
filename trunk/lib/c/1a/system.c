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




#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <process.h>
#include <spawn.h>
#include <libgen.h>
#include <paths.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef PATH_MAX
#define PATH_MAX	1024 	/* Should use fpathconf(path, _PC_PATH_MAX) */
#endif

int system(const char *cmd) {
	int						pid, status;
	struct sigaction		sa, savintr, savequit;
	char					*argv[4];
	spawn_inheritance_type	inherit;
	char					path[PATH_MAX + 1];
	char 					buff[PATH_MAX + 1];
	char					*sh;

	if(confstr(_CS_PATH, path, sizeof path) == 0 || !(sh = pathfind_r(path, "sh", "x", buff, sizeof buff))) {
		sh = _PATH_BSHELL;

		// If cmd is NULL we do an existance check on the shell.
		if(!cmd) {
			return eaccess(sh, X_OK) != -1;
		}
	}
	// If cmd is NULL return existance of shell.
	if(!cmd) {
		return 1;
	}

	// Setup arguments for spawn.
    argv[0] = "sh";
	argv[1] = "-c";
	argv[2] = (char *)cmd;
	argv[3] = NULL;


	// Ignore SIGINT,SIGQUIT and mask SIGCHLD on parent.
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, &savintr);
	sigaction(SIGQUIT, &sa, &savequit);
	sigaddset(&sa.sa_mask, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sa.sa_mask, &inherit.sigmask);

	// Inialize inheritance structure for spawn.
	sigfillset(&inherit.sigdefault);
	inherit.flags = SPAWN_SETSIGDEF | SPAWN_SETSIGMASK;

	// POSIX 1003.1d implementation.
	if((pid = spawn(sh, 0, NULL, &inherit, argv, environ)) == -1) {
		status = -1;
	} else while(waitpid(pid, &status, 0) == -1) {
		if(errno != EINTR) {
			status = -1;
			break;
		}
	}

	// restore SIGINT, SIGQUIT, SIGCHLD.
	sigaction(SIGINT, &savintr, NULL);
	sigaction(SIGQUIT, &savequit, NULL);
	sigprocmask(SIG_SETMASK, &inherit.sigmask, NULL);

	return(status);
}


#ifdef TEST

main(int argc, char**argv) 		
{
	if (argc>1) 
		printf("system returned %d\n",system(argv[1]));
	else printf("Must supply a cmd line parm\n");
}

#endif

__SRCVERSION("system.c $Rev: 153052 $");
