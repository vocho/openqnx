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
#include <stddef.h>
#include <inttypes.h>
#include <sys/rsrcdbmgr.h>
#include <sys/rsrcdbmsg.h>

extern int _rsrcdbmgr_pack(void *header, int hbytes, int nbytes_off,
					       void *data, int dwidth, int noff, int nelm,
					       void *reply, int rbytes);

int rsrcdbmgr_attach(rsrc_request_t *list, int32_t count) {
	rsrc_cmd_t	request;
	int			ret;

	if (!list || !count) {
		errno = EINVAL;
		return -1;
	}

	request.i.type =  RSRCDBMGR_RSRC_CMD;
	request.i.subtype = RSRCDBMGR_REQ_ATTACH;
	request.i.pid = 0;
	request.i.count = count;
	//request.i.nbytes set in the pack code

	/* Quickly zero out the name if it isn't being used */
	for(ret=0; ret < count; ret++) { 
		if(!(list[ret].flags & RSRCDBMGR_FLAG_NAME)) {
			list[ret].name = "";
		}
	}

	ret = _rsrcdbmgr_pack(&request, sizeof(request), offsetof(struct _rsrc_cmd, nbytes),
						   list, sizeof(*list), offsetof(rsrc_request_t, name), count,
						   list, count * sizeof(*list));
	return ret;
}

__SRCVERSION("rsrcdbmgr_attach.c $Rev: 153052 $");
