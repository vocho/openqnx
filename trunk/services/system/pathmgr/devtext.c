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

#include "externs.h"
#include <fcntl.h>
#include <sys/dcmd_all.h>
#include <sys/dcmd_chr.h>
#include "pinode.h"

static pthread_mutex_t		text_mutex = PTHREAD_MUTEX_INITIALIZER;
static dev_t text_devno;

static int text_read(resmgr_context_t *ctp, io_read_t *msg, void *ocb) {
	unsigned			ioflag = (uintptr_t)ocb;
	int					c;

	if(!(ioflag & _IO_FLAG_RD)) {
		return EBADF;
	}

	if(ioflag & O_NONBLOCK) {
		if(msg->i.xtype & _IO_XFLAG_BLOCK) {
			ioflag &= ~O_NONBLOCK;
		}
	} else {
		if(msg->i.xtype & _IO_XFLAG_NONBLOCK) {
			ioflag |= O_NONBLOCK;
		}
	}

	switch(msg->i.xtype & _IO_XTYPE_MASK) {
	case _IO_XTYPE_NONE:
		break;
	default:
		// We don't handle this xtype, return error
		return ENOSYS;
	}

	if (!msg->i.nbytes) {
		// Nothing requested so return EOF
		_IO_SET_READ_NBYTES(ctp, 0);
		return EOK;
	}
	for(;;) {
		struct _msg_info			info; 
		uint64_t					to;

		pthread_mutex_lock(&text_mutex);
		c = scrn_poll_key();
		pthread_mutex_unlock(&text_mutex);
		if(c >= 0) {
			break;
		}
		if(ioflag & O_NONBLOCK) {
			return EAGAIN;
		}
		to = 100*1000000;
		(void)TimerTimeout_r(CLOCK_REALTIME, 1 << STATE_NANOSLEEP, 0, &to, NULL);
		if(MsgInfo(ctp->rcvid, &info) != -1 && (info.flags & _NTO_MI_UNBLOCK_REQ)) {
			return EINTR;
		}
	}
	if(c > 0xff) {
		// Non-ascii so return EOF
		_IO_SET_READ_NBYTES(ctp, 0);
		return EOK;
	}
	*(char *)msg = c;
	_IO_SET_READ_NBYTES(ctp, 1);
	return _RESMGR_PTR(ctp, msg, 1);
}

static int text_write(resmgr_context_t *ctp, io_write_t *msg, void *ocb) {
	unsigned			ioflag = (uintptr_t)ocb;

	if(!(ioflag & _IO_FLAG_WR)) {
		return EBADF;
	}

	switch(msg->i.xtype & _IO_XTYPE_MASK) {
	case _IO_XTYPE_NONE:
		break;
	default:
		// We don't handle this xtype, return error
		return ENOSYS;
	}
	if(msg->i.nbytes > ctp->msg_max_size - sizeof msg->i) {
		msg->i.nbytes = ctp->msg_max_size - sizeof msg->i;
	}
	pthread_mutex_lock(&text_mutex);
	scrn_display((char *)(&msg->i) + sizeof msg->i, msg->i.nbytes);
	pthread_mutex_unlock(&text_mutex);
	_IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);
	return EOK;
}

static int text_stat(resmgr_context_t *ctp, io_stat_t *msg, void *ocb) {
	memset(&msg->o, 0x00, sizeof msg->o);
	msg->o.st_ino = PINO_TEXT;
	msg->o.st_size = 1;
	msg->o.st_ctime =
	msg->o.st_mtime =
	msg->o.st_atime = time(0);
	msg->o.st_mode = 0666 | S_IFCHR;
	msg->o.st_nlink = 1;
	msg->o.st_dev = (ctp->info.srcnd << ND_NODE_BITS) | path_devno;
	msg->o.st_rdev = (ctp->info.srcnd << ND_NODE_BITS) | text_devno;
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

static int text_devctl(resmgr_context_t *ctp, io_devctl_t *msg, void *ocb) {
	union {
		int					oflag;
		int					mountflag;
	}					*data = (void *)(msg + 1);
	unsigned			ioflag = (uintptr_t)ocb;
	int					nbytes = 0;
	
	switch((unsigned)msg->i.dcmd) {
	case DCMD_ALL_GETFLAGS:
		data->oflag = (ioflag & ~O_ACCMODE) | ((ioflag - 1) & O_ACCMODE);
		nbytes = sizeof data->oflag;
		break;

	case DCMD_ALL_SETFLAGS:
		break;

	case DCMD_CHR_ISATTY:
		break;

	default:
		return ENOSYS;
	}

	if(nbytes) {
		msg->o.ret_val = 0;
		return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + nbytes);
	}
	return EOK;
}

static const resmgr_io_funcs_t text_io_funcs = {
	_RESMGR_IO_NFUNCS,
	text_read,
	text_write,
	0,
	text_stat,
	0,
	text_devctl
};

static int text_open(resmgr_context_t *ctp, io_open_t *msg, void *handle, void *extra) {
	if(resmgr_open_bind(ctp, (void *)msg->connect.ioflag, 0) == -1) {
		return errno;
	}
	return EOK;
}

static const resmgr_connect_funcs_t text_connect_funcs = {
	_RESMGR_CONNECT_NFUNCS,
	text_open
};

void devtext_init(void) {
	// allocate null directory entry
	resmgr_attach(dpp, NULL, "/dev/text", 0, 0, &text_connect_funcs, &text_io_funcs, 0);
	rsrcdbmgr_proc_devno("dev", &text_devno, -1, 0);
}

__SRCVERSION("devtext.c $Rev: 211988 $");
