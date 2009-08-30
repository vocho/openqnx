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
#include <sys/dcmd_chr.h>


static void ENDIAN_SWAPMQATTR(struct mq_attr *attr)
{
	ENDIAN_SWAP32(&attr->mq_maxmsg);
	ENDIAN_SWAP32(&attr->mq_msgsize);
	ENDIAN_SWAP32(&attr->mq_flags);
	ENDIAN_SWAP32(&attr->mq_curmsgs);
	ENDIAN_SWAP32(&attr->mq_sendwait);
	ENDIAN_SWAP32(&attr->mq_recvwait);
}

int
io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, struct ocb *ocb) {
	MQDEV				*dev = ocb->ocb.attr;
	union {
		struct mq_attr		mq_attr;
	}					*dcp = _DEVCTL_DATA(msg->i);
	struct mqclosemsg	*closemsg;
	int					nbytes = 0;
	int					status;

	// Let common code handle DCMD_ALL_* cases
	if((status = iofunc_devctl_default(ctp, msg, &ocb->ocb)) != _RESMGR_DEFAULT) {
		return status;
	}

	// Check the class for character device requests and ENOTTY those
	status = (msg->i.dcmd >> 8) & 0xFF;
	if (status == _CMD_IOCTL_TTY || status == _DCMD_CHR) {
		return ENOTTY;
	}

	switch(msg->i.dcmd) {

	case DCMD_MISC_MQGETATTR:
		dcp->mq_attr = dev->mq_attr;
		dcp->mq_attr.mq_flags |= ocb->ocb.ioflag & O_NONBLOCK;
		if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			ENDIAN_SWAPMQATTR(&dcp->mq_attr);
		}
		nbytes = sizeof(dcp->mq_attr);
		break;

	case DCMD_MISC_MQSETATTR:
		if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			ENDIAN_SWAPMQATTR(&dcp->mq_attr);
		}
		dev->mq_attr.mq_flags = dcp->mq_attr.mq_flags & ~O_NONBLOCK;
		ocb->ocb.ioflag = (ocb->ocb.ioflag & ~O_NONBLOCK) |
			(dcp->mq_attr.mq_flags & O_NONBLOCK);
		break;

	case DCMD_MISC_MQSETCLOSEMSG:
		closemsg = NULL;
		if(msg->i.nbytes) {
			if(msg->i.nbytes > dev->mq_attr.mq_msgsize) {
				return EMSGSIZE;
			}
			if((closemsg = malloc(sizeof(*closemsg) + msg->i.nbytes - 1)) == NULL) {
				return ENOMEM;
			}
			closemsg->nbytes = msg->i.nbytes;
			memcpy(closemsg->data, dcp, msg->i.nbytes);
		}

		if(ocb->closemsg) {
			free(ocb->closemsg);
		}

		ocb->closemsg = closemsg;
		break;

	default:
		return _RESMGR_DEFAULT;
	}

	if(nbytes == 0) {
		return EOK;
	}

	msg->o.ret_val = 0;
	SETIOV(ctp->iov + 0, &msg->o, sizeof(msg->o) + nbytes);
	return _RESMGR_NPARTS(1);
}

__SRCVERSION("$File: io_devctl.c $$Rev: 169544 $");
