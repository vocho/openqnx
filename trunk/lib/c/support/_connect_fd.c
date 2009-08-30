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




//Force rebuild
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/pathmgr.h>
#include <sys/pathmsg.h>
#include "connect.h"

#include "stkchk.h"

/*
 This function is currently a work in progress, it will eventually return all of the
 fd's for a given path and mode etc:

 Flags are the same as for _connect but we assume the following:
 mode  = 
 oflag = O_NONBLOCK | O_RDONLY
 sflag = SH_DENYNO
 access = _IO_FLAG_RD
 file_type = FTYPE_ANY
 extra_type = FTYPE_ANY
*/
int _connect_fd(int base, const char *path, mode_t mode, unsigned oflag, unsigned sflag, unsigned subtype, 
                int testcancel, unsigned accessl, unsigned file_type, unsigned extra_type, unsigned extra_len, 
			    const void *extra, unsigned response_len, void *response, int *status, int *fd_len, void *fd_array) {
	struct _connect_ctrl			ctrl;
	struct _io_connect				msg;		
	int								fd;

	memset(&ctrl, 0x00, sizeof ctrl);
	ctrl.base = base;
	ctrl.extra = extra;
	ctrl.send = testcancel ? MsgSendv : MsgSendvnc;
	ctrl.msg = &msg;

	//If fd_array == NULL, don't bother with fd_array
	//If fd_len == 0 and fd_array set then we allocate memory dynamically
	//If fd_len != 0 then we don't allocate memory for the user.
	if (fd_array && fd_len) {
		if (!*fd_len) {
			ctrl.fds_len = FD_BUF_INCREMENT;
			ctrl.fds = malloc(ctrl.fds_len*sizeof(*ctrl.fds)) ;
			ctrl.flags |= FLAG_MALLOC_FDS;
		}
		else if (*fd_len) {
			ctrl.fds = *(int **)fd_array;
			ctrl.fds_len = *fd_len;
		}
	}

	memset(&msg, 0x00, sizeof msg);
	msg.subtype = subtype;
	msg.sflag = sflag;
	msg.ioflag = oflag;
	msg.mode = mode;
	msg.file_type = file_type;
	msg.access = accessl;
	msg.extra_type = extra_type;
	msg.extra_len = extra_len;

	fd = _connect_ctrl(&ctrl, path, response_len, response);
	if(status) {
		*status = ctrl.status;
	}

	//On return make sure that fd_len contains the number of fd's
	if (ctrl.flags & FLAG_MALLOC_FDS) {
		*(int **)fd_array = ctrl.fds;
	}
	if (fd_len) {
		*fd_len = ctrl.fds_index;
	}

	return fd;
}

__SRCVERSION("_connect_fd.c $Rev: 153052 $");
