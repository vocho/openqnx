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
 *  popen.c
 *
 */
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <spawn.h>
#include <libgen.h>
#include <pthread.h>
#include <paths.h>
#include <sys/wait.h>

#ifndef PATH_MAX
#define PATH_MAX	1024 	/* Should use fpathconf(path, _PC_PATH_MAX) */
#endif

static struct popen_entry {
	struct popen_entry			*next;
	FILE						*fp;
	pid_t						pid;
}							*popen_list;
static pthread_mutex_t		popen_mutex = PTHREAD_MUTEX_INITIALIZER;

FILE *popen(const char *command, const char *mode) {
	int						fds[2];
	int						fdmap[3];
	int						fpindex;
	struct popen_entry		*p;
	char					*argv[4];
	int						status;

	if(mode[0] != 'r' && mode[0] != 'w') {
		errno = EINVAL;
		return 0;
	}

	if(!(p = malloc(sizeof *p))) {
		errno = ENOMEM;
		return 0;
	}

	if(pipe(fds) == -1) {
		free(p);
		return 0;
	}

	if((status = pthread_mutex_lock(&popen_mutex)) != EOK) {
		free(p);
		close(fds[0]);
		close(fds[1]);
		errno = status;
		return 0;
	}
	p->pid = -1;
	p->next = popen_list;
	popen_list = p;
	pthread_mutex_unlock(&popen_mutex);

	if(mode[0] == 'r') {
		fdmap[STDIN_FILENO] = STDIN_FILENO;
		fdmap[STDOUT_FILENO] = fds[1];
		fpindex = 0;
	} else {
		fdmap[STDIN_FILENO] = fds[0];
		fdmap[STDOUT_FILENO] = STDOUT_FILENO;
		fpindex = 1;
	}
	fdmap[STDERR_FILENO] = STDERR_FILENO;

	if(!(p->fp = fdopen(fds[fpindex], mode))) {
		status = errno;
		close(fds[fpindex]);
	} else {
		char					path[PATH_MAX + 1];
		char 					buff[PATH_MAX + 1];
		char					*sh;

		(void)fwide(p->fp, -1);

		status = errno;
		if(confstr(_CS_PATH, path, sizeof path) == 0 || !(sh = pathfind_r(path, "sh", "x", buff, sizeof buff))) {
			sh = _PATH_BSHELL;
		}
		errno = status;
	    argv[0] = "sh";
		argv[1] = "-c";
		argv[2] = (char *)command;
		argv[3] = 0;

		if((p->pid = spawn(sh, 3, fdmap, 0, argv, 0)) == -1) {
			status = errno;
		}
	}
	close(fds[1 - fpindex]);

	if(p->pid == -1) {
		struct popen_entry		*p1, **pp;

		pthread_mutex_lock(&popen_mutex);
		for(pp = &popen_list; (p1 = *pp); pp = &p1->next) {
			if(p1 == p) {
				*pp = p1->next;
				break;
			}
		}
		pthread_mutex_unlock(&popen_mutex);

		if(p->fp) {
			fclose(p->fp);
		}
		free(p);
		errno = status;
		return 0;
	}

	return p->fp;
}

int pclose(FILE *fp) {
	int						status;
	struct popen_entry		*p, **pp;
	pid_t					pid;

	pthread_mutex_lock(&popen_mutex);
	for(pp = &popen_list; (p = *pp); pp = &p->next) {
		if(p->pid != -1 && p->fp == fp) {
			*pp = p->next;
			break;
		}
	}
	pthread_mutex_unlock(&popen_mutex);
	if(!p) {
		errno = EINVAL;
		return -1;
	}
	pid = p->pid;
	fclose(p->fp);
	free(p);

	if(waitpid(pid, &status, 0) == -1) {
		return -1;
	}
	return status;
}

__SRCVERSION("popen.c $Rev: 153052 $");
