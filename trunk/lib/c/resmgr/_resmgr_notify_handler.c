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
#include <atomic.h>
#include <gulliver.h>
#include <sys/resmgr.h>
#include <sys/poll.h>
#include "resmgr.h"

static void ENDIAN_SWAPNOTIFY(struct _io_notify *notify)
{
	ENDIAN_SWAP32(&notify->mgr[0]);
	ENDIAN_SWAP32(&notify->mgr[1]);
	ENDIAN_SWAP32(&notify->flags_extra_mask);
	ENDIAN_SWAP32(&notify->flags_exten);
	ENDIAN_SWAP32(&notify->nfds);
	ENDIAN_SWAP32(&notify->fd_first);
	ENDIAN_SWAP32(&notify->nfds_ready);
	ENDIAN_SWAP64(&notify->timo);
}
static void ENDIAN_SWAPFDS(struct pollfd *fds, int n)
{
	while (n > 0) {
		ENDIAN_SWAP32(&fds->fd);
		ENDIAN_SWAP16(&fds->events);
		ENDIAN_SWAP16(&fds->revents);
		--n;
		++fds;
	}
}

int
_resmgr_notify_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg_in)
{
	struct binding *binding;
	int cando_upto, cando_each, todo, trailers;
	int offset_fd, offset_cur, offset_last, found, ret, first, no_gap;
	struct pollfd *fds;
	io_notify_t *msg;


	_Uint16t combine_len;
	_Int32t	action;

	offset_last = 0;

	msg = &msg_in->notify;
	if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) ENDIAN_SWAPNOTIFY(&msg->i);

	combine_len = msg->i.combine_len;
	action      = msg->i.action;

	/* First callout into lower layer */
	msg->i.flags_exten |= _NOTIFY_FLAGS_EXTEN_FIRST;

	offset_fd = msg->i.fd_first;
	cando_each = (ctp->msg_max_size - (ctp->offset + sizeof(msg->i))) / sizeof(struct pollfd);
	todo = msg->i.nfds - offset_fd;
	
	cando_upto = cando_each;
	offset_cur = offset_fd;
	fds = (struct pollfd *)(&msg->i + 1);
	if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) ENDIAN_SWAPFDS(fds, cando_each);

	found = 0;
	first = 1;
	no_gap = 1;
	trailers = 0;

	do {
		if (offset_fd >= cando_upto) {
			if (found) {
				if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) ENDIAN_SWAPFDS(fds, cando_each);
				(void)MsgWrite(ctp->rcvid, fds, cando_each * sizeof(struct pollfd),
				    sizeof(msg->o) + offset_last * sizeof(struct pollfd));
			}

			if ((ret = MsgRead_r(ctp->rcvid, fds, cando_each * sizeof(struct pollfd),
			    sizeof(msg->i) + offset_fd * sizeof(struct pollfd))) < 0) {
				return -ret;
			}
			if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) ENDIAN_SWAPFDS(fds, cando_each);

			offset_last = offset_fd;
			found = 0;

			cando_upto += ret / (int)sizeof(struct pollfd);
			offset_cur = 0;
		}


		for (; todo > 0 && offset_fd < cando_upto; offset_cur++, offset_fd++, todo--) {
			if ((fds[offset_cur].events & POLLRESERVED) && fds[offset_cur].fd >= 0) {
				ctp->info.coid = fds[offset_cur].fd;

				if ((binding = (struct binding *)_resmgr_handle(&ctp->info, 0,
				    _RESMGR_HANDLE_FIND_LOCK)) == (void *)-1) {
					if (first) {
						/*
						 * The first one should be the coid
						 * the message came in on.  Always
						 * knock this one down so the client
						 * doesn't retry in a loop.
						 */
						fds[offset_cur].events &= ~POLLRESERVED;
						fds[offset_cur].revents |= POLLNVAL;
						msg->o.fd_first++;
						found = 1;
						trailers++;
					}
					else {
						no_gap = 0;
						trailers = 0;
					}
				}
				else {
					fds[offset_cur].events &= ~POLLRESERVED;

					msg->i.flags = (unsigned short)fds[offset_cur].events |
					    ((unsigned short)fds[offset_cur].events << 28);
					msg->i.flags |= _NOTIFY_COND_EXTEN | msg->i.flags_extra_mask;

					msg->i.combine_len = combine_len;
					msg->i.action      = action;

					if ((ret = _resmgr_io_handler(ctp, msg_in, binding)) > 0)
						return ret;

					msg->i.flags_exten &= ~_NOTIFY_FLAGS_EXTEN_FIRST;
					if (msg->o.flags) {
						msg->o.flags |= ((uint32_t)msg->o.flags &~ _NOTIFY_COND_EXTEN) >> 28;
						fds[offset_cur].revents = msg->o.flags;
						msg->o.nfds_ready++;
						action = _NOTIFY_ACTION_POLL; /* Don't arm any more */
					}
					msg->o.fd_first += no_gap;
					found = 1;
					trailers++;
				}
				first = 0;
			}
			else {
				fds[offset_cur].events &= ~POLLRESERVED;
				msg->o.fd_first += no_gap;
				trailers++;
			}

			msg->i.event.sigev_value.sival_int++;
			msg->i.event.sigev_value.sival_int &= _NOTIFY_DATA_MASK;
		}
	} while (todo > 0);

	msg->o.nfds -= trailers;

	if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) ENDIAN_SWAPFDS(fds, cando_each);
	if (offset_fd >= cando_each) {
		(void)MsgWrite(ctp->rcvid, fds, offset_cur * sizeof(struct pollfd),
		    sizeof(msg->o) + offset_last * sizeof(struct pollfd));
		ret = sizeof(msg->o);
	}
	else {
		ret = sizeof(msg->o) + offset_cur * sizeof(struct pollfd);
	}

	ctp->status = 0;
	if (ctp->info.flags & _NTO_MI_ENDIAN_DIFF) ENDIAN_SWAPNOTIFY(&msg->i);
	return _RESMGR_PTR(ctp, &msg->o, ret);
}

__SRCVERSION("_resmgr_notify_handler.c $Rev: 153052 $");
