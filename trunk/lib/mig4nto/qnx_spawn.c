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

/*
 * qnx_spawn.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <spawn.h>
#include <mig4nto.h>

#define FD_MAX  10	/* Max for QNX Neutrino is OPEN_MAX, and 10 for QNX 4 */

/*
 *  __msgbuf        Under QNX 4 this was the spawn message buffer sent to
 *	                the process manager.  In mig4nto this is ignored.
 *
 *  __sched_algo    QNX4 and NTO use the same names for scheduling
 *                  algorithms, but their values are different.  Be
 *					very careful if you are not just recompiling with the
 *					macros from the QNX Neutrino header files.
 *					
 *					Note also that in QNX Neutrino SCHED_OTHER is
 *					SCHED_RR.  QNX Neutrino does not have QNX 4's
 *					adaptive scheduling algorithm.  As such, there
 *					is no equivalent of SCHED_FAIR.
 *
 *  __flags         NOTE: the following flags are unsupported in mig4nto:
 *             		_SPAWN_XCACHE
 *                  
 *  __iov       	If this is given then unlike QNX 4, the fds passed
 *					within it will be the only ones inherited by the
 *					child.  This is true even for the iovs that are -1.
 *            
 *  __ctfd      	The mig4nto implementation returns -1 and sets
 *					errno to EINVAL if the __ctfd parameter is anything
 *					other than -1.
 */
pid_t
qnx_spawn(int __exec, struct _proc_spawn *__msgbuf, nid_t __node,
		  int __prio, int __sched_algo, int __flags,
		  const char *__path, char **__argv, char **__envp,
		  char *__iov, int __ctfd)
{
	struct inheritance inherit;
	int     fdcount = 0;
	int     fdmap[FD_MAX];
	long    argmaxlen = 0;
	int     count;
	char    **arg;

	memset(&inherit, 0, sizeof(inherit));

	arg = __argv;
	for (count = 0; argmaxlen < ARG_MAX && arg && arg[count]; count++)
		argmaxlen += strlen(arg[count]);
	arg = __envp;
	for (count = 0; argmaxlen < ARG_MAX && arg && arg[count]; count++)
		argmaxlen += strlen(arg[count]);
	if (argmaxlen > ARG_MAX) {
		errno = E2BIG;
		return -1;
	}

	if (strlen(__path) > PATH_MAX) {
		errno = ENAMETOOLONG;
		return -1;
	}

	if (__ctfd != -1) {
		errno = EINVAL;
		return -1;
	}

	if (__iov)					/* Count and map number of mapped fd's  */
		for (fdcount = 0; fdcount < FD_MAX; fdcount++)
	    	fdmap[fdcount] = __iov[fdcount];
	
	if (__prio != -1 || __sched_algo != -1) {
		inherit.flags |= SPAWN_EXPLICIT_SCHED;  
		if (__sched_algo == -1)
			__sched_algo = SCHED_NOCHANGE;
		inherit.policy = __sched_algo;
		if (__prio == -1)
			sched_getparam(0, &inherit.param);
		else
			inherit.param.sched_priority = __prio;
	}
	if (__exec != 0)					/* Cause the spawn to act like exec() */
		inherit.flags |= SPAWN_EXEC;
	if (__node != 0) {					/* spawn to remote node */
		inherit.flags |= SPAWN_SETND;
		inherit.nd = __node;
	}
	if (__flags != 0) {
		/* 
		 * with SPAWN_SETSIGIGN we are not giving a new ignore mask,
		 * we are just telling it which ones to ignore in addition to
		 * any that may already be ignored
		 */
		if (__flags & _SPAWN_SIGCLR) {	/* don't inherit parents ignore mask */
			inherit.flags |= SPAWN_SETSIGDEF;
			sigfillset(&inherit.sigdefault);
		}
		if (__flags & (_SPAWN_BGROUND | _SPAWN_NOHUP)) {
			inherit.flags |= SPAWN_SETSIGIGN;
			sigemptyset(&inherit.sigignore);
			if (__flags & _SPAWN_BGROUND) {
	            sigaddset(&inherit.sigignore, SIGINT);
	            sigaddset(&inherit.sigignore, SIGQUIT);
			}
			if (__flags & _SPAWN_NOHUP)
				sigaddset( &inherit.sigignore, SIGHUP);
		}
		if (__flags & _SPAWN_HOLD)		/* start process in stopped state */
			inherit.flags |= SPAWN_HOLD;
		if (__flags & _SPAWN_NEWPGRP) {	/* new process starts a new group. */
			inherit.flags |= SPAWN_SETGROUP;
			inherit.pgroup = SPAWN_NEWPGROUP;
		}
		if (__flags & _SPAWN_NOZOMBIE)	/* Process will not zombie */
			inherit.flags |= SPAWN_NOZOMBIE;
		if (__flags & _SPAWN_SETSID)	/* process starts a new session. */
			inherit.flags |= SPAWN_SETSID;
		if (_SPAWN_TCSETPGRP & __flags)	/* start a new terminal group. */
			inherit.flags |= SPAWN_TCSETPGROUP;
	}                    
	return spawn(__path, fdcount, fdmap, &inherit, __argv, __envp);
}
