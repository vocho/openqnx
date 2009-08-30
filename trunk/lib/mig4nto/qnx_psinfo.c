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
 * qnx_psinfo.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <devctl.h>
#include <errno.h>
#include <sys/dcmd_chr.h>
#include <termios.h>
#include <libgen.h>
#include <sys/procfs.h>
#include <time.h>
#include <sched.h>
#include <mig4nto.h>

#define PROC_BASE "/proc"				/* Location of the proc filesystem */

static pid_t next_proc_num(pid_t pnum);

struct dinfo_s {
	procfs_debuginfo    info;
	char                pathbuffer[PATH_MAX];
};

/* 
 *  Some of the information that was associated with the process
 *  in QNX 4 is associated with the thread in QNX Neutrino
 *  (e.g. state, blocked_on, ...).  The assumption here is that
 *  if you are migrating a QNX 4 process to QNX Neutrino, you
 *  will only have one thread, so this information is taken from
 *  the thread with thread id 1 (the main() thread).
 *
 *  The following fields are populated in the migration version of
 *	qnx_psinfo():
 *
 *      psdata->pid             The process id
 *      psdata->blocked_on      What process is blocked on (pid).  
 *      psdata->pid_group       Process group
 *      psdata->ruid            Real user ID
 *      psdata->rgid            Real group ID
 *      psdata->euid            Effective User ID
 *      psdata->egid            Effective Group ID
 *      psdata->umask           Process umask
 *      psdata->sid             Session ID
 *		psdata->signal_ignore	Process signal ignore mask
 *      psdata->signal_mask     Thread signal block mask
 *      psdata->state           Thread state
 *      psdata->priority        Process priority
 *      psdata->max_priority    Max Priority
 *      psdata->sched_algorithm 
 *
 *      psdata->un.proc.name    The name of the program image.
 *                              This name is limited to 100 bytes 
 *                              including the NULL.
 *      psdata->un.proc.father  Parent Process
 *      psdata->un.proc.son     Child Process
 *      psdata->un.proc.brother Sibling process
 *      psdata->un.proc.times   All times set to zero
 *  
 *  All other psdata structure elements are set to MIG4NTO_UNSUPP.
 *
 *  Notes:
 *      Neutrino 2.0 doesn't support time-accounting information, 
 *      so the members of the tms structures are always set to 0.
 *
 *      qnx_psinfo can currently examine processes only, as there are
 *      no /proc entries for virtual circuits and pulses.
 *
 *      The segdata parameter is ignored.
 *
 *      Currently proc_pid can only be 0 or PROC_PID.
 */
pid_t
qnx_psinfo(pid_t proc_pid, pid_t pid, struct _psinfo *psdata,
			unsigned segindex, struct _seginfo *segdata)
{
	struct dinfo_s	dinfo;
	procfs_status	dthread;			/* Thread info struct  */
	procfs_info		dprocess;			/* Process info struct */
	pid_t   		my_pid = pid;
	int     		count;                  
	int     		proc_fd;
	int     		prio;
	int     		max_prio = -1;
	char    		*ptr;
	char    		buf[_POSIX_PATH_MAX+1];

	if (proc_pid != 0 && proc_pid != PROC_PID) {
		errno = ENOSYS;
		return -1;
	}
	if (pid == 0)
		my_pid = getpid();
	memset(psdata, 0, sizeof(struct _psinfo));
	sprintf(buf, "%s/%d", PROC_BASE, my_pid);
	if ((proc_fd = open(buf, O_RDONLY)) == -1) {
		/* search for next available pid.   */
		my_pid = next_proc_num(my_pid);
		sprintf(buf, "%s/%d/as", PROC_BASE, my_pid);
		if ((proc_fd = open(buf, O_RDONLY)) == -1) {
			errno = EINVAL;	/* No process with a process id >= pid exists */
			return -1;
		}
	}

	if (devctl(proc_fd, DCMD_PROC_INFO, &dprocess, sizeof(dprocess), NULL) == -1) {
		close(proc_fd);
		return -1;
	}
	
	if (devctl(proc_fd, DCMD_PROC_MAPDEBUG_BASE, &dinfo, sizeof(dinfo), NULL) == -1) {
		close(proc_fd);
		return -1;
	}

	dthread.tid = 1;
	if (devctl(proc_fd, DCMD_PROC_TIDSTATUS, &dthread, sizeof(dthread), NULL) == -1) {
		close(proc_fd);
		return -1;
	}

	psdata->pid = dprocess.pid;
	switch (dthread.state) {
	case STATE_SEND:
	case STATE_REPLY:
		psdata->blocked_on = dthread.blocked.connect.pid;
		break;
	case STATE_RECEIVE:
		psdata->blocked_on = dthread.blocked.channel.chid;
		break;
	case STATE_SEM:
		psdata->blocked_on = dthread.blocked.sync.id;
		break;
	default:
		psdata->blocked_on = 0;
		break;
	}
	psdata->pid_group   = dprocess.pgrp;
	psdata->ruid        = dprocess.uid;	/* map ruid to uid */
	psdata->rgid        = dprocess.gid;	/* getpgid(my_pid); */
	psdata->euid        = dprocess.euid;
	psdata->egid        = dprocess.egid; 
	psdata->flags       = MIG4NTO_UNSUPP;

	/* Stack Pointer won't fit 64 -> 32     */
	psdata->sp_reg      = MIG4NTO_UNSUPP; 
	/* May or may not have segments.        */
	psdata->ss_reg      = MIG4NTO_UNSUPP;
	psdata->magic_off   = MIG4NTO_UNSUPP; 
	psdata->magic_sel   = MIG4NTO_UNSUPP;
	psdata->ldt         = MIG4NTO_UNSUPP;
	psdata->umask       = dprocess.umask;
	psdata->signal_ignore   = dprocess.sig_ignore.__bits[0]; 
	/* signals can be pending either on the process or on the thread
	   so no way to have a QNX 4 ounterparts */
	psdata->signal_pending  = MIG4NTO_UNSUPP; 

	psdata->signal_mask     = dthread.sig_blocked.__bits[0];
	psdata->signal_off      = MIG4NTO_UNSUPP; /* Signal offset.   */
	psdata->signal_sel      = MIG4NTO_UNSUPP; /* Signal selector. */
	psdata->sid             = dprocess.sid;
	psdata->sid_nid         = MIG4NTO_UNSUPP;
	psdata->state           = dthread.state; 
	psdata->priority        = dthread.priority;
	psdata->sched_algorithm = dthread.policy;
	for (count = SCHED_FIFO; count <= SCHED_OTHER; count++) {
		prio = sched_get_priority_max(count);
		if (max_prio < prio)
			max_prio = prio;
	}
	psdata->max_priority    = max_prio;     /* Max Priority     */

	ptr = basename(dinfo.info.path);
	memcpy(psdata->un.proc.name, ptr, sizeof(psdata->un.proc.name) );
	psdata->un.proc.name[sizeof(psdata->un.proc.name) - 1] = '\0';
	psdata->un.proc.father  	= dprocess.parent;
	psdata->un.proc.son     	= dprocess.child;
	psdata->un.proc.brother 	= dprocess.sibling;
	psdata->un.proc.debugger	= MIG4NTO_UNSUPP; /* Debugger Process */
	/* access rights from qnx_segment_arm() */
	psdata->un.proc.mpass_pid 	= MIG4NTO_UNSUPP;
	psdata->un.proc.mpass_pid_zero = MIG4NTO_UNSUPP;
	psdata->un.proc.mpass_sel 	= MIG4NTO_UNSUPP;
	psdata->un.proc.mpass_flags = MIG4NTO_UNSUPP;
	psdata->un.proc.links   	= MIG4NTO_UNSUPP; /* Num Links */
	psdata->un.proc.file_time 	= MIG4NTO_UNSUPP;
	/* num selectors in this process    */
	psdata->un.proc.nselectors 	= MIG4NTO_UNSUPP; 
	/* at what time the program started */
	psdata->un.proc.start_time 	= MIG4NTO_UNSUPP;
	psdata->un.proc.mxcount  	= MIG4NTO_UNSUPP;
	times(&psdata->un.proc.times);

	return dprocess.pid;
}

static pid_t
next_proc_num(pid_t pnum)
{
	DIR				*dirp;
	struct dirent	*dptr;
	pid_t			dnum = -1;
	int				rval = -1;

	if (pnum <= 0)
			return -1;
	if ((dirp = opendir(PROC_BASE)) != NULL) {
		do {
			if ((dptr = readdir(dirp)) != NULL) {
				dnum = atoi(dptr->d_name);
				/* looking for closest one that is greater than pnum */
				if (dnum > pnum && (rval == -1 || (dnum - pnum < rval - pnum)))
					rval = dnum;
			}
		} while (dptr);
	}
	closedir(dirp);
	return rval;
}
