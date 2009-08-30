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




#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/procmsg.h>

int setgroups(int gidsetsize, const gid_t *grouplist) {
	proc_getsetid_t					msg;
	iov_t							iov[2];

	msg.i.type = _PROC_GETSETID;
	msg.i.subtype = _PROC_ID_SETGROUPS;
	msg.i.pid = 0;
	msg.i.ngroups = gidsetsize;
	SETIOV(iov + 0, &msg.i, sizeof msg.i);
	SETIOV(iov + 1, grouplist, gidsetsize * sizeof *grouplist);
	
	return MsgSendvnc(PROCMGR_COID, iov + 0, 2, 0, 0);
}

__SRCVERSION("setgroups.c $Rev: 153052 $");
