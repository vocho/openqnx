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
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <sys/resmgr.h>
#include <sys/pathmsg.h>
#include "connect.h"

extern int __dir_keep_symlink; /* _init_libc() */

#ifndef PATH_MAX
#define PATH_MAX	1024
#endif
int chdir(const char *path) {
	struct _connect_ctrl			ctrl;
	struct _io_connect				msg;		
	int								fd;
	int								euid;
	/* Should use fpathconf(path, _PC_PATH_MAX) */
	char							buffer[PATH_MAX + 1];
	struct _io_stat					s;
	struct stat						r;
	iov_t							iov[2];

	memset(&ctrl, 0x00, sizeof ctrl);
	ctrl.base = _NTO_SIDE_CHANNEL;
	ctrl.path = buffer;
	ctrl.pathsize = sizeof buffer;
	ctrl.send = MsgSendvnc;
	ctrl.msg = &msg;
	ctrl.extra = &s;

	s.type = _IO_STAT;
	s.combine_len = sizeof s;
	s.zero = 0;

	memset(&msg, 0x00, sizeof msg);
	msg.ioflag = O_LARGEFILE | O_NOCTTY;
	msg.subtype = _IO_CONNECT_COMBINE_CLOSE;
	msg.extra_len = sizeof s;

	if((fd = _connect_ctrl(&ctrl, path, sizeof r, &r)) == -1) {
		return -1;
	}
	ConnectDetach(fd);

	if(buffer[0] == '\0') {
		errno = ENAMETOOLONG;
		return -1;
	}

	if(!S_ISDIR(r.st_mode)) {
		errno = ENOTDIR;
		return -1;
	}

	/* Need to permission check the last component to verify 
	   that we have execute permission on this last component */
	if ((euid = geteuid()) == 0) {
		; //OK
	}
	else if (euid == r.st_uid) {
		if (!(r.st_mode & S_IXUSR)) {
			errno = EACCES;
			return -1;
		}
	}
	else {
		gid_t *grouplist;
		int	   glsize;

		glsize = getgroups(0, NULL) + 1;
		if (!(grouplist = alloca(glsize * sizeof(gid_t)))) {
			errno = ENOMEM;
			return -1;
		}

		grouplist[0] = getegid();
		glsize = getgroups(glsize-1, &grouplist[1]) + 1;
		while (--glsize >= 0) {
			if (r.st_gid == grouplist[glsize]) {
				break;
			}
		}
	
		//If no matching group id's then we are an 'other'
		if (glsize >= 0 && !(r.st_mode & S_IXGRP)) {
			errno = EACCES;
			return -1;
		}
		else if (glsize < 0 && !(r.st_mode & S_IXOTH)) {
			errno = EACCES;
			return -1;
		}
	}

	memset(&msg, 0x00, sizeof msg);
	msg.type = _PATH_CHDIR;
	if (__dir_keep_symlink == 0) {
		/* Pass the fully resolved path to proc */
		path = buffer;
		if (ctrl.chroot_len == strlen(path)) {
			/* we are in the chroot-ed directory */
			path = "/";
		} else {
			path += ctrl.chroot_len;
		}
	}
	msg.path_len = strlen(path) + 1;
	SETIOV(iov + 0, &msg, offsetof(struct _io_connect, path));
	SETIOV(iov + 1, path, msg.path_len);
	return MsgSendv(PATHMGR_COID, iov, 2, 0, 0);
}

__SRCVERSION("chdir.c $Rev: 205764 $");
