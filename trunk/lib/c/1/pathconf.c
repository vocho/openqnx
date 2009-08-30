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
#include <errno.h>

long pathconf(const char *path, int name) {
	struct _io_pathconf			s;
	int status;
	int fd;

	s.type = _IO_PATHCONF;
	s.combine_len = sizeof s;
	s.zero = 0;
	s.name = name;

        switch (name) {
                case _PC_2_SYMLINKS:
			status = -1;
			fd = open(path, O_RDONLY | O_NONBLOCK | O_LARGEFILE | O_NOCTTY);
			if (fd != -1) {
				status = fpathconf(fd, name);
				close(fd);
			}
                        break;
                default:
			status = _connect_combine(path, 0, O_NONBLOCK | O_LARGEFILE | O_NOCTTY, SH_DENYNO, 0, 0, sizeof s, &s, 0, 0);
                        break;
        }
        return (status);

}

__SRCVERSION("pathconf.c $Rev: 158217 $");
