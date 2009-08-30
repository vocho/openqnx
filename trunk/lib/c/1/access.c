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
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/iomsg.h>
#include <sys/dcmd_all.h>
#include <sys/neutrino.h>

#if (R_OK != S_IROTH || W_OK != S_IWOTH || X_OK != S_IXOTH)
#error access() implementation requires *_OK and S_I*OTH equivalence
#endif

static int __access(const char *path, int amode, int oflag) {
	int							fd;
	int							mode, n;
	struct _io_stat				s;
	struct stat					r;
	struct _client_info			info;

	// Only 1003.1a valid modes allowed
	if((amode & (F_OK | R_OK | W_OK | X_OK)) != amode) {
		errno = EINVAL;
		return -1;
	}

	// get the stat info
	s.type = _IO_STAT;
	s.combine_len = sizeof s;
	s.zero = 0;
	if((fd = _connect(_NTO_SIDE_CHANNEL, path, 0, oflag | O_NONBLOCK | O_LARGEFILE | O_NOCTTY, SH_DENYNO, _IO_CONNECT_COMBINE, 0, 0, 0, 0, sizeof s, &s, sizeof r, &r, 0)) == -1) {
		return -1;
	}

	// Use realids instead of effective
	ConnectClientInfo(-1, &info, NGROUPS_MAX);
	if(oflag & O_REALIDS) {
		info.cred.euid = info.cred.ruid;
		info.cred.egid = info.cred.rgid;
	}

	if(info.cred.euid == 0) {
		// super-user always gets read-write access
		// and execute access if any x bit is set
		// and directory search access
		mode = (S_ISDIR(r.st_mode) || (r.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) ? S_IROTH | S_IWOTH | S_IXOTH : S_IROTH | S_IWOTH;
	} else if(info.cred.euid == r.st_uid) {
		mode = r.st_mode >> 6;
	} else if(info.cred.egid == r.st_gid) {
		mode = r.st_mode >> 3;
	} else {
		for (n = 0; n < info.cred.ngroups && info.cred.grouplist[n] != r.st_gid; ++n)
			;
		mode = (n >= info.cred.ngroups) ? r.st_mode : r.st_mode >> 3;
	}

	mode &= R_OK | W_OK | X_OK;
	if((mode | amode) != mode) {
		close(fd);
		errno = EACCES;
		return -1;
	}

	// If not a directory, check mount flags
	if(amode & (W_OK | X_OK)) {
		int				flags;

		if(_devctl(fd, DCMD_ALL_GETMOUNTFLAGS, &flags, sizeof flags, 0) != EOK) {
			// Doesn't support mount flags, so default to NOEXEC
			flags = _MOUNT_NOEXEC;
		}

		// Check read-only file system
		if((amode & W_OK) && (flags & _MOUNT_READONLY)) {
			close(fd);
			errno = EROFS;
			return -1;
		}

		// Check mounted noexec
		if(!S_ISDIR(r.st_mode) && (amode & X_OK) && (flags & _MOUNT_NOEXEC)) {
			close(fd);
			errno = EACCES;
			return -1;
		}
	}

	close(fd);
	return 0;
}

int access(const char *path, int amode) {
	return __access(path, amode, O_REALIDS);
}

int eaccess(const char *path, int amode) {
	return __access(path, amode, 0);
}

__SRCVERSION("access.c $Rev: 153052 $");
