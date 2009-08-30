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
#include <sys/iofunc.h>

int iofunc_check_access(resmgr_context_t *ctp, const iofunc_attr_t *attr, mode_t check, const struct _client_info *info) {
	mode_t						mode;

	// Must supply an info entry
	if(!info) {
		return ENOSYS;
	}

	// Root can do anything
	if(info->cred.euid == 0) {
		return EOK;
	}

	// Check for matching owner
	if((check & S_ISUID) && info->cred.euid == attr->uid) {
		return EOK;
	}

	// Check for matching group
	if(check & S_ISGID) {
		unsigned						n;

		if(info->cred.egid == attr->gid) {
			return EOK;
		}

		for(n = 0; n < info->cred.ngroups; n++) {
			if(info->cred.grouplist[n] == attr->gid) {
				return EOK;
			}
		}
	}

	// If not checking read/write/exec perms, return error
	if((check &=  S_IRWXU) == 0) {
		return EPERM;
	}

	if(info->cred.euid == attr->uid) {
		mode = attr->mode;
	} else if(info->cred.egid == attr->gid) {
		mode = attr->mode << 3;
	} else {
		unsigned						n;

		mode = attr->mode << 6;
		for(n = 0; n < info->cred.ngroups; n++) {
			if(info->cred.grouplist[n] == attr->gid) {
				mode = attr->mode << 3;
				break;
			}
		}
	}

	// Check if all permissions are true
	if((mode & check) != check) {
		return EACCES;
	}

	return EOK;
}

__SRCVERSION("iofunc_check_access.c $Rev: 153052 $");
