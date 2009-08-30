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
#include <sys/iofunc.h>

int iofunc_pathconf(resmgr_context_t *ctp, io_pathconf_t *msg, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
	iofunc_mount_t			*mountp = attr->mount;
	int						value = 0;
	unsigned				conf;

	if(mountp) {
		conf = mountp->conf;
	} else {
		conf = IOFUNC_PC_CHOWN_RESTRICTED | IOFUNC_PC_NO_TRUNC;
	}

	switch(msg->i.name) {
	case _PC_CHOWN_RESTRICTED:
		value = (conf & IOFUNC_PC_CHOWN_RESTRICTED) ? 1 : 0;
		break;

	case _PC_NO_TRUNC:
		value = (conf & IOFUNC_PC_NO_TRUNC) ? 1 : 0;
		break;

	case _PC_SYNC_IO:
		value = (conf & IOFUNC_PC_SYNC_IO) ? 1 : 0;
		break;

	case _PC_LINK_DIR:
		value = (conf & IOFUNC_PC_LINK_DIR) ? 1 : 0;
		break;

	case _PC_SYMLOOP_MAX:
		value = SYMLOOP_MAX;
		break;

	case _PC_ASYNC_IO:
		value = _POSIX_ASYNC_IO;
		break;

	case _PC_PRIO_IO:
		value = _POSIX_PRIO_IO;
		break;

	default:
		return _RESMGR_DEFAULT;
	}

	_IO_SET_PATHCONF_VALUE(ctp, value);
	return EOK;
}

__SRCVERSION("iofunc_pathconf.c $Rev: 153052 $");
