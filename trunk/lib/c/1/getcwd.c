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




#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <sys/iomsg.h>
#include "connect.h"

#ifndef PATH_MAX
#define PATH_MAX	1024
#endif

/* NULL buffer means allocate one which the user is responsible for freeing */


char *getcwd(char *buf, size_t size) {
	struct _connect_ctrl			ctrl;
	struct _io_connect				msg;		
	int								fd, len;
	char							*save;
	int saved_errno;

	/* If buffer is NULL, ignore size and malloc */
	if(buf != NULL && size <= 0) {
		errno = EINVAL;
		return 0;
	}

	/* Should use fpathconf(".", _PC_PATH_MAX) */
	if((save = buf) == NULL && (buf = malloc(size = PATH_MAX + 1)) == NULL) {
		errno = ENOMEM;
		return 0;
	}

	memset(&ctrl, 0x00, sizeof ctrl);
	ctrl.base = _NTO_SIDE_CHANNEL;
	ctrl.path = buf;
	ctrl.pathsize = size;
	ctrl.flags = _IO_CONNECT_RET_CHROOT;

	/*
	 * If we're resolving symlinks in chdir() (see __dir_keep_symlink
	 * in chdir() and init_libc()) we should never get one here;
	 * however, if we're leaving them in, don't resolve them here.
	 */
	ctrl.flags |= FLAG_NO_SYM;

	ctrl.send = MsgSendvnc;
	ctrl.msg = &msg;

	memset(&msg, 0x00, sizeof msg);
	msg.ioflag = O_LARGEFILE | O_NOCTTY;
	msg.subtype = _IO_CONNECT_COMBINE_CLOSE;

	saved_errno=errno;
	if((fd = _connect_ctrl(&ctrl, ".", 0, 0)) == -1) {
		if(save == NULL) {
			free(buf);
		}
		return 0;
	}
	errno=saved_errno;
	ConnectDetach(fd);

	if(buf[0] == '\0') {
		errno = ERANGE;
		if(save == NULL) {
			free(buf);
		}
		return 0;
	}

	//return path relative to last chroot
	if (ctrl.chroot_len > 0) {
		len = strlen(buf);
		if (ctrl.chroot_len == len) {
			//we are in the chroot-ed directory
			strlcpy(buf, "/", size);
		} else { 
			memmove(buf, buf + ctrl.chroot_len,
			    (len + 1) - ctrl.chroot_len);
		}
	}
	
	return buf;
}

__SRCVERSION("getcwd.c $Rev: 206511 $");




