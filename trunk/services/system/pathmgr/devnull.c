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

#include <fcntl.h>
#include <sys/dcmd_all.h>
#include "externs.h"
#include "pinode.h"

static dev_t null_devno;

static int null_read(resmgr_context_t *ctp, io_read_t *msg, void *ocb) {
	unsigned		ioflag = (uintptr_t)ocb;

	if(!(ioflag & _IO_FLAG_RD)) {
		return EBADF;
	}
	switch(msg->i.xtype & _IO_XTYPE_MASK) {
	case _IO_XTYPE_NONE:
	case _IO_XTYPE_OFFSET:	 
		break;
	default:
		// We don't handle this xtype, return error
		return ENOSYS;
	}
	_IO_SET_READ_NBYTES(ctp, 0);
	return EOK;
}

static int null_write(resmgr_context_t *ctp, io_write_t *msg, void *ocb) {
	unsigned		ioflag = (uintptr_t)ocb;

	if(!(ioflag & _IO_FLAG_WR)) {
		return EBADF;
	}
	switch(msg->i.xtype & _IO_XTYPE_MASK) {
	case _IO_XTYPE_NONE:
	case _IO_XTYPE_OFFSET:	
		break;
	default:
		// We don't handle this xtype, return error
		return ENOSYS;
	}
	_IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);
	return EOK;
}

static int null_stat(resmgr_context_t *ctp, io_stat_t *msg, void *ocb) {
	memset(&msg->o, 0x00, sizeof msg->o);
	msg->o.st_ino = PINO_NULL;
	msg->o.st_size = 0;
	msg->o.st_ctime =
	msg->o.st_mtime =
	msg->o.st_atime = time(0);
	msg->o.st_mode = 0666 | S_IFCHR;
	msg->o.st_nlink = 1;
	msg->o.st_dev = (ctp->info.srcnd << ND_NODE_BITS) | path_devno;
	msg->o.st_rdev = (ctp->info.srcnd << ND_NODE_BITS) | null_devno;
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

static int null_devctl(resmgr_context_t *ctp, io_devctl_t *msg, void *ocb) {
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

	default:
		return ENOSYS;
	}

	if(nbytes) {
		msg->o.ret_val = 0;
		return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + nbytes);
	}
	return EOK;
}

static const resmgr_io_funcs_t null_io_funcs = {
	_RESMGR_IO_NFUNCS,
	null_read,
	null_write,
	0,
	null_stat,
	0,
	null_devctl
};

static int null_open(resmgr_context_t *ctp, io_open_t *msg, void *extra, void *reserved) {
	if(resmgr_open_bind(ctp, (void *)msg->connect.ioflag, 0) == -1) {
		return errno;
	}
	return EOK;
}

static const resmgr_connect_funcs_t null_connect_funcs = {
	_RESMGR_CONNECT_NFUNCS,
	null_open
};

void devnull_init(void) {
	resmgr_attr_t	rattr;

	// allocate null directory entry
	memset(&rattr, 0x00, sizeof(rattr));
	rattr.flags = RESMGR_FLAG_CROSS_ENDIAN;
	resmgr_attach(dpp, &rattr, "/dev/null", 0, 0, &null_connect_funcs, &null_io_funcs, 0);
	rsrcdbmgr_proc_devno("dev", &null_devno, -1, 0);
}

__SRCVERSION("devnull.c $Rev: 211988 $");
