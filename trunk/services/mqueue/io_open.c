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


#ifndef PATH_MAX
	#define PATH_MAX	_POSIX_PATH_MAX
#endif

static int
create_device(char *name, enum _file_type type, MQDEV *dev) {
	char	 path[PATH_MAX];
	unsigned	*count, limit;
	int			id;

	if (type == _FTYPE_MQUEUE)
		count = &num_mq, limit = max_num_mq;
	else
		count = &num_sem, limit = max_num_sem;
	if (*count >= limit)
		errno = ENFILE, id = -1;
	else if ((id = resmgr_attach(dpp, NULL, strcat(strcpy(path, "/"), name), type, 0, &mq_connect_funcs, &mq_io_funcs, dev)) != -1)
		++*count;
	return(id);
}

static MQDEV *
check_duplicate(char *name, MQDEV *head) {
	while(head != NULL && strcmp(name, head->name)) {
		head = head->link;
	}
	return head;
}

int
io_open(resmgr_context_t *ctp, io_open_t *msg, MQDEV *dev, void *extra) {
	struct mq_attr	 *mqp = extra;
	uint32_t		 *smp = extra;
	iofunc_attr_t	 *attr = &dev->attr;
	MQDEV			**head;
	struct mq_attr	  mq_attr;
	int				  status;
	dev_t			  rdev;

	if(S_ISDIR(dev->attr.mode)) {
		// Open on a new/non-existent queue.
		if((msg->connect.ioflag & O_CREAT) == 0) {
			return ENOENT;
		}

		// It must have a file_type of _FTYPE_MQUEUE or _FTYPE_SEM
		memset(&mq_attr, 0, sizeof(mq_attr));
		switch(msg->connect.file_type) {
		case _FTYPE_MQUEUE:
			rdev = S_INMQ;
			head = &mq_dir_attr.link;
			if(msg->connect.extra_type == _IO_CONNECT_EXTRA_MQUEUE) {
				if (msg->connect.extra_len != sizeof(struct mq_attr))
					return(ENOSYS);
				if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
					ENDIAN_SWAP32(&mqp->mq_maxmsg);
					ENDIAN_SWAP32(&mqp->mq_msgsize);
				}
				if((mq_attr.mq_maxmsg = mqp->mq_maxmsg) <= 0  ||
				   (mq_attr.mq_msgsize = mqp->mq_msgsize) <= 0) {
					return EINVAL;
				}
			} else {
				mq_attr.mq_maxmsg = 1024;
				mq_attr.mq_msgsize = 4096;
			}
			break;

		case _FTYPE_SEM:
			rdev = S_INSEM;
			head = &sem_dir_attr.link;
			mq_attr.mq_maxmsg = _POSIX_SEM_VALUE_MAX;
			mq_attr.mq_flags = MQ_SEMAPHORE;
			if(msg->connect.extra_type == _IO_CONNECT_EXTRA_SEM) {
				if (msg->connect.extra_len != sizeof(uint32_t))
					return(ENOSYS);
				if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
					ENDIAN_SWAP32(smp);
				}
				mq_attr.mq_curmsgs = *smp;
			}
			break;

		default:
			return ENOSYS;
		}

		// Check for O_CREAT race condition (PR-11060)
		if ((dev = check_duplicate(msg->connect.path, *head)) != NULL) {
			// Re-target open to the already created device.
			ctp->id = dev->id;
			goto race;		// In case non-trivial open verification code
		}

		// Get a device entry and the input/output buffers for it.
		if((dev = MemchunkCalloc(memchunk, 1, sizeof(*dev) + msg->connect.path_len - sizeof(char))) == NULL) {
			return ENOSPC;
		}
			
		msg->connect.mode = (msg->connect.mode & ~S_IFMT) | S_IFNAM;
		if((status = iofunc_open(ctp, msg, &dev->attr, attr, 0)) != EOK) {
			MemchunkFree(memchunk, dev);
			return status;
		}

		dev->mq_attr = mq_attr;
		dev->attr.rdev = rdev;
		IOFUNC_NOTIFY_INIT(dev->notify);

		// Add the new queue to the pathname space
		if((dev->id = create_device(msg->connect.path, msg->connect.file_type, dev)) == -1) {
			if ((status = errno) == EMFILE) { //We have created too many connections, this is the system limit
				status = ENFILE;			  //Tell the client the system is full.
			}
			MemchunkFree(memchunk, dev);
			return status;
		}
		strcpy(dev->name, msg->connect.path);
		dev->link = *head, *head = dev;

		// Re-target open to the newly created device.
		ctp->id = dev->id;
	} else {
race:
		// Open on an existing queue.
		if((status = iofunc_open(ctp, msg, &dev->attr, 0, 0)) != EOK) {
			return status;
		}
	}

	// Attach the ocb to the device
	if((status = iofunc_ocb_attach(ctp, msg, NULL, &dev->attr, NULL)) == -1) {
		return status;
	}

	return EOK;
}

__SRCVERSION("io_open.c $Rev: 153052 $");
