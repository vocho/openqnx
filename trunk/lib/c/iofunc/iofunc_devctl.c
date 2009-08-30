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
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/dcmd_all.h>

int iofunc_devctl(resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
	union {
		int					oflag;
		int					mountflag;
	}					*data = (void *)(msg + 1);
	unsigned			nbytes = 0;

	switch((unsigned)msg->i.dcmd) {
	case DCMD_ALL_GETFLAGS:
		data->oflag = (ocb->ioflag & O_SETFLAG) | ((ocb->ioflag - 1) & O_ACCMODE);
		nbytes = sizeof data->oflag;
		break;

	case DCMD_ALL_SETFLAGS:
		if(data->oflag & (O_SYNC | O_DSYNC | O_RSYNC)) {
			iofunc_mount_t				*mountp = attr->mount;

			if(!mountp || (mountp->conf & IOFUNC_PC_SYNC_IO) == 0) {
				return EINVAL;
			}
		}
		ocb->ioflag = (ocb->ioflag & ~O_SETFLAG) | (data->oflag & O_SETFLAG);
		break;

	case DCMD_ALL_GETMOUNTFLAGS: {
		iofunc_mount_t				*mountp = attr->mount;

		data->mountflag = mountp ? (mountp->flags & IOFUNC_MOUNT_FLAGS) : 0;
		nbytes = sizeof data->mountflag;
		break;
	}

 	case DCMD_ALL_FADVISE:
 		if(S_ISFIFO(attr->mode)) {
 			return ESPIPE;
 		}
 		return _RESMGR_DEFAULT;

	default:
		return _RESMGR_DEFAULT;
	}

	if(nbytes) {
		msg->o.ret_val = 0;
		return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + nbytes);
	}
	return EOK;
}

__SRCVERSION("iofunc_devctl.c $Rev: 153052 $");
