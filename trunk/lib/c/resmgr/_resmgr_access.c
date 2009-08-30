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
#include <sys/resmgr.h>
#include "resmgr.h"

int _resmgr_access(resmgr_context_t *ctp, struct _msg_info *msginfo, int ioflag, unsigned key1, void *msg, int size) {
	int								status;
	struct _client_info				info1, info2;

	// If the same process, it is fine
	if(ctp->info.scoid == msginfo->scoid) {
		return EOK;
	}

	if((status = ConnectClientInfo_r(ctp->info.scoid, &info1, 0)) != EOK) {
		return status;
	}

	if((status = ConnectClientInfo_r(msginfo->scoid, &info2, 0)) != EOK) {
		return status;
	}

	// allow root or same effective user id.
	if(ioflag & O_REALIDS) {
		if(info1.cred.ruid == 0 || info1.cred.ruid == info2.cred.ruid) {
			return EOK;
		}
	} else {
		if(info1.cred.euid == 0 || info1.cred.euid == info2.cred.euid) {
			return EOK;
		}
	}

	if(msg && key1) {
		unsigned						key2;

		SETIOV(ctp->iov + 0, msg, size);

		// If key is valid, the process manager authorized this dup, so allow it
		if(MsgKeyData_r(ctp->rcvid, _NTO_KEYDATA_VERIFY, key1, &key2, ctp->iov + 0, 1) == EOK && key2 == 0) {
			return EOK;
		}
	}

	return EACCES;
}

__SRCVERSION("_resmgr_access.c $Rev: 153052 $");
