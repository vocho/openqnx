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
#include <sys/procmsg.h>
#include <sys/neutrino.h>

static int _qnx_setids(pid_t pid, id_t eid, id_t rid, unsigned subtype) {
	proc_getsetid_t					msg;

	msg.i.type = _PROC_GETSETID;
	msg.i.subtype = subtype;
	msg.i.pid = pid;
	msg.i.eid = eid;
	msg.i.rid = rid;
	
	return MsgSendnc(PROCMGR_COID, &msg.i, sizeof msg.i, &msg.o, sizeof msg.o);
}

int setuid(uid_t uid) {
	if(uid == (uid_t)-1) {
		errno = EINVAL;	
		return -1;
	}
	return _qnx_setids(0, uid, -1, _PROC_ID_SETUID);
}

int seteuid(uid_t uid) {
	if(uid == (uid_t)-1) {
		errno = EINVAL;	
		return -1;
	}
	return _qnx_setids(0, uid, -1, _PROC_ID_SETEUID);
}

int setreuid(uid_t rid, uid_t eid) {
	return _qnx_setids(0, eid, rid, _PROC_ID_SETREUID);
}

int setgid(gid_t gid) {
	if(gid == (gid_t)-1) {
		errno = EINVAL;	
		return -1;
	}
	return _qnx_setids(0, gid, -1, _PROC_ID_SETGID);
}

int setegid(gid_t gid) {
	if(gid == (gid_t)-1) {
		errno = EINVAL;	
		return -1;
	}
	return _qnx_setids(0, gid, -1, _PROC_ID_SETEGID);
}

int setregid(gid_t rid, gid_t eid) {
	return _qnx_setids(0, eid, rid, _PROC_ID_SETREGID);
}

__SRCVERSION("setids.c $Rev: 153052 $");
