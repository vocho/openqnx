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
#include <devctl.h>
#include <sys/dcmd_chr.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>


int ttyname_r(int fd, char *buf, size_t bufsize) {
	int				err, n;
	struct _ttyinfo	info;
	struct _server_info serverinfo;

	err = devctl(fd, DCMD_CHR_TTYINFO, &info, sizeof(info), 0);
	if(err != EOK)
		return(err);

	if(ConnectServerInfo(0, fd, &serverinfo) == -1) {
		return(errno);
	}

	n = netmgr_ndtostr(ND2S_DIR_SHOW | ND2S_LOCAL_STR, serverinfo.nd, buf, bufsize);
	if(n == -1) {
		return errno;
	}

	n--;
	if(info.ttyname[0] == (char)'/') {
		n--;
	}

	if(strlen(info.ttyname) + n + 1 > bufsize) {
		return(ERANGE);
	}

	strcpy(buf + n, info.ttyname);
	return(EOK);
}

__SRCVERSION("ttyname_r.c $Rev: 153052 $");
