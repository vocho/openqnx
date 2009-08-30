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
#include <sys/iomsg.h>

long fpathconf(int fd, int name) {
	io_pathconf_t				msg;
	int status;
	int saved_errno;

	msg.i.type = _IO_PATHCONF;
	msg.i.combine_len = sizeof msg.i;
	msg.i.zero = 0;
	msg.i.name = name;
	saved_errno = errno;
        status = MsgSendnc(fd, &msg.i, sizeof msg.i, 0, 0);
        switch (name) {
                case _PC_2_SYMLINKS:
                        if (status == -1) {
                                if (errno == ENOSYS)
                                        errno = saved_errno;
                        }
                        break;
                default:
                        break;
        }
        return (status);
}

__SRCVERSION("fpathconf.c $Rev: 155374 $");
