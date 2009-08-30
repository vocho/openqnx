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
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#include <sys/iomsg.h>
#include <sys/resmgr.h>
#include <sys/mount.h>
#include "resmgr.h"

int _resmgr_mount_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, struct link *link, io_mount_extra_t *extra,
			              int	(*func)(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, void *handle, void *extra)) {
	struct binding				*binding = NULL;
	int							status = EOK;
	io_mount_extra_t			*newextra;

	if ( msg->link.connect.extra_type == _IO_CONNECT_EXTRA_MOUNT_OCB) {
		// Lookup passed in handle to send an ocb if required
		if((binding = (struct binding *)_resmgr_handle(&extra->extra.cl.info, 0, _RESMGR_HANDLE_FIND_LOCK)) == (void *)-1) {
			return ENXIO;
		}

		// Make sure they are from the same process
		/*
		if((status = _resmgr_access(ctp, &extra->node.info, 0, 0, 0, 0)) != EOK) {
			return status;
		}
		*/
	}
	

	//We need to find a faster/more elegant way to make sure that we have the whole 
	//sized buffer to slurp in. For now, just allocate a new buffer 
	if (!(newextra = (io_mount_extra_t *)malloc(extra->nbytes))) {
		if(binding) _resmgr_handle(&extra->extra.cl.info, 0, _RESMGR_HANDLE_UNLOCK);
		return(ENOMEM);
	}

	//Copy the structure following the message (extra + data) into our allocated buffer.
	if ((status = MsgRead(ctp->rcvid, newextra, extra->nbytes, (char *)extra - (char *)msg)) != extra->nbytes) {
		if(binding) _resmgr_handle(&extra->extra.cl.info, 0, _RESMGR_HANDLE_UNLOCK);
		free(newextra);
		return(errno);
	}

	//Now that we have the buffer, set the pointers to the data, type and special device string
	memset(&newextra->extra, 0, sizeof(newextra->extra));

	if (binding) {
		newextra->extra.srv.ocb = binding->ocb;
	}

	if (newextra->datalen) {
		newextra->extra.srv.data = (void *)(newextra +1);
	}

	newextra->extra.srv.type = 
	newextra->extra.srv.special = (char *)(newextra + 1) + newextra->datalen;

	if ((status = ((newextra->nbytes - sizeof(io_mount_extra_t)) 
						 - newextra->datalen) 
						 - (strlen(newextra->extra.srv.type) +1)) > 0) {
		newextra->extra.srv.special +=  strlen(newextra->extra.srv.type) + 1;
	}
	else {
		newextra->extra.srv.special = NULL;
	}

	//Actually perform the mount callout
	status = func(ctp, msg, link->handle, newextra);

	free(newextra);
	if(binding)_resmgr_handle(&extra->extra.cl.info, 0, _RESMGR_HANDLE_UNLOCK);

	return(status);
}

__SRCVERSION("_resmgr_mount_handler.c $Rev: 153052 $");
