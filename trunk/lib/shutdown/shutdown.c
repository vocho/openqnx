/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable license fees to QNX
 * Software Systems before you may reproduce, modify or distribute this software,
 * or any work that includes all or part of this software.	 Free development
 * licenses are available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *
 * This file may contain contributions from others.	 Please review this entire
 * file for other proprietary rights or license notices, as well as the QNX
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/
 * for other information.
 * $
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <sys/sysmgr.h>
#include <sys/procfs.h>
#include <sys/procmgr.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <sys/shutdown.h>

static const struct
{
	uint16_t slow_wait; /* time to wait for proc in "slow" mode */
	uint16_t fast_wait; /* time to wait for proc in "fast" mode */
} process_waits[] =
{
	{ 2000, 5 },
	{ 10, 5 },
	{ 10, 5 },
	{ 800, 800 },
	{ 5, 5 }
};


static int pidinfo_cmp(const void *a,const void *b)
{
	ProcessInfo_t *p1 = (ProcessInfo_t*)a,*p2 = (ProcessInfo_t*)b;
	if(p1->class != p2->class)
		return(p1->class > p2->class ? 1 : -1);

	if(p1->start_time != p2->start_time)
		return(p1->start_time > p2->start_time ? 1 : -1);

	return(0);
}


//
// Collect information on a process and add it to the pid vector.
// If we can't get info then we ignore the process.
// We also sort each process into various classes. We rely on a
// naming convention which is not great but the best available without
// adding a new process capability and modifying each process to set
// its class itself.
//
static int getpidinfo(char const *name,ProcessInfo_t *pip)
{
	char buf[PATH_MAX];
	procfs_info pi;
	int fd,r;
	unsigned offset;

	if((fd = open(name,O_RDONLY)) == -1)
		return(-1);

	if(devctl(fd,DCMD_PROC_INFO,&pi,sizeof(pi),0) ||
	   lseek(fd,pi.initial_stack + sizeof(offset),SEEK_SET) == -1 ||
	   read(fd,&offset,sizeof(offset)) != sizeof(offset) ||
	   lseek( fd, offset, SEEK_SET ) == -1 ||
	   (r = read(fd,buf,PATH_MAX)) <= 0 ||
	   !memchr(buf,'\0',r))
	{
		(void) close(fd);
		return(-1);
	}

	pip->name = strdup(basename(buf));
	if (NULL == pip->name) {
		return(-1);
	}
	pip->pid = pi.pid;

	if(!strncmp(pip->name,"phfont",6) ||
	   !strcmp(pip->name, "fontsleuth") ||
	   !strcmp(pip->name,"pwm") ||
	   !strncmp(pip->name, "devg-", 5) ||
	   !strncmp(pip->name, "devi-", 5) ||
	   !strncmp(pip->name, "io-gr", 5) ||
	   !strncmp(pip->name, "io-display", 10) ||
	   // These 2 ignore sigterm so I decided not to slay
	   // them to avoid the 8 second wait.
	   !strncmp(pip->name, "fs-", 3) ||
	   !strncmp(pip->name, "pci-", 4) ||
	   !strncmp(pip->name, "devc-", 5))
		pip->class = CLASS_DISPLAY;
	else if(!stricmp(pip->name,"photon"))
		pip->class = shutdown_classify(pip);
	else if(!strncmp(pip->name,"devb-", 5) ||
			!strncmp(pip->name, "devf-", 5) ||
			!strcmp(pip->name, "pipe"))
		pip->class = CLASS_FSYS;
	else if((pi.flags & _NTO_PF_CHECK_INTR) || pi.sid == 1)
		pip->class = CLASS_DAEMON;
	else
		pip->class = shutdown_classify(pip);

	// Need to kill init first or it will start a login when it
	// sees its children dying!
	if(!strcmp(pip->name, "init") || !strcmp(pip->name, "tinit")) {
		pip->class = CLASS_APP;
		pip->start_time = 0;
	}
	else
		pip->start_time = pi.pid & 0x3fff;

	(void) close(fd);

	if(pip->class == -1)
		free(pip->name), pip->name = NULL;

	return(pip->class);
}


void shutdown_system(int type,int flags)
{
	char buf[PATH_MAX+128];
	DIR *dir = NULL;
	struct dirent *dirent;
	int class,count,i,ndisplay_count = 0,photon_count = 0;
	pid_t me = getpid(),pid;
	ProcessInfo_t * pidvec = NULL, * pip;
	struct sched_param param;
	DisplayData_t ddata;
	size_t pid_count;
	char const * const proc_dir = "/proc";

	if(type == SHUTDOWN_SYSTEM || type == SHUTDOWN_REBOOT)
	{
		if(!(flags & FLAG_DEBUG) && geteuid() != 0)
		{
			shutdown_error("You must be root to shutdown the system.");
			exit(EXIT_FAILURE);
		}
	}

	if (!(flags & FLAG_NO_DAEMON)) {
		/* try to daemonize */
		(void) procmgr_daemon(0, PROCMGR_DAEMON_NODEVNULL | PROCMGR_DAEMON_NOCLOSE);
	}

	/* raise our priority so there is less chance of the world changing
	   under our feet */
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	sched_setscheduler(0,SCHED_FIFO,&param);

	/* build a list of all running processes. Skipping procnto and ourselves */

	if(!(dir = opendir(proc_dir)))
	{
		shutdown_error("Unable to access procfs.");
		exit(EXIT_FAILURE);
	}

	for(pid_count = 0;dirent = readdir(dir);pid_count++);

	if(!(pidvec = calloc(pid_count, sizeof(*pidvec))))
	{
		(void) closedir(dir), dir = NULL;
		shutdown_error("Insufficient memory to build process list.");
		exit(EXIT_FAILURE);
	}

	for(rewinddir(dir),pid_count = 0,pip = pidvec;dirent = readdir(dir);)
	{
		pid = strtoul(dirent->d_name,NULL,0);

		if(pid > 1 && pid != me)
		{
			int res = snprintf(buf, sizeof(buf), "%s/%s/as", proc_dir, dirent->d_name);
			if (-1 == res || res >= sizeof(buf)) {
				(void) closedir(dir), dir = NULL;
				shutdown_error("Failed to build process list.");
				exit(EXIT_FAILURE);
			}

			if((class = getpidinfo(buf,pip)) != -1)
			{
				pid_count++;
				pip++;

				if(class == CLASS_PHOTON_APP)
					photon_count++;
				if(class < CLASS_DISPLAY)
					ndisplay_count++;
			}
		}
	}

	(void) closedir(dir), dir = NULL;

	//
	// We shut down processes by sending them a SIGTERM.
	// We order the processes into classes and kill each class in turn.
	// For each class we kill them in reverse time order based upon their
	// start time (newest first).
	//
	// For the case of a SHUTDOWN_PHOTON we hit Photon with a SIGHUP
	//

	qsort(pidvec,pid_count,sizeof(*pidvec),pidinfo_cmp);

	(void) signal(SIGTERM, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);

	for(class = CLASS_PHOTON_APP - 1,i = 0,pip = pidvec;
		i < ndisplay_count;
		shutdown_progress(++i,(type==SHUTDOWN_PHOTON || type==SHUTDOWN_PHOTON_USER) ? photon_count : ndisplay_count),pip++)
	{
		// After we shutdown all photon apps we target Photon directly.

		if((type == SHUTDOWN_PHOTON || type == SHUTDOWN_PHOTON_USER) && pip->class > CLASS_PHOTON_APP)
		{
			for(;i < pid_count;++i,pip++)
				if(pip->class == CLASS_DISPLAY && !stricmp(pip->name,"photon"))
				{
					if(flags & (FLAG_VERBOSE | FLAG_VERY_VERBOSE))
					{
						ddata.proc_class = class = pip->class;
						shutdown_display(DISPLAY_CLASS,&ddata);
					}

					if(flags & FLAG_VERY_VERBOSE)
					{
						ddata.proc_name = pip->name;
						shutdown_display(DISPLAY_PROC,&ddata);
					}

					shutdown_done(type);

					if(!(flags & FLAG_DEBUG))
						kill(pip->pid, type == SHUTDOWN_PHOTON ? SIGHUP : SIGTERM);

					exit(EXIT_SUCCESS);
				}

			shutdown_error("Unable to locate the Photon server.");
			exit(EXIT_FAILURE);
		}

		if(flags & (FLAG_VERBOSE | FLAG_VERY_VERBOSE))
		{
			if(pip->class != class)
			{
				ddata.proc_class = class = pip->class;
				shutdown_display(DISPLAY_CLASS,&ddata);
			}
		}

		if(flags & FLAG_VERY_VERBOSE)
		{
			ddata.proc_name = pip->name;
			shutdown_display(DISPLAY_PROC,&ddata);
		}

		if((flags & FLAG_DEBUG) && strcmp(pip->name,"noterm"))
			continue;

		(void) kill(pip->pid, SIGTERM);

		do
		{
			count = (flags & FLAG_FAST) ? process_waits[pip->class].fast_wait : process_waits[pip->class].slow_wait;
			for(;count && !kill(pip->pid,0);count--)
			{
				(void) delay(10);
				shutdown_process();
			}

			/* applications may get a SIGKILL if they won't go away */
			if(pip->class <= CLASS_APP && !count)
			{
				if(pip->class == CLASS_APP || (flags & FLAG_FAST))
				{
					(void) kill(pip->pid, SIGKILL);
					count = ~0;
				}
				else
				{
					switch(shutdown_prompt(pip->name,pip->pid))
					{
						case PROMPT_WAIT:
							break;

						case PROMPT_SKIP:
							count = ~0;
							break;

						case PROMPT_CANCEL:
							exit(EXIT_FAILURE);

						default:
							(void) kill(pip->pid, SIGKILL);
							count = ~0;
							break;
					}
				}
			}
		} while (pip->class <= CLASS_APP && !count);
	}

	/* all of the non-display procs have been shut down at this
	   point.  Do a final display update */

	switch(type)
	{
		case SHUTDOWN_SYSTEM:
		case SHUTDOWN_REBOOT:
			shutdown_done(type);
			break;

		default:
			exit(EXIT_SUCCESS);
	}

	if(!(flags & FLAG_DEBUG))
	{
		if(type == SHUTDOWN_SYSTEM)
			/* loop forever */
			for(;;);
		else
		{
			/* shutdown display & reboot */
			for(;i < pid_count;i++,pip++)
				(void) kill(pip->pid, SIGTERM);

			sysmgr_reboot();
		}
	}

	for (i = 0; i < pid_count; ++i) {
		free(pidvec[i].name), pidvec[i].name = NULL;
	}
	free(pidvec), pidvec = NULL;

}
