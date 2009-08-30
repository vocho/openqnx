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
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>

int getgroups(int gidsetsize, gid_t grouplist[]) {
	struct _client_info			info;

	ConnectClientInfo(-1, &info, NGROUPS_MAX);
	if(gidsetsize) {
		if(gidsetsize < info.cred.ngroups) {
			errno = EINVAL;
			return -1;
		}
		memcpy(grouplist, info.cred.grouplist, info.cred.ngroups * sizeof info.cred.grouplist[0]);
	}
	return info.cred.ngroups;
}

__SRCVERSION("getgroups.c $Rev: 153052 $");
