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

#define _SLOG_MINSIZE (3 * sizeof(int))

static int
_io_write_log(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb, unsigned cnt, int txt) {
	unsigned			 n;
	int					*ptr;
	int					*ptr2;
	struct slogdev		*trp;
	struct waiting		*wap;
	struct timeval    tval;
	int msecs;
	int txt_hdr[_SLOG_HDRINTS] = {_SLOG_TEXTBIT, 0,0};

	/*
	 * The /dev/console entrypoint doesn't add the HDRINTS, so if this
	 * is a /dev/console entry, we'll have to add the space for the headers
	 * here.
	 */
  	if (txt)
		cnt += _SLOG_HDRINTS;

	// If no room we remove events to make room. This is the normal case
	// after we have been running for awhile.
	trp = (struct slogdev *) ocb->attr;
	while((NumInts - trp->cnt) < cnt) {
		n = _SLOG_GETCOUNT(*trp->get) + _SLOG_HDRINTS;
		check_overrun(trp, trp->get, n);
		trp->cnt -= n;
		trp->get += n;
		if(trp->get >= trp->end) {
			trp->get = trp->beg + (trp->get - trp->end);
		}
	}

	// Patch in the size of the event and the time.
	if(txt){
		ptr = txt_hdr;
		ptr2 = (int *)(sizeof(msg->i) + (char *)&msg->i);
	}
	else{
		ptr = (int *)(sizeof(msg->i) + (char *)&msg->i);
		ptr2 = ptr + _SLOG_HDRINTS;
	}
  gettimeofday(&tval, NULL);
  msecs = tval.tv_usec/1000;
  ptr[0] = ((*ptr & (~0x00fffff0)) | (msecs << 4)) | ((cnt - _SLOG_HDRINTS) << 16);
  ptr[2] = tval.tv_sec;
	// Really used for debugging
	if(Verbose >= 3) printf("Add %5d %5d Put: %p, beg : %p end: %p\n", trp->put-trp->beg, cnt, trp->put,trp->beg,trp->end);

	/* Insert first the hdrints, and then the buffer */

	// Check for a wrap in the buffer and if so copy in two pieces
	if(trp->put + _SLOG_HDRINTS >= trp->end) {
		n = trp->end - trp->put;
		memcpy(trp->put, ptr, n*sizeof(int));
		memcpy(trp->beg, ptr + n, (_SLOG_HDRINTS - n)*sizeof(int));
		trp->put = trp->beg + _SLOG_HDRINTS - n;
	} else {
		memcpy(trp->put, ptr, _SLOG_HDRINTS * sizeof(int));
		trp->put += _SLOG_HDRINTS;
	}
	cnt -= _SLOG_HDRINTS;
	trp->cnt += _SLOG_HDRINTS;
	
	// Check for a wrap in the buffer and if so copy in two pieces
	if(trp->put + cnt >= trp->end) {
		n = trp->end - trp->put;
		memcpy(trp->put, ptr2, n*sizeof(int));
		memcpy(trp->beg, ptr2 + n, (cnt - n)*sizeof(int));
		trp->put = trp->beg + cnt - n;
	} else {
		memcpy(trp->put, ptr2, cnt * sizeof(int));
		trp->put += cnt;
	}
	trp->cnt += cnt;

	/* 
		PR 8029
		
		Technically it's possible for the put pointer to wrap around and a log to fit exactly into
		the remaining slot, causing put == get.  This causes a false positive on the read side, which
		uses the condition to trigger "empty buffer" and other calculations.  These other calculations
		preclude the ability to use trp->cnt as the only flag for empty buffer.  So we'll ensure that
		there is always a non-zero amount of room between put and get, except at start (the empty buffer).

	*/
	
	/* if we've hit this PR 8029 special case, remove an extra entry */
	if (trp->put == trp->get) {
		n = _SLOG_GETCOUNT(*trp->get) + _SLOG_HDRINTS;
		check_overrun(trp,trp->get, n);
		trp->cnt -=n;
		trp->get +=n;
		if (trp->get >= trp->end) {
			trp->get = trp->beg + (trp->get-trp->end);
		}
	}
	
	/*
	PR 26878
	To avoid priority inversion 'readers' waiting list has to be in order of decreased priority.
	Function wait_add()@io_read.c provides that .
	*/
	// Wakeup any readers waiting for an event.
	while((wap = trp->waiting)) {
		trp->waiting = wap->next;
		ctp->rcvid = wap->rcvid;
		free(wap);
		if(resmgr_msg_again(ctp, ctp->rcvid)==-1)
		{
			if(Verbose>=3)
				printf("resmgr_msg_again failed\n");
		}
	}

	return(EOK);
}



int
io_write(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb) {
	int					 status;
	int					 nonblock;
	unsigned			 cnt;

	// Is device open for write?
	if((status = iofunc_write_verify(ctp, msg, ocb, &nonblock)) != EOK)
		return status;

	// No special xtypes
	if((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
		return(EINVAL);

	// Writes must be between 3 and 255 ints
	msg->i.nbytes = (msg->i.nbytes + (sizeof(int) - 1)) & ~(sizeof(int) - 1);
	if(msg->i.nbytes < _SLOG_MINSIZE || msg->i.nbytes > _SLOG_MAXSIZE)
		return(EINVAL);

	cnt = msg->i.nbytes/sizeof(int);

	/*
	PR 26878
	To avoid priority inversion reply to 'writer' immediately.
	*/
	MsgReply(ctp->rcvid,  msg->i.nbytes, NULL, 0);

	_io_write_log(ctp, msg, ocb, cnt, 0);


	return(_RESMGR_NOREPLY);
}


void check_overrun(struct slogdev *trp, int *ptr, int cnt) {
	struct ocbs		*list;
	IOFUNC_OCB_T	*ocb;
	int				*get;

	if(cnt == 0)
		return;

	for(list = trp->ocbs ; list ; list = list->next) {
		if((ocb = list->ocb) && (get = OCBGET(ocb))) {
			if (ptr == NULL && cnt == -1) {
				if(Verbose >= 2) printf("Reset blocked reader\n");
				OCBGET(ocb) = NULL;
			}
			else if(get == ptr && ptr != trp->put) {
				if(Verbose >= 1) printf("Overrun %d %d\n", get-trp->beg, cnt);
				get += cnt;
				if(get >= trp->end)
					get = trp->beg + (get - trp->end);
				OCBGET(ocb) = get;
			}
		}
	}
}


/*
 * io_console_write handles IO_WRITE messages to /dev/console.  These messages
 * get put into the slog buffer without any timestamp information.
 */

int
io_console_write(resmgr_context_t *ctp, io_write_t *msg, iofunc_ocb_t *ocb) {
	int		n;
	int		off;
	int		len;
	int		ret;
	int		cnt;
	int		status;
	int		nonblock;
	char	*cptr;

	ret = EOK;
	cptr = (char *) (sizeof(msg->i) + (char *)&msg->i);
	off = 0;
	len = msg->i.nbytes;
	
	// Is device open for write?
	if((status = iofunc_write_verify(ctp, msg, ocb, &nonblock)) != EOK)
		return status;

	// No special xtypes
	if((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
		return(EINVAL);
	
	while(off < len) {
		
		/* Limit write to max payload, NULL terminate the string,
		 * and figure out how many ints we're writing.
		 */
		n = min(len - off,  _SLOG_MAXSIZE - _SLOG_HDRINTS*sizeof(int) - 1);
		cptr[n] = '\0';
		cnt = n/sizeof(int) +1;

		/* put it in the log */
		ret = _io_write_log(ctp, msg, ocb, cnt, 1);
		if(ret != EOK)
			break;
	
		off += n;

		//skip the MsgRead() if we're done.
		if (off == len)
			break;
		
		ret = MsgRead(ctp->rcvid, cptr, n, off + sizeof(msg->i));
		if(ret <= 0) 
			break;
	}

	_IO_SET_WRITE_NBYTES(ctp, off);
	return(ret);
}

__SRCVERSION("io_write.c $Rev: 204636 $");
