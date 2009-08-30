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

int _connect_entry(int base, const char *path, mode_t mode, unsigned oflag, unsigned sflag, unsigned subtype, int testcancel,
		unsigned accessl, unsigned file_type, unsigned extra_type, unsigned extra_len, const void *extra,
		unsigned response_len, void *response, int *status, struct _io_connect_entry *entry, int enoretry) {
	struct _connect_ctrl			ctrl;
	struct _io_connect				msg;		
	int								fd;

	memset(&ctrl, 0x00, sizeof ctrl);
	ctrl.base = base;
	ctrl.extra = extra;
	ctrl.send = testcancel ? MsgSendv : MsgSendvnc;
	ctrl.msg = &msg;
	if((ctrl.entry = entry)) {
		ctrl.flags |= FLAG_TEST_ENTRY | ((enoretry) ? FLAG_NO_RETRY : 0);
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
	return fd;
}

__SRCVERSION("_connect_entry.c $Rev: 153052 $");
