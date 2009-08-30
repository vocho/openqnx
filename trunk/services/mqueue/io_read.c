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


int
io_read(resmgr_context_t *ctp, io_read_t *msg, struct ocb *ocb) {
	MQDEV					*dev = ocb->ocb.attr;
	MQMSG					*mp;
	static MQMSG			dummy;
	MQWAIT					*wp;
	int						nonblock, status, n, rcvid;

	// Is queue open for read?
	if((status = iofunc_read_verify(ctp, msg, &ocb->ocb, &nonblock)) != EOK) {
		return status;
	}

	// If an xtype is specified make sure it is a mqueue.
	if((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE && (msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_MQUEUE) {
		return ENOSYS;
	}

	// Is the msg buffer too small for the queue?
	if(msg->i.nbytes < dev->mq_attr.mq_msgsize) {
		return EMSGSIZE;
	}

	// Are there waiting msgs.
	if(dev->mq_attr.mq_curmsgs == 0) {
		if(nonblock & O_NONBLOCK) {
			return EAGAIN;
		}

		if((wp = MemchunkMalloc(memchunk, sizeof(*wp))) == NULL) {
			return ENOMEM;
		}

		wp->rcvid = ctp->rcvid;
		wp->scoid = ctp->info.scoid;
		wp->coid = ctp->info.coid;
		wp->priority = 0;	// Must get real priority from ctp->info
		wp->xtype = msg->i.xtype;
		LINK_PRI_CLIENT(&dev->waiting_read, wp);
		++dev->mq_attr.mq_recvwait;

		return _RESMGR_NOREPLY;
	}

	// Reply with the data
	mp = (dev->mq_attr.mq_flags & MQ_SEMAPHORE) ? &dummy : dev->waiting_msg[0];
	if(mp->nbytes) {
		dev->attr.flags |= (IOFUNC_ATTR_ATIME | IOFUNC_ATTR_DIRTY_TIME);
	}
	resmgr_endian_context(ctp, _IO_READ, S_IFNAM, msg->i.xtype);
	_IO_SET_READ_NBYTES(ctp, mp->nbytes);
	if((msg->i.xtype & _IO_XTYPE_MASK) == _IO_XTYPE_MQUEUE) {
		uint32_t		prio;

		prio = mp->priority;
		SETIOV(&ctp->iov[0], &prio, sizeof(prio));
		SETIOV(&ctp->iov[1], mp->data, mp->nbytes);
		if(resmgr_msgreplyv(ctp, ctp->iov, 2) == -1) {
			return errno;
		}
	} else {
		SETIOV(&ctp->iov[0], mp->data, mp->nbytes);
		if(resmgr_msgreplyv(ctp, ctp->iov, 1) == -1) {
			return errno;
		}
	}

	// Remove the msg
	if(mp != &dummy) {
		if ((dev->waiting_msg[0] = mp->next) == NULL)
			dev->waiting_msg[1] = NULL;
		MemchunkFree(memchunk, mp);
	}
	--dev->mq_attr.mq_curmsgs;

	// Keep stat info up-to-date. We overload st_size to be messages waiting.
	dev->attr.nbytes = dev->mq_attr.mq_curmsgs;

	// Since we removed a msg we may need to wake someone waiting for a msg.
	if(wp = dev->waiting_write) {

		// Unlink and free wait entry
		rcvid = wp->rcvid;
		dev->waiting_write = wp->next;
		MemchunkFree(memchunk, wp);
		--dev->mq_attr.mq_sendwait;

		// Process the message
		resmgr_msg_again(ctp, rcvid);
	}

	if((n = dev->mq_attr.mq_maxmsg - dev->mq_attr.mq_curmsgs) != 0
			&& IOFUNC_NOTIFY_OUTPUT_CHECK(dev->notify, n)) {
		iofunc_notify_trigger(dev->notify, n, IOFUNC_NOTIFY_OUTPUT);
	}

	return _RESMGR_NOREPLY;
}

__SRCVERSION("io_read.c $Rev: 153052 $");
