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
#include <stdlib.h>
#include <string.h>
#include <sys/neutrino.h>
#include <sys/rsrcdbmgr.h>
#include <sys/rsrcdbmsg.h>
#include <sys/types.h>

#define RSRCDBMGR_MINOR_NUM RSRCDBMGR_TYPE_COUNT

int rsrcdbmgr_minor_attach(int major, int minor_guess) {
	dev_t ret;
	char  buffer[20];

	itoa(major, buffer, 10);
	ret = rsrcdbmgr_devno_attach(buffer, minor_guess, 0);
	return((ret == (dev_t)-1)  ? ret : minor(ret));
}

dev_t rsrcdbmgr_devno_attach(const char *name, int minor_guess, int flags) {
	iov_t					iovout[3], iovin[1];
	rsrc_cmd_t				request;
	rsrc_minor_request_t	data;

	if (name && strchr(name, '/')) {
		errno = EINVAL;
		return((dev_t)-1);
	}
	if (!name)
		name = " ";

	request.i.type = RSRCDBMGR_RSRC_CMD;
	request.i.subtype = RSRCDBMGR_REQ_ATTACH | RSRCDBMGR_MINOR_NUM;
	request.i.pid = 0;
	request.i.count = 1;
	request.i.nbytes = sizeof(data) + strlen(name) +1;

	data.major = 0;
	data.minor = minor_guess;	
	data.flags = flags;
	data.name = NULL;

	SETIOV(&iovout[0], &request, sizeof(request));
	SETIOV(&iovout[1], &data, sizeof(data));
	SETIOV(&iovout[2], name, strlen(name) +1);
	SETIOV(&iovin[0], &data, sizeof(data));

	if (MsgSendv(RSRCDBMGR_COID, iovout, 3, iovin, 1) == -1) {
		return((dev_t)-1);
	}

	return(makedev(0, data.major, data.minor));	
}


__SRCVERSION("rsrcdbmgr_minor_attach.c $Rev: 153052 $");
