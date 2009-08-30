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




#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS	32
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <devctl.h>
#include <sys/dcmd_all.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

int _vfcntl(int fd, int cmd, va_list ap) {
	union {
		io_dup_t						dup;
		io_space_t						space;
		io_lock_t						lock;
	}								msg;
	iov_t							iov[4];
	int								arg;
	pid_t	pid;

	switch(cmd) {
	case F_DUPFD: {
		struct _server_info				info;
		int								fd2;
		if(fd == -1 || (fd & _NTO_SIDE_CHANNEL) || ConnectServerInfo(0, fd, &info) != fd) {
			errno = EBADF;
			return -1;
		}
		if((fd2 = va_arg(ap, int)) < 0 || (fd2 & _NTO_SIDE_CHANNEL)) {
			errno = EINVAL;
			return -1;
		}		
		if((fd2 = ConnectAttach(info.nd, info.pid, info.chid, fd2, _NTO_COF_CLOEXEC)) == -1) {
			return -1;
		}
		msg.dup.i.type = _IO_DUP;
		msg.dup.i.combine_len = sizeof msg.dup;
		msg.dup.i.info.nd = netmgr_remote_nd(info.nd, ND_LOCAL_NODE);
		msg.dup.i.info.pid = getpid();
		msg.dup.i.info.chid = info.chid;
		msg.dup.i.info.scoid = info.scoid;
		msg.dup.i.info.coid = fd;
		if(MsgSendnc(fd2, &msg.dup.i, sizeof msg.dup.i, 0, 0) == -1) {
			ConnectDetach_r(fd2);
			return -1;
		}
		ConnectFlags_r(0, fd2, FD_CLOEXEC, 0);
		return fd2;
	}

	case F_GETFD:
		return ConnectFlags(0, fd, 0, 0);

	case F_SETFD:
		return ConnectFlags(0, fd, ~0, va_arg(ap, int));
		
	case F_GETFL:
		if(_devctl(fd, DCMD_ALL_GETFLAGS, &arg, sizeof arg, 0) == -1) {
			return -1;
		}
		return arg;

	case F_SETFL:
		arg = va_arg(ap, int);
		return _devctl(fd, DCMD_ALL_SETFLAGS, &arg, sizeof arg, _DEVCTL_FLAG_NORETVAL);

	case F_GETOWN:
		if(_devctl(fd, DCMD_ALL_GETOWN, &pid, sizeof pid, 0) == -1) {
			return -1;
		}
		return pid;
		
	case F_SETOWN:
		pid = va_arg(ap, pid_t);
		return _devctl(fd, DCMD_ALL_SETOWN, &pid, sizeof pid, _DEVCTL_FLAG_NORETVAL);
		
	case F_ALLOCSP64:
	case F_FREESP64: {
		flock64_t				*area = va_arg(ap, flock64_t *);

		msg.space.i.start = area->l_start;
		msg.space.i.len = area->l_len;
		msg.space.i.whence = area->l_whence;
		goto common;
	}
	case F_ALLOCSP:
		cmd = F_ALLOCSP64;	/* Always pass the 64 bit values */
		goto stuff;
	case F_FREESP:
		cmd = F_FREESP64;	/* Always pass the 64 bit values */
stuff: {
		flock_t					*area = va_arg(ap, flock_t *);

		msg.space.i.start = area->l_start;
		msg.space.i.len = area->l_len;
		msg.space.i.whence = area->l_whence;
	}
common:
		msg.space.i.type = _IO_SPACE;
		msg.space.i.combine_len = sizeof msg.space.i;
		msg.space.i.subtype = cmd;
		return MsgSend(fd, &msg.space.i, sizeof msg.space.i, 0, 0);

	case F_GETLK:
	case F_SETLK:
	case F_SETLKW:
	case F_GETLK64:
	case F_SETLK64:
	case F_SETLKW64:
		msg.lock.i.type = _IO_LOCK;
		msg.lock.i.combine_len = sizeof msg.lock.i;
		msg.lock.i.subtype = cmd;
		SETIOV(iov + 0, &msg.lock.i, sizeof msg.lock.i);
		SETIOV(iov + 1, va_arg(ap, flock_t *), sizeof(flock_t));
		iov[3] = iov[1];
		SETIOV(iov + 2, &msg.lock.o, sizeof msg.lock.o);
		return MsgSendv(fd, iov + 0, 2, iov + 2, (cmd == F_GETLK || cmd == F_GETLK64) ? 2 : 1);
		
	default:
		break;
	}

	errno = EINVAL;
	return -1;
}

int fcntl(int fd, int cmd, ...) {
	va_list  	ap;
	int			ret;

	va_start(ap, cmd);
	ret = _vfcntl(fd, cmd, ap);
	va_end(ap);
	return ret;
}

__SRCVERSION("fcntl.c $Rev: 153052 $");
