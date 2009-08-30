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




#include <errno.h>
#include <sys/procmsg.h>
#include <sys/resource.h>

int  getrusage(int who, struct rusage *usage) {
	proc_resource_usage_t		msg;

	msg.i.type = _PROC_RESOURCE;
	msg.i.subtype = _PROC_RESOURCE_USAGE;
	msg.i.pid = 0;
	msg.i.who = who;
	return MsgSendnc(PROCMGR_COID, &msg.i, sizeof msg.i, usage, sizeof *usage);
}

__SRCVERSION("getrusage.c $Rev: 153052 $");
