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
#include <sys/iomsg.h>

int rmdir(const char *path) {
	int							fd;

	if((fd = _connect(_NTO_SIDE_CHANNEL, path, S_IFDIR, O_NOCTTY, SH_DENYNO, _IO_CONNECT_UNLINK, 0, 0, 0, 0, 0, 0, 0, 0, 0)) == -1) {
		return -1;
	}
	ConnectDetach(fd);
	return 0;
}

__SRCVERSION("rmdir.c $Rev: 153052 $");
