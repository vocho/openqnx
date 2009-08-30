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
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/iofunc.h>
#include <sys/netmgr.h>
#include "iofunc.h"

int iofunc_fdinfo_default(resmgr_context_t *ctp, io_fdinfo_t *msg, iofunc_ocb_t *ocb) {
	int							pathmax;
	int							len;
	unsigned					flags = msg->i.flags;

	if((pathmax = (ctp->msg_max_size - ctp->offset) - sizeof msg->o) < 0) {
		return EMSGSIZE;
	}
	pathmax = min(pathmax, msg->i.path_len);
	memset(msg->o.zero, 0x00, sizeof msg->o.zero);

	(void)iofunc_fdinfo(ctp, ocb, ocb->attr, &msg->o.info);

	// Check if the request comes from a remote machine.
	if(ND_NODE_CMP(ctp->info.srcnd, ND_LOCAL_NODE)) {
		flags &= ~_FDINFO_FLAG_LOCALPATH;
	}

	if((len = resmgr_pathname(ctp->id, flags, (char *)(&msg->o + 1), pathmax)) == -1) {
		len = 0;
	}

	_IO_SET_FDINFO_LEN(ctp, len);
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + min(pathmax, len));
}

__SRCVERSION("iofunc_fdinfo_default.c $Rev: 153052 $");
