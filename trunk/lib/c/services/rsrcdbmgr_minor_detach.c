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
#include <sys/neutrino.h>
#include <sys/rsrcdbmgr.h>
#include <sys/rsrcdbmsg.h>
#include <sys/types.h>

#define RSRCDBMGR_MINOR_NUM RSRCDBMGR_TYPE_COUNT

int rsrcdbmgr_minor_detach(int major, int minor) {
	dev_t	dev;

	dev = makedev(0, major, minor);
	return(rsrcdbmgr_devno_detach(dev, 0));
}

int rsrcdbmgr_devno_detach(dev_t devno, int flags) {
	iov_t					iovout[2], iovin[1];
	rsrc_cmd_t				request;
	rsrc_minor_request_t	data;

	request.i.type = RSRCDBMGR_RSRC_CMD;
	request.i.subtype = RSRCDBMGR_REQ_DETACH | RSRCDBMGR_MINOR_NUM;
	request.i.pid = 0;
	request.i.count = 1;
	request.i.nbytes = sizeof(data);

	data.major = major(devno);
	data.minor = minor(devno);	
	data.flags = 0;
	data.name = NULL;

	SETIOV(&iovout[0], &request, sizeof(request));
	SETIOV(&iovout[1], &data, sizeof(data));
	SETIOV(&iovin[0], &data, sizeof(data));

	if (MsgSendv(RSRCDBMGR_COID, iovout, 2, iovin, 1) == -1) {
		return(-1);
	}

	return(EOK);	
}

__SRCVERSION("rsrcdbmgr_minor_detach.c $Rev: 153052 $");
