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




#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/iomsg.h>
#include <sys/uio.h>

/*
 * This one funcion does notify and/or read
 * 
 *	fd			The fd to send to.
 *	action		Notify action (if 0 and event==NULL don't do notify)
 *	flags		Notify flags
 *	event		Notify event (if 0 and event==NULL don't do notify) 
 *	iov			Read iov (Start at 2nd element, 1st element used my function)
 *	nparts		Number of parts (including 1st unused part)
 *	xtype		Extra type passed to read	
 *	xdata		Extra data returned from read
 *	xdatalen	Extra data returned from read
 *	nbytes		Number of bytes to read (If 0, calculate from iov)
 */
ssize_t _notifyreadxv(int fd, int action, int flags, const struct sigevent *event, iov_t *iov, int nparts, unsigned xtype, void *xdata, size_t xdatalen, size_t nbytes) {
	int							size;
	void						*ptr;
	union {
		struct notify_read {
			struct _io_notify			notify;
			struct _io_read				read;
		}							i;
		union {
			struct _io_notify_reply		notify;
		}							o;
	}							msg;

	// Setup notify
	msg.i.notify.type = _IO_NOTIFY;
	msg.i.notify.combine_len = sizeof msg.i.notify;
	msg.i.notify.action = action;
	msg.i.notify.flags = flags;
	if(event) {
		msg.i.notify.event = *event;
	}

	if(iov) {
		// There must be at least 1 part...
		if(nparts < 1 || (int)xdatalen < 0) {
			errno = EINVAL;
			return -1;
		}

		// Do the read setup
		ptr = &msg.i.read;
		size = sizeof msg.i.read;
		if(event || (action & _NOTIFY_ACTION_POLL)) {
			// Do notify, then read
			ptr = &msg.i;
			size = offsetof(struct notify_read, read) + sizeof msg.i.read;
			msg.i.notify.combine_len = offsetof(struct notify_read, read) | _IO_COMBINE_FLAG;
		}

		// Setup read
		msg.i.read.type = _IO_READ;
		msg.i.read.combine_len = sizeof msg.i.read;
		msg.i.read.xtype = xtype;
		msg.i.read.zero = 0;

		// If nbytes not passed in, calculate it.
		if((msg.i.read.nbytes = nbytes) == 0) {
			int						i;

			for(i = 1; i < nparts; i++) {
				msg.i.read.nbytes += GETIOVLEN(&iov[i]);
			}
		}
		SETIOV(iov + 0, xdata, xdatalen);

		return MsgSendv(fd, ptr, -size, iov, nparts);
	}

	// Only do a notify
	if(MsgSend(fd, &msg.i.notify, sizeof msg.i.notify, &msg.o.notify, sizeof msg.o.notify) == -1) {
		return -1;
	}
	return msg.o.notify.flags;
}

ssize_t _readx(int fd, void *buff, size_t nbytes, unsigned xtype, void *xdata, size_t xdatalen) {
	iov_t			iov[2];

	SETIOV(iov + 1, buff, nbytes);
	return _notifyreadxv(fd, 0, 0, 0, iov, 2, xtype, xdata, xdatalen, nbytes);
}

ssize_t _readxv(int fd, iov_t *iov, int nparts, unsigned xtype, void *xdata, size_t xdatalen, size_t nbytes) {
	return _notifyreadxv(fd, 0, 0, 0, iov, nparts, xtype, xdata, xdatalen, nbytes);
}

ssize_t ionotifyread(int fd, int action, int flags, const struct sigevent *event, void *buff, size_t nbytes) {
	iov_t			iov[2];

	SETIOV(iov + 1, buff, nbytes);
	return _notifyreadxv(fd, action, flags, event, iov, 2, 0, 0, 0, nbytes);
}

ssize_t ionotifyreadx(int fd, int action, int flags, const struct sigevent *event, void *buff, size_t nbytes, unsigned xtype, void *xdata, size_t xdatalen) {
	iov_t			iov[2];

	SETIOV(iov + 1, buff, nbytes);
	return _notifyreadxv(fd, action, flags, event, iov, 2, xtype, xdata, xdatalen, nbytes);
}

__SRCVERSION("_notifyreadxv.c $Rev: 153052 $");
