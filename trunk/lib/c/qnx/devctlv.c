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




#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <devctl.h>
#include <malloc.h>
#include <errno.h>
#include <stdlib.h>

int devctlv(int fd, int dcmd, int sparts, int rparts, const iov_t *sv, const iov_t *rv, int *info_ptr) {
	io_devctl_t				msg;
	int						i, idx;
	int						sbytes;
	int						rbytes;
	iov_t					*iov;
	int						status;

	if(!(iov = alloca((2 + sparts + rparts) * sizeof *iov))) {
		return ENOMEM;
	}

	// Setup data to the device.
	iov[0].iov_base = (caddr_t)&msg.i;
	iov[0].iov_len = sizeof(msg.i);
	sbytes = 0;
	idx = 1;
	for(i = 0; i < sparts; i++) {
		sbytes += sv[i].iov_len;
		iov[idx++] = sv[i];
	}
	sparts++;
	iov[idx].iov_base = (caddr_t)&msg.o;
	iov[idx].iov_len = sizeof(msg.o);
	idx++;
	rbytes = 0;
	for(i = 0; i < rparts; i++) {
		rbytes += rv[i].iov_len;
		iov[idx++] = rv[i];
	}
	rparts++;

	// Stuff the message.
	msg.i.type = _IO_DEVCTL;
	msg.i.combine_len = sizeof msg.i;
	msg.i.dcmd = dcmd;
	msg.i.nbytes = max(sbytes, rbytes);
	msg.i.zero = 0;

	if((status = MsgSendv_r(fd, iov + 0, sparts, iov + sparts, rparts)) != EOK) {
		return status == -ENOSYS ? ENOTTY : -status;
	}

	if(info_ptr) {
		*info_ptr = msg.o.ret_val;
	}

	return EOK;
}

__SRCVERSION("devctlv.c $Rev: 153052 $");
