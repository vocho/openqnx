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





#if !defined PS_H
#define PS_H

#include <time.h>
#include <unistd.h>
#include <signal.h>

struct _ps
{
	uid_t		ruser,
			user;
	gid_t		rgroup,
			group;
	pid_t		pid,
			ppid,
			pgid;
	int		pcpu,	
			vsz,
			sz,
			nice;
	time_t		etime,
			time,
			stime;
	int		tty;
	char 		*comm,
			*args,
			*env;
	int		pflags,
			pri,
			sid,
			suid,
			sgid,
			umask;
	sigset_t	sigign,
			sigpend,
			tsigpend,
			sigqueue,
			tsigblk;
	int		threads,
			tid,
			tflags,
			cpu,
			dflags;
};

extern const char *delimiters;

#endif
