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
#include <sys/resmgr.h>

int iofunc_client_info(resmgr_context_t *ctp, int ioflag, struct _client_info *info) {
	int					status;

	if((status = ConnectClientInfo_r(ctp->info.scoid, info, NGROUPS_MAX)) != EOK) {
		return status;
	}
	if(ioflag & O_REALIDS) {
		uid_t				uid;
		gid_t				gid;

		uid = info->cred.ruid, info->cred.ruid = info->cred.euid, info->cred.euid = uid;
		gid = info->cred.rgid, info->cred.rgid = info->cred.egid, info->cred.egid = gid;
	}
	return EOK;
}

__SRCVERSION("iofunc_client_info.c $Rev: 153052 $");
