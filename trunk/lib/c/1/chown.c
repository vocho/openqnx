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
#include <fcntl.h>
#include <share.h>
#include <sys/iomsg.h>

static int _chown(const char *path, uid_t uid, gid_t gid, int mode) {
	struct _io_chown			msg;

	msg.type = _IO_CHOWN;
	msg.combine_len = sizeof msg;
	msg.uid = uid;
	msg.gid = gid;

	return _connect_combine(path, mode, O_NONBLOCK | O_LARGEFILE | O_NOCTTY, SH_DENYNO, 0, 0, sizeof msg, &msg, 0, 0);
}

int lchown(const char *path, uid_t uid, gid_t gid) {
	return _chown(path, uid, gid, S_IFLNK);
}

int chown(const char *path, uid_t uid, gid_t gid) {
	return _chown(path, uid, gid, 0);
}

__SRCVERSION("chown.c $Rev: 153052 $");
