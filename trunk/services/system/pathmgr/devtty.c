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
#include "pinode.h"

static dev_t tty_devno;

static int tty_read(resmgr_context_t *ctp, io_read_t *msg, void *ocb) {
	switch(msg->i.xtype & _IO_XTYPE_MASK) {
	case _IO_XTYPE_NONE:
		break;
	default:
		// We don't handle this xtype, return error
		return EINVAL;
	}
	_IO_SET_READ_NBYTES(ctp, 0);
	return EOK;
}

static int tty_write(resmgr_context_t *ctp, io_write_t *msg, void *ocb) {
	switch(msg->i.xtype & _IO_XTYPE_MASK) {
	case _IO_XTYPE_NONE:
		break;
	default:
		// We don't handle this xtype, return error
		return EINVAL;
	}
	_IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);
	return EOK;
}

static int tty_stat(resmgr_context_t *ctp, io_stat_t *msg, void *ocb) {
	memset(&msg->o, 0x00, sizeof msg->o);
	msg->o.st_ino = PINO_TTY;
	msg->o.st_size = 0;
	msg->o.st_ctime =
	msg->o.st_mtime =
	msg->o.st_atime = time(0);
	msg->o.st_mode = 0666 | S_IFCHR;
	msg->o.st_nlink = 1;
	msg->o.st_dev = (ctp->info.srcnd << ND_NODE_BITS) | path_devno;
	msg->o.st_rdev = (ctp->info.srcnd << ND_NODE_BITS) | tty_devno;
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

static const resmgr_io_funcs_t tty_io_funcs = {
	_RESMGR_IO_NFUNCS,
	tty_read,
	tty_write,
	0,
	tty_stat
};

static int tty_open(resmgr_context_t *ctp, io_open_t *msg, void *extra, void *reserved) {
	struct _io_connect_link_reply	*linkp = (void *)msg;
	struct _server_info				*info = (void *)(linkp + 1);
	io_openfd_t						*new = (void *)(info + 1);
	unsigned						eflag;
	unsigned						ioflag;
	unsigned						sflag;
	SESSION							*sep;
	PROCESS							*prp;

	if(((ioflag = msg->connect.ioflag) & (_IO_FLAG_RD | _IO_FLAG_WR)) == 0) {
		if(resmgr_open_bind(ctp, 0, 0) == -1) {
			return errno;
		}
		return EOK;
	}

	if(ctp->info.nd != ND_LOCAL_NODE) {
		return ENOREMOTE;
	}

	if(!(prp = proc_lock_pid(ctp->info.pid))) {
		return EL2HLT;
	}
	if(!(sep = prp->session) || sep->fd < 0) {
		return proc_error(ENXIO, prp);
	}
	proc_unlock(prp);
	_IO_SET_CONNECT_RET(ctp, _IO_CONNECT_RET_MSG);
	eflag = msg->connect.eflag;
	sflag = msg->connect.sflag;
	memset(linkp, 0x00, sizeof *linkp);
	linkp->eflag = eflag;
	linkp->path_len = sizeof *new;
	if(ConnectServerInfo(0, sep->fd, info) == -1) {
		return ENXIO;
	}
	memset(new, 0x00, sizeof *new);
	new->i.type = _IO_OPENFD;
	new->i.combine_len = sizeof new->i;
	new->i.sflag = sflag;
	new->i.ioflag = ioflag;
	new->i.xtype = _IO_OPENFD_NONE;
	new->i.info.nd = ctp->info.srcnd; // netmgr_remote_nd(ctp->info.nd, ND_LOCAL_NODE);
	new->i.info.pid = getpid();
	new->i.info.chid = info->chid;
	new->i.info.scoid = info->scoid;
	new->i.info.coid = info->coid;
	SETIOV(ctp->iov + 0, new, offsetof(io_openfd_t, i.key));
    (void)MsgKeyData(ctp->rcvid, _NTO_KEYDATA_CALCULATE, 0, &new->i.key, ctp->iov, 1);
	return _RESMGR_PTR(ctp, msg, sizeof *linkp + sizeof *info + sizeof *new);
}

static const resmgr_connect_funcs_t tty_connect_funcs = {
	_RESMGR_CONNECT_NFUNCS,
	tty_open
};

void devtty_init(void) {
	resmgr_attach(dpp, NULL, "/dev/tty", 0, 0, &tty_connect_funcs, &tty_io_funcs, 0);
	rsrcdbmgr_proc_devno("dev", &tty_devno, -1, 0);
}

__SRCVERSION("devtty.c $Rev: 211988 $");
