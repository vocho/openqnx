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
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <sys/resmgr.h>
#include "connect.h"

#include "stkchk.h"

char *_fullpath(char *buffer, const char *path, size_t size) {
	struct _connect_ctrl			ctrl;
	struct _io_connect				msg;		
	int								fd;
	char							*orig_buffer;

	orig_buffer = buffer;
	if(!buffer) {
		if(size == 0 || !(buffer = malloc(size))) {
			errno = ENOMEM;
			return 0;
		}
	}
	memset(&ctrl, 0x00, sizeof ctrl);
	ctrl.base = _NTO_SIDE_CHANNEL;
	ctrl.path = buffer;
	ctrl.pathsize = size;
	ctrl.send = MsgSendvnc;
	ctrl.msg = &msg;

	memset(&msg, 0x00, sizeof msg);
	msg.ioflag = O_LARGEFILE | O_NOCTTY;
	msg.subtype = _IO_CONNECT_COMBINE_CLOSE;

	if((fd = _connect_ctrl(&ctrl, path, 0, 0)) == -1) {
		goto failure;
	}
	ConnectDetach(fd);

	if(buffer[0] == '\0') {
		errno = ENAMETOOLONG;
		goto failure;
	}

	return buffer;

failure:
	if(orig_buffer == NULL) {
		// We malloc'd the storage.
		free(buffer);
	}
	return 0;
}

__SRCVERSION("_fullpath.c $Rev: 153052 $");
