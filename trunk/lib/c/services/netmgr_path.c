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
#include <string.h>
#include <sys/neutrino.h>
#include "netmgr_send.h"

int netmgr_path(const char *netname, const char *suffix, char *path, size_t path_max) {
	int nd, size;
	char *ptr;

	nd = netmgr_strtond(netname, &ptr);
	if(nd == -1) {
		return -1;
	}
	if(suffix != NULL) {
		ptr = (char *)suffix;
	}
	size = netmgr_ndtostr(ND2S_DIR_SHOW|ND2S_DOMAIN_SHOW|ND2S_NAME_SHOW, nd, path, path_max);
	if(size == -1) {
		return -1;
	}
	if((size += strlen(ptr)) > path_max) {
		errno = ENAMETOOLONG;
		return -1;
	}
	strcat(path, ptr);
	return size;
}

__SRCVERSION("netmgr_path.c $Rev: 153052 $");
