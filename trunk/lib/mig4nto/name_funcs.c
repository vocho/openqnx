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
 *  name_funcs.c - QNX 4 to QNX Neutrino migration functions
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/iomsg.h>
#include <malloc.h>
#include <string.h>
#include <mig4nto.h>
#include <mig4nto_procmsg.h>

extern magic_t	magic;	/* defined in mig4nto.h */

int
name_init(void)
{
	procmgr_msg_t	msg;
	procmgr_reply_t	reply;

    msg.hdr.type = _IO_MSG;
	msg.hdr.combine_len = sizeof(msg.hdr);
	msg.hdr.mgrid = MGR_ID_PROCMGR;
	msg.hdr.subtype = _PROCMGR_GET_NID;
	if (MsgSend(magic.procmgr_fd, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
		return -1;
	magic.nid = reply.un.get_nid.nid;

	return 0;
}

/*
 *  qnx_name_attach
 *
 *  Note that currently the only valid values for nid are 0 and the
 *  local nid.
 */
int
qnx_name_attach(nid_t nid, char *name)
{
	procmgr_msg_t   msg;
	procmgr_reply_t reply;

	if (nid != 0 && nid != getnid()) {
		errno = EHOSTUNREACH;
		return -1;
	}
	msg.hdr.type = _IO_MSG;
	msg.hdr.combine_len = sizeof(msg.hdr);
	msg.hdr.mgrid = MGR_ID_PROCMGR;
	msg.hdr.subtype = _PROCMGR_QNX_NAME_ATTACH;
	msg.un.attach.nid = nid;
	strncpy(msg.un.attach.user_name_data, name, _NAME_MAX_LEN);
	msg.un.attach.user_name_data[_NAME_MAX_LEN] = '\0';
	if (MsgSend(magic.procmgr_fd, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
		return -1;
	errno = reply.set_errno_to_this;
	return reply.un.attach.name_id;
}

/*
 *  qnx_name_detach
 *
 *  Note that currently the only valid values for nid are 0 and the
 *  local nid.
 */
int
qnx_name_detach(nid_t nid, int name_id)
{
	procmgr_msg_t   msg;
	procmgr_reply_t reply;

	if (nid != 0 && nid != getnid()) {
		errno = EHOSTUNREACH;
		return -1;
	}
	msg.hdr.type = _IO_MSG;
	msg.hdr.combine_len = sizeof(msg.hdr);
	msg.hdr.mgrid = MGR_ID_PROCMGR;
	msg.hdr.subtype = _PROCMGR_QNX_NAME_DETACH;
	msg.un.detach.nid = nid;
	msg.un.detach.name_id = name_id;
	if (MsgSend(magic.procmgr_fd, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
		return -1;
	errno = reply.set_errno_to_this;
	return reply.un.detach.retval;
}

/*
 *  qnx_name_locate
 *
 *  Note that currently the only valid values for nid are 0 and the
 *  local nid.
 */
int
qnx_name_locate(nid_t nid, char *name, unsigned size, unsigned *copies)
{
	procmgr_msg_t   msg;
	procmgr_reply_t reply;

	if (nid != 0 && nid != getnid()) {
		errno = EHOSTUNREACH;
		return -1;
	}
	msg.hdr.type = _IO_MSG;
	msg.hdr.combine_len = sizeof(msg.hdr);
	msg.hdr.mgrid = MGR_ID_PROCMGR;
	msg.hdr.subtype = _PROCMGR_QNX_NAME_LOCATE;
	msg.un.locate.nid = nid; /* 0 */
	msg.un.locate.vc_size = size;
	strncpy(msg.un.locate.name, name, _NAME_MAX_LEN);
	msg.un.locate.name[_NAME_MAX_LEN] = '\0';
	if (MsgSend(magic.procmgr_fd, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
		return -1;
	errno = reply.set_errno_to_this;
	if (copies)
		*copies = reply.un.locate.copies;
	return reply.un.locate.pid;
}

/*
 *
 *  qnx_name_query
 *
 *  Note that currently the only valid values for proc_pid are 0 or
 *  PROC_PID.
 */
int
qnx_name_query(pid_t proc_pid, int name_id, struct _nameinfo *buffer)
{
	procmgr_msg_t   msg;
	procmgr_reply_t reply;

	if (proc_pid != 0 && proc_pid != PROC_PID) {
		errno = ESRCH;
		return -1;
	}
	msg.hdr.type = _IO_MSG;
	msg.hdr.combine_len = sizeof(msg.hdr);
	msg.hdr.mgrid = MGR_ID_PROCMGR;
	msg.hdr.subtype = _PROCMGR_QNX_NAME_QUERY;
	msg.un.name_query.proc_pid = proc_pid;
	msg.un.name_query.name_id = name_id;
	if (MsgSend(magic.procmgr_fd, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
		return -1;
	errno = reply.set_errno_to_this;
	memcpy(buffer, &reply.un.name_query.buf, sizeof(struct _nameinfo));
	return reply.un.name_query.name_id;
}

/*
 *  qnx_vc_name_attach
 *
 *  Note that currently the only valid values for nid are 0 and the
 *  local nid.
 */
int
qnx_vc_name_attach(nid_t nid, unsigned length, char *name)
{
	if (nid != 0 && nid != getnid()) {
		errno = EHOSTUNREACH;
		return -1;
	}
	if (nid == 0)
		nid = getnid();
	return qnx_name_locate(nid, name, length, NULL);
}

/*
 *  qnx_vc_attach
 *
 *  Note that virual circuits are not currently implemented.  The
 *  only valid value for pid is the process id of a local process
 *  and the only valid values for nid are 0 and the local nid.
 */
int
qnx_vc_attach(nid_t nid, pid_t pid, unsigned length, int flags)
{
	if (nid != 0 && nid != getnid()) {
		errno = EHOSTUNREACH;
		return -1;
	}
	if (kill(pid, 0) != 0) {
		errno = ESRCH;
		return -1;
	}
	return pid;
}

/*
 *  qnx_vc_detach
 *
 *  Note that virual circuits are not currently implemented.  The
 *  only valid value for vid is the process id of a local process.
 */
int
qnx_vc_detach(pid_t vid)
{
	if (kill(vid, 0) != 0) {
		errno = ESRCH;
		return -1;
	}
	return 0;
}
