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
io_write(resmgr_context_t *ctp, io_write_t *msg, struct ocb *ocb) {
	MQDEV		*dev = ocb->ocb.attr;
	MQMSG		*mp;
	MQWAIT		*wp;
	void		*data;
	unsigned	priority = 0;
	unsigned	client_prio;
	int			nonblock, status, nbytes, rcvid, preread;

	// Will be NULL if called from io_closeocb with a closemsg.
	if(msg != NULL) {
		// Is queue open for write?
		if((status = iofunc_write_verify(ctp, msg, &ocb->ocb, &nonblock)) != EOK) {
			return status;
		}

		// If an xtype is specified make sure it is an mqueue.
		if((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
			if((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_MQUEUE) {
				return ENOSYS;
			}
			priority = msg->i.xtype >> 16;
			if(priority < 0 || priority >= MQ_PRIO_MAX) {
				return EINVAL;
			}
		}

		// Is the msg too big for the queue?
		if(msg->i.nbytes > dev->mq_attr.mq_msgsize) {
			return EMSGSIZE;
		}

		client_prio = ctp->info.priority;

		// If there is not enough room for another msg we must block.
		if(dev->mq_attr.mq_curmsgs >= dev->mq_attr.mq_maxmsg) {
			if(nonblock & O_NONBLOCK) {
				return EAGAIN;
			}

			if((wp = MemchunkMalloc(memchunk, sizeof(*wp))) == NULL) {
				return ENOSPC;
			}

			wp->rcvid = ctp->rcvid;
			wp->scoid = ctp->info.scoid;
			wp->coid = ctp->info.coid;
			wp->priority = client_prio;
			wp->xtype = msg->i.xtype;
			LINK_PRI_CLIENT(&dev->waiting_write, wp);
			++dev->mq_attr.mq_sendwait;

			return _RESMGR_NOREPLY;
		}
		data = (char *)msg + sizeof(msg->i), nbytes = msg->i.nbytes, preread = ctp->size - sizeof(msg->i);
	} else {
		data = ocb->closemsg->data, nbytes = preread = ocb->closemsg->nbytes;
	}

	if((dev->mq_attr.mq_flags & MQ_SEMAPHORE) == 0) {

		// Try fast process-to-process flip (without queuing msg)
		if((wp = dev->waiting_read) != NULL && (msg == NULL || ctp->size >= sizeof(msg->i) + nbytes)) {
			if(nbytes) {
				dev->attr.flags |= (IOFUNC_ATTR_CTIME | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_ATIME | IOFUNC_ATTR_DIRTY_TIME);
			}
			resmgr_endian_context(ctp, _IO_READ, S_IFNAM, wp->xtype);
			_IO_SET_READ_NBYTES(ctp, nbytes);
			rcvid = ctp->rcvid, ctp->rcvid = wp->rcvid;
			if((wp->xtype & _IO_XTYPE_MASK) == _IO_XTYPE_MQUEUE) {
			uint32_t	prio = priority;
				SETIOV(&ctp->iov[0], &prio, sizeof(prio));
				SETIOV(&ctp->iov[1], data, nbytes);
				resmgr_msgreplyv(ctp, ctp->iov, 2);
			} else {
				SETIOV(&ctp->iov[0], data, nbytes);
				resmgr_msgreplyv(ctp, ctp->iov, 1);
			}
			ctp->rcvid = rcvid;
			dev->waiting_read = wp->next;
			MemchunkFree(memchunk, wp);
			--dev->mq_attr.mq_recvwait;
			return EOK;
		}

		// Get a msg buffer.
		if((mp = MemchunkMalloc(memchunk, MQ_DATAOFF + nbytes)) == NULL) {
			return EAGAIN;
		}

		mp->next = NULL;
		mp->priority = priority;
		if(mp->nbytes = nbytes) {
			dev->attr.flags |= (IOFUNC_ATTR_CTIME | IOFUNC_ATTR_MTIME | IOFUNC_ATTR_DIRTY_TIME);
		}

		// Save/Get the data into msg buffer
		if(msg == NULL || preread >= nbytes) {
			memcpy(mp->data, data, nbytes);
		} else {
			memcpy(&mp->data[0], data, preread);
			if(MsgRead(ctp->rcvid, &mp->data[preread], nbytes - preread, ctp->size) != nbytes - preread) {
				MemchunkFree(memchunk, mp);
				return EIO;
			}
		}

		// Queue the msg
		LINK_PRI_MSG(dev->waiting_msg, mp);
	}

	// Reply with status
	if(msg != NULL)
		MsgError(ctp->rcvid, EOK);

	++dev->mq_attr.mq_curmsgs;

	// Keep stat info up-to-date. We overload st_size to be messages waiting.
	dev->attr.nbytes = dev->mq_attr.mq_curmsgs;

	// Since we added a msg we may need to wake someone waiting for a msg.
	if(wp = dev->waiting_read) {

		// Unlink and free wait entry
		rcvid = wp->rcvid;
		dev->waiting_read = wp->next;
		MemchunkFree(memchunk, wp);
		--dev->mq_attr.mq_recvwait;

		// Process the message
		resmgr_msg_again(ctp, rcvid);
	} else {
		// Check for notify conditions
		if (IOFUNC_NOTIFY_INPUT_CHECK(dev->notify, dev->mq_attr.mq_curmsgs, dev->mq_attr.mq_curmsgs == 1)) {
			iofunc_notify_trigger(dev->notify, dev->mq_attr.mq_curmsgs, IOFUNC_NOTIFY_INPUT);
		}
	}

	return _RESMGR_NOREPLY;
}

__SRCVERSION("io_write.c $Rev: 153052 $");
