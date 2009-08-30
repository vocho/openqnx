/*
 * $QNXLicenseC:
 * Copyright 2006, QNX Software Systems. All Rights Reserved.
 *
 * This source code may contain confidential information of QNX Software
 * Systems (QSS) and its licensors.  Any use, reproduction, modification,
 * disclosure, distribution or transfer of this software, or any software
 * that includes or is based upon any of this code, is prohibited unless
 * expressly authorized by QSS by written agreement.  For more information
 * (including whether this source code file has been published) please
 * email licensing@qnx.com. $
 */
#define RESMGR_OCB_T void
#define RESMGR_HANDLE_T void
#include <atomic.h>
#include <stdio.h>
#include <fcntl.h>
#include <devctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/dcmd_chr.h>
#include <sys/uio.h>
#include <sys/iofunc.h>
#include <sys/netmgr.h>

#include "dmgr_priv.h"

//#define DPRINT(P) printf P
#define DPRINT(P) 

static pid_t mypid;

static int di_bypass(resmgr_context_t *ctp, void *umsg, struct relay_ocb *ocb)
{
	struct relay_cb *rcb = ocb->rcb;
	int    slen, dlen, n;
	char   *rbuf = NULL;
	
	slen = ctp->info.srcmsglen;
	dlen = ctp->info.dstmsglen;

	if (max(slen, dlen) > rcb->bpbuf_size) {
		if ((rbuf = alloca(max(slen, dlen))) == NULL)
		  return errno;
	} else {
		rbuf = ocb->bpbuf;
	}

	if (slen > ctp->info.msglen) {
		if (MsgRead(ctp->rcvid, rbuf, slen, 0) == -1 ||
			(n = MsgSend(ocb->realfd, rbuf, slen, rbuf, dlen)) == -1)
		{
			return errno;
		}
	} else {
		if ((n = MsgSend(ocb->realfd, umsg, slen, rbuf, dlen)) == -1) {
			return errno;
		}
	}
	MsgReply(ctp->rcvid, n, rbuf, dlen);

	return _RESMGR_NOREPLY;
}

static int di_open(resmgr_context_t *ctp, io_open_t *msg, void *handle, void *extra)
{
	struct relay_ocb *o;
	struct relay_cb  *rcb = handle;
	int    realfd;
	extern resmgr_io_funcs_t di_io_funcs;

	if (ctp->id == rcb->resid) {
		realfd = open(rcb->devname, (msg->connect.ioflag - 1));
	} else {
		realfd = dup(rcb->rcoid);
	}

	if (realfd == -1 || (o = malloc(sizeof(*o))) == NULL)
	{
		return errno;
	}

	memset(o, 0, sizeof(*o));
	o->realfd = realfd;
	o->rcb = rcb;
	if ((o->bpbuf = malloc(rcb->bpbuf_size)) == NULL ||
		(resmgr_open_bind(ctp, o, &di_io_funcs) == -1))
	{
		int n = errno;
		free(o);
		close(realfd);
		return n;
	}

	pthread_mutex_lock(&rcb->mutex);
	if (ctp->id == rcb->resid) {
		o->next = rcb->owner;
		rcb->owner = o;
	} else {
		o->next = rcb->cloner;
		rcb->cloner = o;
	}
	pthread_mutex_unlock(&rcb->mutex);
	return EOK;
}

static int di_read(resmgr_context_t *ctp, io_read_t *msg, void *ocb)
{
	struct relay_ocb  *o = ocb;
	struct relay_cb *rcb = o->rcb;
	struct relay_cache *cap;
	struct wrec *rp;
	int n, toread;
	int off, recoff, nriov;
	iov_t  riov[2];
	uint8_t *cp;

restart:
	if (ctp->id == rcb->resid) {
		cap = &rcb->ccache;
	} else {
		cap = &rcb->ocache;
	}
	
	pthread_rwlock_rdlock(&cap->ca_rwlock);
	if ((rp = o->rp) == NULL) {
		rp = o->rp = cap->ca_rp;
		o->rec_off = 0;
	}
	
	/* if owner is reading, see if there is a cloner write in buffer */
	toread = msg->i.nbytes;
	off = 0;
	recoff = o->rec_off;
	
	while (toread && rp->next) {
		cp = (uint8_t *)((char *)rp + WRECSIZE + recoff);
		if (cp > cap->ca_end)
		  cp -= cap->ca_size;
		
		if (cp + rp->reclen - recoff < cap->ca_end) {
			SETIOV(riov + 0, cp, rp->reclen - recoff);
			nriov = 1;
		} else {
			SETIOV(riov + 0, cp, cap->ca_end - cp);
			SETIOV(riov + 1, cap->ca_buf, rp->reclen - riov->iov_len - recoff);
			nriov = 2;
		}

		n = MsgWritev(ctp->rcvid, riov, nriov, off);
		if (n == -1) {
			pthread_rwlock_unlock(&cap->ca_rwlock);
			return errno;
		}
		
		off += n;
		recoff += n;
		toread = toread > n ? toread - n : 0;

		if (recoff < rp->reclen) {
			o->rec_off = recoff;
			break;
		}
		
		/* move read point for this ocb */
		o->rp = rp->next;
		o->rec_off = recoff = 0;

		rp = o->rp;
	}
	pthread_rwlock_unlock(&cap->ca_rwlock);
	if (!off) {
		/* owner doing read, will be bypass, since we could be blocked,
		 * we unlcok the attr so cloners will have a chance.
		 */
		if (ctp->id == rcb->resid) {
			o->block_tid = pthread_self();
			iofunc_attr_unlock(&rcb->attr);
			n = di_bypass(ctp, msg, o);
			iofunc_attr_lock(&rcb->attr);
			o->block_tid = 0;
			/* if I was unblocked, pick the input from buffer */
			if (n == EINTR) {
				if (o->flag & OCB_FLAG_UNBLOCK) {
					MsgError(ctp->rcvid, EINTR);
					return _RESMGR_NOREPLY;
				} else {
					goto restart;
				}
			}
			return n;
		} else {
			o->rcvid = ctp->rcvid;
		}
	} else {
		MsgReply(ctp->rcvid, off, 0, 0);
	}
	return _RESMGR_NOREPLY;
}

static int distance(struct relay_cache *cap, void *from, void *to, iov_t *wiov, int *nwiov)
{
	int dist;
	
	if (from <= to || to == cap->ca_buf) {
		if (to == cap->ca_buf) 
		  dist = (uint32_t)cap->ca_buf + cap->ca_size - (uint32_t)from - WRECSIZE;
		else if (from == to)
		  dist = cap->ca_size - WRECSIZE;
		else
		  dist = to - from - WRECSIZE;
		if (nwiov) {
			SETIOV(wiov, from + WRECSIZE, dist);
			*nwiov = 1;
		}
	} else {
		dist = to - from + cap->ca_size  - WRECSIZE;
		if (nwiov) {
			SETIOV(wiov + 0, from + WRECSIZE, (uint32_t)cap->ca_buf + cap->ca_size - (uint32_t)from - WRECSIZE);
			SETIOV(wiov + 1, cap->ca_buf, (uint32_t)to - (uint32_t)cap->ca_buf);
			*nwiov = 2;
		}
	}
	return dist;
}

static int di_write(resmgr_context_t *ctp, io_write_t *msg, void *ocb)
{
	struct relay_ocb  *o = ocb, *ocb0;
	struct relay_cb *rcb = o->rcb;
	struct relay_cache *cap;
	int left, n, off, nwiov;
	struct wrec *rp, *wp;
	iov_t wiov[2];

	if (ctp->id == rcb->resid) {
		cap = &rcb->ocache;
	} else {
		/* if cloner can't write, just return */
		if (!rcb->cloner_key) {
			_IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);
			return EOK;
		}
		cap = &rcb->ccache;
	}
	
	pthread_rwlock_wrlock(&cap->ca_rwlock);
	off  = sizeof(*msg);
	left = msg->i.nbytes;

	wp = cap->ca_wp;
	rp = cap->ca_rp;

	/* if the client's write bigger then buffer size, invalidate all
	 * records in buffer; send all data to real server.
	 */
	if (left > cap->ca_size) {
		struct relay_ocb *ocb0;

		if (ctp->id == rcb->resid) {
			/* invalidate all cloners read pointer */
			pthread_mutex_lock(&rcb->mutex);
			for (ocb0 = rcb->cloner; ocb0; ocb0 = ocb0->next)
			  ocb0->rp = NULL;
			pthread_mutex_unlock(&rcb->mutex);
			
			/* write these data to real fd */
			while (left > cap->ca_size) {
				if ((n = MsgRead(ctp->rcvid, cap->ca_buf, cap->ca_size, off)) == -1)
				{
					pthread_rwlock_unlock(&cap->ca_rwlock);
					return errno;
				}
				write(o->realfd, cap->ca_buf, n);
				left -= n;
				off  += n;
			}
		} else {
			/* invalidate all owners read pointer */
			pthread_mutex_lock(&rcb->mutex);
			for (ocb0 = rcb->owner; ocb0; ocb0 = ocb0->next)
			  ocb0->rp = NULL;
			pthread_mutex_unlock(&rcb->mutex);
			left = left % cap->ca_size;
			off  = (left / cap->ca_size) * cap->ca_size;
		}
		rp = wp = (struct wrec *)cap->ca_buf;
		rp->next = NULL;
	}
		
	/* the rest part will be write into buffer, also send to real fd */
	if (left) {
		uint8_t *wp0;
		int dist0;
		
		/* first, find out where the new wp would land */
		wp0 = (char *)wp + WRECSIZE + left;
		wp0 = (char *)(((uint32_t)wp0 + sizeof(int) - 1) & ~(sizeof(int) - 1));
		
		if (wp0 >= cap->ca_end) {
			wp0 = wp0 - cap->ca_end + cap->ca_buf;
		} 
		
		if ((uint32_t)(cap->ca_end - wp0) < WRECSIZE) {
			wp0 = cap->ca_buf;
		}
		dist0 = distance(cap, wp, wp0, 0, 0) + WRECSIZE;
		
		/* second, move the cap->ca_rp if necessary */
		pthread_mutex_lock(&rcb->mutex);
		while (distance(cap, wp, rp, wiov, &nwiov) < dist0)
		{
			/* move one record, invalide certain cloners's read pointer */
			ocb0 = (ctp->id == rcb->resid) ? rcb->cloner : rcb->owner;
			for (; ocb0; ocb0 = ocb0->next)
			  if (ocb0->rp == rp)
				ocb0->rp = NULL;
			
			if (rp->next)
			  rp = rp->next;
		}
		pthread_mutex_unlock(&rcb->mutex);
		cap->ca_rp = rp;

		/* read data into obuf */
		if ((n = MsgReadv(ctp->rcvid, wiov, nwiov, off)) == -1)
		{
			perror("MsgReadv");
			pthread_rwlock_unlock(&cap->ca_rwlock);
			return errno;
		}

		/* adjust wiov to real size */
		if (nwiov == 1 || n < wiov[0].iov_len) {
			nwiov = 1;
			wiov[0].iov_len = n;
		} else {
			wiov[1].iov_len = n - wiov[0].iov_len;
		}
		
		/* owner's write also send to realfd */
		if (ctp->id == rcb->resid) {
			n = writev(o->realfd, wiov, nwiov);
		}

		left -= n;
		off += n;

		/* move write pointer */
		wp->next = (struct wrec *)wp0;
		wp->reclen = n;
		wp = (struct wrec *)wp0;

		pthread_mutex_lock(&rcb->mutex);
		if (ctp->id == rcb->resid) {
			/* if owner writing, pass it to cloner */
			for (ocb0 = rcb->cloner; ocb0; ocb0 = ocb0->next)
			{
				int ret;
				
				if (ocb0->rcvid) {
					if ((ret = MsgWritev(ocb0->rcvid, wiov, nwiov, 0)) == -1)
					{
						MsgError(ocb0->rcvid, errno);
						ocb0->rp = 0;
						ocb0->rec_off = 0;
					} else {
						if (ret == n) {
							ocb0->rp = wp;
						} else {
							ocb0->rec_off += ret;
						}
						MsgReply(ocb0->rcvid, ret, 0, 0);
					}
					ocb0->rcvid = 0;
				} else {
					iofunc_notify_trigger(ocb0->notify, n, IOFUNC_NOTIFY_INPUT);
				}
			}
		} else {
			/* if cloner writing, inject that to owner */
			for (ocb0 = rcb->owner; ocb0; ocb0 = ocb0->next)
			{
				if (ocb0->block_tid)
				{
					SignalKill(ND_LOCAL_NODE, mypid, ocb0->block_tid, SIGUSR1, 0, 0);
				} else {
					iofunc_notify_trigger(ocb0->notify, n, IOFUNC_NOTIFY_INPUT);
				}
			}
		}
		pthread_mutex_unlock(&rcb->mutex);
	}
	wp->next = 0;
	cap->ca_wp = wp;
	
	_IO_SET_WRITE_NBYTES(ctp, msg->i.nbytes);
	pthread_rwlock_unlock(&cap->ca_rwlock);

	return EOK;
}

static int di_closeocb(resmgr_context_t *ctp, void *reserved, void *ocb)
{
	struct relay_ocb  *o = ocb, *o0, **ocbp;
	struct relay_cb *rcb = o->rcb;
	
	pthread_mutex_lock(&rcb->mutex);
	if (ctp->id == rcb->resid) 
	  ocbp = &rcb->owner;
	else
	  ocbp = &rcb->cloner;
	
	for (o0 = *ocbp; o0 && o0 != ocb; ocbp = &o0->next, o0 = *ocbp);
	if (o0) {
		*ocbp = o0->next;
	}
	pthread_mutex_unlock(&rcb->mutex);

	iofunc_notify_remove(ctp, o->notify);
	close(o->realfd);
	free(o->bpbuf);
	free(o);
	
	/* is this device umounted? */
	if (!rcb->devname && !rcb->owner && !rcb->cloner) {
		free(rcb->ocache.ca_buf);
		free(rcb->ccache.ca_buf);
		free(rcb);
	}

	return EOK;
}

static int di_stat(resmgr_context_t *ctp, io_stat_t *msg, void *ocb)
{
	return di_bypass(ctp, msg, ocb);
}

static int di_notify(resmgr_context_t *ctp, io_notify_t *msg, void *ocb)
{
	struct relay_ocb  *o = ocb;
	struct relay_cb *rcb = o->rcb;
	struct relay_cache *cap;
	int trig = 0;
	extern int ncoid;
	
	if (ctp->id == rcb->resid) {
		cap = &rcb->ccache;
	} else {
		cap = &rcb->ocache;
		/* if cloner can't write, don't allow it set this notify */
		if (!rcb->cloner_key) {
			msg->i.flags &= ~_NOTIFY_COND_OUTPUT;
		} else {
			trig = _NOTIFY_COND_OUTPUT;
		}
	}
	
	if ((o->rp && o->rp->next) || (!o->rp && cap->ca_rp->next))
	  trig |= _NOTIFY_COND_INPUT;
	
	/* if a owner can't INPUT, we have to arm real device */
	if (trig == 0 && ctp->id == rcb->resid) {
		struct sigevent evt;
		struct _io_notify_reply msgo;
		
		memcpy(&evt, &msg->i.event, sizeof(evt));
		SIGEV_PULSE_INIT(&msg->i.event, ncoid, -1, SI_NOTIFY, rcb->index & 0xffff);
		if ((trig = MsgSend(o->realfd, msg, sizeof(msg->i), &msgo, sizeof(msgo))) == -1)
		{
			return errno;
		}
		trig = msgo.flags;
		memcpy(&msg->i.event, &evt, sizeof(evt));
	}
	
	return iofunc_notify(ctp, msg, o->notify, trig, 0, 0);
}

static int di_devctl(resmgr_context_t *ctp, io_devctl_t *msg, void *ocb)
{
	struct relay_cb *rcb = ((struct relay_ocb *)ocb)->rcb;
	
	/* a cloner is not allowed to SET any attribute, return EOK to cheat him */
	if (ctp->id == rcb->resdid && (get_device_direction(msg->i.dcmd) & DEVDIR_TO) != 0)
	{
		msg->o.ret_val = 0;
		return EOK;
	}
	
	return di_bypass(ctp, msg, ocb);
}

static int di_unblock(resmgr_context_t *ctp, io_pulse_t *msg, void *ocb)
{
	struct relay_ocb  *o = ocb;
	struct relay_cb *rcb = o->rcb;
	
	DPRINT(("unblock(%d)\n", o->realfd));
	if (ctp->id == rcb->resdid && o->rcvid) {
		MsgError(o->rcvid, EINTR);
		return _RESMGR_NOREPLY;
	}
	
	if (o->block_tid) {
		o->flag |= OCB_FLAG_UNBLOCK;
		SignalKill(ND_LOCAL_NODE, mypid, o->block_tid, SIGUSR1, 0, 0);
	} else if (o->rcvid) {
		MsgError(o->rcvid, EINTR);
	}
	return _RESMGR_NOREPLY;
}

int di_mgr_init(resmgr_connect_funcs_t *cf, resmgr_io_funcs_t *iof)
{
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, cf, _RESMGR_IO_NFUNCS, iof);
	cf->open       = di_open;
	
	iof->read      = di_read;
	iof->write     = di_write;
	iof->close_ocb = di_closeocb;
	iof->notify    = di_notify;
	iof->devctl    = di_devctl;
	iof->unblock   = di_unblock;
	iof->stat      = di_stat;
	/* other iofuncs could also set to di_bypass() */

	/* buffer my pid, to be used in SignalKill() */
	mypid = getpid();
	return 0;
}  
