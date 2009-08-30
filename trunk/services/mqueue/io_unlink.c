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





#include "externs.h"


void
delete_msgs(MQDEV *dev) {
	MQMSG	*mp, *link;

	for(mp = dev->waiting_msg[0]; mp != NULL; mp = link) {
		link = mp->next;
		MemchunkFree(memchunk, mp);
	}
	dev->waiting_msg[0] = dev->waiting_msg[1] = NULL;
}

int
io_unlink(resmgr_context_t *ctp, io_unlink_t *msg, MQDEV *dev, void *reserved) {
	MQDEV	**head;
	int		status;

	// You can't unlink the directory itself or a non-existant queue.
	if(S_ISDIR(dev->attr.mode)) {
		if(msg->connect.path[0]) {
			return ENOENT;
		} else {
			return EPERM;
		}
	}

#ifndef MQUEUE_1_THREAD
	iofunc_attr_lock(&dev->attr);
#endif

	status = iofunc_unlink(ctp, msg, &dev->attr, 0, 0);

#ifndef MQUEUE_1_THREAD
	iofunc_attr_unlock(&dev->attr);
#endif

	// Make sure unlink is allowed
	if(status != EOK) {
		return status;
	}

	if(dev->attr.rdev == S_INMQ) {
		--num_mq;
		head = &mq_dir_attr.link;
	} else {
		--num_sem;
		head = &sem_dir_attr.link;
	}
	while(*head != NULL) {
		if(*head == dev) {
			*head = dev->link;
			break;
		}
		else {
			head = &(*head)->link;
		}
	}

	// We always remove the device from the namespace so nobody can open it.
	if(dev->attr.nlink == 0 && dev->attr.count == 0) {
		resmgr_detach(dpp, dev->id, _RESMGR_DETACH_ALL);
		delete_msgs(dev);
		MemchunkFree(memchunk, dev);
	} else {
		// We can't destroy the device if anyone still has it open.
		resmgr_detach(dpp, dev->id, _RESMGR_DETACH_PATHNAME);
	}

	return EOK;
}

__SRCVERSION("io_unlink.c $Rev: 153052 $");
