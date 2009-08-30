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




#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <share.h>
#include <sys/iomsg.h>
#include <sys/ftype.h>

int pipe(int fildes[2]) {
	if((fildes[0] = _connect(0, "/dev/pipe", 0, O_RDONLY | O_NOCTTY, SH_DENYNO, _IO_CONNECT_OPEN,
			0, _IO_FLAG_RD | _IO_FLAG_WR, _FTYPE_PIPE, 0, 0, 0, 0, 0, 0)) != -1) {
		if((fildes[1] = _sopenfd(fildes[0], O_WRONLY, SH_DENYNO, _IO_OPENFD_PIPE)) != -1) {
			return 0;
		}
		close(fildes[0]);
	} else if(errno == ENOENT) {
		errno = ENOSYS;
	}
	return -1;
}

__SRCVERSION("pipe.c $Rev: 153052 $");
