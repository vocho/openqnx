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
#include <sys/stat.h>
#include <errno.h>
#include <sys/procmsg.h>

mode_t _umask(pid_t pid, mode_t cmask) {
	proc_umask_t				msg;

	msg.i.type = _PROC_UMASK;
	msg.i.subtype = _PROC_UMASK_SET;
	msg.i.umask = cmask;
	msg.i.pid = pid;

	if(MsgSendnc(PROCMGR_COID, &msg.i, sizeof msg.i, &msg.o, sizeof msg.o) == -1) {
		return (mode_t)-1;
	}
	return msg.o.umask;
}

mode_t umask(mode_t cmask) {
	return _umask(0, cmask);
}

__SRCVERSION("umask.c $Rev: 153052 $");
