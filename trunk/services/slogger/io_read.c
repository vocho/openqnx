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
io_read(resmgr_context_t *ctp, io_read_t *msg, iofunc_ocb_t *ocb) {
	int					 status;
	int					 nonblock;
	int					 nparts;
	int					 room;
	unsigned			 n;
	unsigned			 cnt;
	int					*ptr, *ptrend;
	struct slogdev		*trp;

	// Is device open for read?
	if((status = iofunc_read_verify(ctp, msg, ocb, &nonblock)) != EOK)
		return status;
	nonblock &= O_NONBLOCK;

	// No special xtypes
	if((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
		return EINVAL;

	// Reads must be at least _SLOG_HDRINTS (3) and a multiple of an int.
	if((n = msg->i.nbytes/sizeof(int)) < _SLOG_HDRINTS)
		return(EINVAL);
	if(msg->i.nbytes & (sizeof(int) - 1))
		return(EINVAL);

	trp = (struct slogdev *) ocb->attr;

	// First time in after an open ptr will be NULL.
	ptr = OCBGET(ocb);
	if(ptr == NULL)
		OCBGET(ocb) = ptr = trp->get;

	// If there are no events block.
	if(ptr == trp->put) {

		if(nonblock)
			return(EAGAIN);

		if(wait_add(trp, ctp->rcvid, ctp->info.priority) != 0)
			return(EAGAIN);
			
		return(_RESMGR_NOREPLY);
	}

	// Calculate the number of ints we can transfer.
	// To keep the clients life easy we never return a partial event.
	for(cnt = 0, ptrend = ptr ; ptrend != trp->put ;) {
		int i;

		i = _SLOG_GETCOUNT(*ptrend) + _SLOG_HDRINTS;
		if(i + cnt >= n)
			break;

		cnt += i;
		ptrend += i;
		if(ptrend >= trp->end)
			ptrend = trp->beg + (ptrend - trp->end);
	}

	// If the users buffer was to small let him know.
	if(cnt == 0)
		return(E2BIG);

	// Really used for debugging
	if(Verbose >= 2) printf("Get %5d %5d\n", ptr - trp->beg, cnt);

	// How many ints can we get without wrapping.
	room = trp->end - ptr;
	if(cnt > room) {
		// Wrap around and send data in 2 parts.
		SETIOV(ctp->iov + 0, ptr, room*sizeof(int));
		SETIOV(ctp->iov + 1, trp->beg, (cnt - room)*sizeof(int));
		nparts = 2;

		ptr = &trp->beg[cnt - room];
	} else {
		// No wrap around, send data in only 1 part.
		SETIOV(ctp->iov + 0, ptr, cnt*sizeof(int));
		nparts = 1;

		if(room == cnt)
			ptr = trp->beg;
		else
			ptr += cnt;
	}

	// Update position and access time.
	OCBGET(ocb) = ptr;
	ocb->attr->flags |= (IOFUNC_ATTR_ATIME | IOFUNC_ATTR_DIRTY_TIME);

	// Reply with the data.
	if(MsgReplyv(ctp->rcvid, cnt*sizeof(int), &ctp->iov[0], nparts) == -1)
		return(errno);

	return(_RESMGR_NOREPLY);
}

/*
PR26878
Waiting requests are inserted in priority order. 
*/
int
wait_add(struct slogdev *trp, int rcvid, int priority) {
	struct waiting *new_wap;

	// Get a new wait entry.
	if((new_wap = malloc(sizeof(*new_wap))) == NULL)
		return(-1);

	// Add the entry to the wait queue.
	new_wap->rcvid = rcvid;
	new_wap->priority = priority;
	
	// Put higher priority clients first
	if((trp->waiting==NULL) || (priority >= trp->waiting->priority))
	{
		new_wap->next = trp->waiting;
		trp->waiting = new_wap;
	}
	else
	{
		struct waiting	*wap = trp->waiting;

		for(; (wap->next != NULL) && (priority < wap->next->priority); wap=wap->next);

		new_wap->next = wap->next;
		wap->next = new_wap;
	}
	
	return(0);
	}

__SRCVERSION("io_read.c $Rev: 153052 $");
