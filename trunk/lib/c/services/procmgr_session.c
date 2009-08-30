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
#include <errno.h>
#include <inttypes.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <sys/procmsg.h>
#include <sys/netmgr.h>

int procmgr_session(uint32_t nd, pid_t sid, int id, unsigned event) {
	proc_session_t			msg;

	// @@@@ Should make this work remotely
	if(nd != ND_LOCAL_NODE) {
		errno = ENOREMOTE;
		return -1;
	}
	msg.i.type = _PROC_SESSION;
	msg.i.subtype = 0;
	msg.i.sid = sid;	
	msg.i.id = id;	
	msg.i.event = event;

	return MsgSendnc(PROCMGR_COID, &msg.i, sizeof msg.i, 0, 0);
}

__SRCVERSION("procmgr_session.c $Rev: 153052 $");
