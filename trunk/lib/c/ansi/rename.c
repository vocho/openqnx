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
#include <sys/resmgr.h>
#include "connect.h"

#include "stkchk.h"

#ifndef PATH_MAX
#define PATH_MAX	1024	/* Should use fpathconf(path, _PC_PATH_MAX) */
#endif

int rename(const char *old, const char *new) {
	struct _connect_ctrl			ctrl;
	struct _io_connect				msg;		
	struct _io_connect_entry		entry;
	char							buffer[PATH_MAX + 1];
	int								fd;

	memset(&ctrl, 0x00, sizeof ctrl);
	ctrl.base = _NTO_SIDE_CHANNEL;
	ctrl.entry = &entry;
	ctrl.flags = FLAG_NO_PREFIX | FLAG_SET_ENTRY;
	ctrl.path = buffer;
	ctrl.pathsize = sizeof buffer;
	ctrl.send = MsgSendvnc;
	ctrl.msg = &msg;

	memset(&msg, 0x00, sizeof msg);
	msg.ioflag = O_LARGEFILE | O_NOCTTY;
	msg.subtype = _IO_CONNECT_COMBINE_CLOSE;
	msg.mode = S_IFLNK;

	if((fd = _connect_ctrl(&ctrl, old, 0, 0)) == -1) {
		return -1;
	}
	ConnectDetach(fd);

	if(buffer[0] == '\0') {
		errno = ENAMETOOLONG;
		return -1;
	}
	if (buffer[0] == '/' && buffer[1] == '\0')
		buffer[0] = '\0';

	msg.extra_type = _IO_CONNECT_EXTRA_RENAME;
	msg.extra_len = strlen(buffer) + 1;
	msg.subtype = _IO_CONNECT_RENAME;
	ctrl.extra = buffer;
	ctrl.flags = FLAG_TEST_ENTRY | FLAG_ERR_COLLISION;

	//If we fail here, it was likely because we couldn't
	//find a device which would handle us. If we get
	//ENXIO then switch it to an EXDEV in this case
	if((fd = _connect_ctrl(&ctrl, new, 0, 0)) == -1) {
		errno = ((errno == ENXIO) || (errno == EEXIST)) ? EXDEV : errno;
		return -1;
	}
	ConnectDetach(fd);

	return 0;
}

__SRCVERSION("rename.c $Rev: 153052 $");
