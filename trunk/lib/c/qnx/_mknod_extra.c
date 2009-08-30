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




#include <stdarg.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include <sys/iomsg.h>

int _mknod_extra(const char *path, mode_t mode, dev_t dev, unsigned extra_type, unsigned extra_len, void *extra) {
	int					fd;
	int					status;

	if((fd = _connect(_NTO_SIDE_CHANNEL, path, mode, O_CREAT | O_NOCTTY, SH_DENYNO, _IO_CONNECT_MKNOD, 1,
				0, dev, extra_type, extra_len, extra, 0, 0, &status)) == -1) {
		return -1;
	}
	ConnectDetach(fd);
	return status;
}

__SRCVERSION("_mknod_extra.c $Rev: 153052 $");
