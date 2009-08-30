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
#include <string.h>
#include <sys/neutrino.h>
#include <sys/rsrcdbmgr.h>
#include <sys/rsrcdbmsg.h>
#include <inttypes.h>

#include <stdio.h>

#define  RSRCDBMGR_REQ_COUNT   (12 << 8)

/*
 This allows you to retreive information from the resource database:
 pid == 0 
	-> Query all free blocks
 pid == -1
	-> Query all used blocks
 pid == -2
	-> Query all free & used blocks
 pid > 0 
	-> Query blocks owned by pid

 If list == NULL || listcnt == 0 
	-> Return the number of blocks which match the criteria

 Otherwise return a maximum of 'listcnt' blocks of data from the offset 
 'start' (0 based) which match the criteria 'name'. 

 [message]
 [name]
*/
int rsrcdbmgr_query_name(rsrc_alloc_t *list, int listcnt, int start, pid_t pid, char *name, unsigned type) {
	iov_t		iovin[1], iovout[2];
	rsrc_cmd_t	request;

	if (start < 0 || listcnt < 0) {
		errno = EINVAL;
		return(-1);
	}

	request.i.type = RSRCDBMGR_RSRC_CMD;

	if (!list || !listcnt) {	//Interested in the count
		listcnt = start = 0;
		request.i.subtype = RSRCDBMGR_REQ_COUNT;
	} else {					//Interested in the data
		request.i.subtype = RSRCDBMGR_REQ_QUERY_NAME;
	}
	request.i.subtype |= (type & RSRCDBMGR_TYPE_MASK);

	request.i.pid = pid;
	request.i.count = listcnt;
	request.i.index = start;
	request.i.nbytes = (name) ? strlen(name) + 1 : 0;

	SETIOV(&iovout[0], &request, sizeof(request));
	SETIOV(&iovout[1], name, request.i.nbytes);
	SETIOV(&iovin[0], list, listcnt * sizeof(rsrc_alloc_t));

	return(MsgSendv(RSRCDBMGR_COID, iovout, 2, iovin, (listcnt) ? 1 : 0));
}

/*
 We have to support this historical method of access.  It is really
 totally brain-dead, but that is life so we do some work to fake it
 up as much as possible.  Things which we don't map over include:
 - We don't fill in the type field of the flags
*/
int rsrcdbmgr_query(rsrc_alloc_t *list, int listcnt, int start, uint32_t type) {
	int count, ret;
	pid_t pid;

	pid = (type & RSRCDBMGR_FLAG_USED) ? -1 : 0;

	count = rsrcdbmgr_query_name(NULL, 0, start, pid, NULL, type);
	if(count <= 0 || !list || !listcnt) {
		return count;
	}

	ret = rsrcdbmgr_query_name(list, listcnt, start, pid, NULL, type);

	return (ret != -1) ? count : ret;
}

__SRCVERSION("rsrcdbmgr_query.c $Rev: 153052 $");
