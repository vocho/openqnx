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




#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <sys/procmsg.h>

#define STD_FILENO_MAX		(max(max(STDIN_FILENO, STDOUT_FILENO), STDERR_FILENO))

int procmgr_daemon(int status, unsigned flags) {
	proc_daemon_t			msg;
	int						nfds;

	msg.i.type = _PROC_DAEMON;
	msg.i.subtype = 0;
	msg.i.status = status;	
	msg.i.flags = flags;

	if((nfds = MsgSendnc(PROCMGR_COID, &msg.i, sizeof msg.i, 0, 0)) == -1) {
		return -1;
	}
	if(!(flags & PROCMGR_DAEMON_NOCLOSE)) {
		int						fd;

		for(fd = STD_FILENO_MAX + 1; fd < nfds; fd++) {
			close(fd);
		}
	}
	if(!(flags & PROCMGR_DAEMON_NODEVNULL)) {
		int						fd;

		if((fd = open("/dev/null", O_RDWR | O_NOCTTY)) != -1) {
			dup2(fd, STDIN_FILENO);
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			if(fd > STD_FILENO_MAX) {
				close(fd);
			}
		}
	}
	return nfds;
}

__SRCVERSION("procmgr_daemon.c $Rev: 153052 $");
