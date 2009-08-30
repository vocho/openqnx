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
#include <share.h>
#include <string.h>
#include <sys/iofunc.h>

void iofunc_attr_init(iofunc_attr_t *attr, mode_t mode, iofunc_attr_t *dattr, struct _client_info *info) {
	// zero the attribute
	memset(attr, 0x00, sizeof *attr);

	// if parent, make mount point the same
	attr->mount = dattr ? dattr->mount : 0;

	// flag all times as dirty
	attr->flags = IOFUNC_ATTR_DIRTY_TIME | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_ATIME | IOFUNC_ATTR_CTIME;

	// Use effective id's, if no info available use executition ids
	// and set mode and link count
	attr->uid = info ? info->cred.euid : getuid();
	attr->mode = mode;
	if(dattr && (dattr->mode & S_ISGID)) {
		attr->gid = dattr->gid;
		if(S_ISDIR(mode)) {
			attr->mode |= S_ISGID;
		}
	} else {
		attr->gid = info ? info->cred.egid : getgid();
	}

	attr->nlink = 1;
}

__SRCVERSION("iofunc_attr_init.c $Rev: 153052 $");
