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

#include <unistd.h>
#include "externs.h"
//#define	MSG_XFER_CHUNK_SIZE		1024


int kdecl
ker_msg_current(THREAD *act, struct kerargs_msg_current *kap) {
	CONNECT				*cop;
	THREAD				*thp;

	lock_kernel();

	if(act->client != 0) {
		act->client->args.ms.server = 0;
		act->client = 0;
	}

	if(kap->rcvid == 0) {
		return EOK;
	}

	if((cop = lookup_rcvid((KERARGS *)(void *)kap, kap->rcvid, &thp)) == NULL) {
		return ENOERROR;
	}
	
	// Make sure thread is replied blocked on a channel owned by this process.
	if(thp->state != STATE_REPLY && thp->state != STATE_NET_REPLY) {
			return ESRCH;
	}
	
	// Verify that the message has been fully received, and that the receiving
	// thread has completed any specialret() processing that needs to be done.
	if ( thp->internal_flags & _NTO_ITF_SPECRET_PENDING ) {
		return ESRCH;
	}
	
	if(thp->blocked_on != cop) {
		CONNECT *cop1 = cop;
		cop = thp->blocked_on;
		if((cop->flags & COF_VCONNECT) == 0 || cop->un.lcl.cop != cop1) {
			return ESRCH;
		}
	}

	if(thp->args.ms.server != 0) {
		thp->args.ms.server->client = 0;
	}

	thp->args.ms.server = act;
	act->client = thp;

	/* By default the server runs with message driven priority. */
	if((cop->channel->flags & _NTO_CHF_FIXED_PRIORITY) == 0) {
		act->real_priority = thp->priority;
		/* @@@ Should we call adjust_priority() here?? */
		AP_INHERIT_CRIT(act,thp);
		adjust_priority(act, thp->priority, thp->dpp, 1);
	}

	return EOK;
}

int kdecl
ker_msg_readv(THREAD *act, struct kerargs_msg_readv *kap) {
	CONNECT	*cop;
	THREAD	*thp;
	int		xferstat;
	unsigned	slen;

	if((cop = lookup_rcvid((KERARGS *)(void *)kap, kap->rcvid, &thp)) == NULL) {
		return ENOERROR;
	}

	// Make sure thread is replied blocked on a channel owned by this process.
	if(thp->state != STATE_REPLY && thp->state != STATE_NET_REPLY) {
			return ESRCH;
	}
	
	// Verify that the message has been fully received, and that the receiving
	// thread has completed any specialret() processing that needs to be done.
	if ( thp->internal_flags & _NTO_ITF_SPECRET_PENDING ) {
		return ESRCH;
	}
	
	if(thp->blocked_on != cop) {
		CONNECT *cop1 = cop;
		cop = thp->blocked_on;
		if((cop->flags & COF_VCONNECT) == 0 || cop->un.lcl.cop != cop1) {
			return ESRCH;
		}
	}
	
	act->args.ms.rmsg = kap->rmsg;
	act->args.ms.rparts = kap->rparts;

	// Validate IOV boundary
	if(kap->rparts < 0) {
		// Single part
		uintptr_t	base, last;
		int			len;

		base = (uintptr_t) kap->rmsg;
		len = -kap->rparts;
		last = base + len - 1;
		if((base > last) || !WITHIN_BOUNDRY(base, last, act->process->boundry_addr)) {
			return ESRVRFAULT;
		}
	} else if(kap->rparts == 1) {
		// One IOV
		uintptr_t	base, last;
		int			len;

		base = (uintptr_t) GETIOVBASE(kap->rmsg);
		len = GETIOVLEN(kap->rmsg);
		last = base + len - 1;
		if(((base > last) || !WITHIN_BOUNDRY(base, last, act->process->boundry_addr)) && (len != 0)) {
			return ESRVRFAULT;
		}
	} else {
		// Multi-IOV. Walk IOVs for boundary check.
		uintptr_t	base, last;
		int			len, rparts = kap->rparts;
		IOV			*iov = kap->rmsg;

		while(rparts) {
			base = (uintptr_t) GETIOVBASE(iov);
			len = GETIOVLEN(iov);
			last = base + len - 1;
			if(((base > last) || !WITHIN_BOUNDRY(base, last, act->process->boundry_addr)) && (len != 0)) {
				return ESRVRFAULT;
			}
			iov++;
			--rparts;
		}
	}

	if(thp->flags & _NTO_TF_BUFF_MSG) {
		unsigned	dlen;

		slen = thp->args.msbuff.msglen;
		if(kap->offset < slen) {
			slen -= kap->offset;
			xferstat = xfer_cpy_diov(act, act->args.ms.rmsg,
									thp->args.msbuff.buff + kap->offset,
									act->args.ms.rparts,
									slen);
			dlen = xferlen(act, act->args.ms.rmsg, act->args.ms.rparts);
			if(dlen < slen) slen = dlen;
		} else {
			//No data to transfer
			xferstat = 0;
			slen = 0;
		}
	} else {
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
		*(volatile void**) &(act->blocked_on) = thp;
		*(volatile int_fl_t*) &(act->internal_flags) |= _NTO_ITF_MSG_DELIVERY;
		*(volatile int_fl_t*) &(thp->internal_flags) |= _NTO_ITF_MSG_DELIVERY;
#endif
		xferstat = xfermsg(act, thp, 0, kap->offset);
		slen = thp->args.ms.msglen;
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
		lock_kernel();
		thp->internal_flags &= ~_NTO_ITF_MSG_DELIVERY;
		act->internal_flags &= ~_NTO_ITF_MSG_DELIVERY;
		act->blocked_on = 0;
		if(thp->internal_flags & _NTO_ITF_MSG_FORCE_RDY) {
			force_ready(thp,KSTATUS(thp));
			thp->internal_flags &= ~_NTO_ITF_MSG_FORCE_RDY;
		}
#endif
	}

	lock_kernel();
	if(xferstat) {
		kererr(act, (xferstat & XFER_SRC_FAULT) ? EFAULT : ESRVRFAULT);
	} else {
		SETKSTATUS(act, slen);
	}

	act->restart = NULL;
	return ENOERROR;
}



int kdecl
ker_msg_writev(THREAD *act, struct kerargs_msg_writev *kap) {
	CONNECT	*cop;
	THREAD	*thp;
	int		xferstat;

	if((cop = lookup_rcvid((KERARGS *)(void *)kap, kap->rcvid, &thp)) == NULL) {
		return ENOERROR;
	}

	// Make sure thread is replied blocked on a channel owned by this process.
	if(thp->state != STATE_REPLY && thp->state != STATE_NET_REPLY) {
			return ESRCH;
	}
	
	// Verify that the message has been fully received, and that the receiving
	// thread has completed any specialret() processing that needs to be done.
	if ( thp->internal_flags & _NTO_ITF_SPECRET_PENDING ) {
		return ESRCH;
	}
	
	if(thp->blocked_on != cop) {
		CONNECT *cop1 = cop;
		cop = thp->blocked_on;
		if((cop->flags & COF_VCONNECT) == 0 || cop->un.lcl.cop != cop1) {
			return ESRCH;
		}
	}

	// Validate IOV boundary
	if(kap->sparts < 0) {
		// Single part
		uintptr_t	base, last;
		int			len;

		base = (uintptr_t) kap->smsg;
		len = -kap->sparts;
		last = base + len - 1;
		if((base > last) || !WITHIN_BOUNDRY(base, last, act->process->boundry_addr)) {
			return ESRVRFAULT;
		}
	} else if(kap->sparts == 1) {
		// One IOV
		uintptr_t	base, last;
		int			len;

		base = (uintptr_t) GETIOVBASE(kap->smsg);
		len = GETIOVLEN(kap->smsg);
		last = base + len - 1;
		if(((base > last) || !WITHIN_BOUNDRY(base, last, act->process->boundry_addr)) && (len != 0)) {
			return ESRVRFAULT;
		}
	} else {
		// Multi-IOV. Walk IOVs for boundary check.
		uintptr_t	base, last;
		int			len, rparts = kap->sparts;
		IOV			*iov = kap->smsg;

		while(rparts) {
			base = (uintptr_t) GETIOVBASE(iov);
			len = GETIOVLEN(iov);
			last = base + len - 1;
			if(((base > last) || !WITHIN_BOUNDRY(base, last, act->process->boundry_addr)) && (len != 0)) {
				return ESRVRFAULT;
			}
			iov++;
			--rparts;
		}
	}
	
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
	*(volatile void**) &(act->blocked_on) = thp;
	*(volatile int_fl_t*) &(act->internal_flags) |= _NTO_ITF_MSG_DELIVERY;
	*(volatile int_fl_t*) &(thp->internal_flags) |= _NTO_ITF_MSG_DELIVERY;
#endif
	act->args.ms.smsg = kap->smsg;
	act->args.ms.sparts = kap->sparts;
	xferstat = xfermsg(thp, act, kap->offset, 0);
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
	lock_kernel();
	thp->internal_flags &= ~_NTO_ITF_MSG_DELIVERY;
	act->internal_flags &= ~_NTO_ITF_MSG_DELIVERY;
	act->blocked_on = 0;
	if(thp->internal_flags & _NTO_ITF_MSG_FORCE_RDY) {
		force_ready(thp,KSTATUS(thp));		
		thp->internal_flags &= ~_NTO_ITF_MSG_FORCE_RDY;
	}
#endif

	if(xferstat) {
		lock_kernel();
		kererr(act, (xferstat & XFER_SRC_FAULT) ? ESRVRFAULT : EFAULT);
	} else {
		lock_kernel();
		SETKSTATUS(act, act->args.ms.msglen);
	}

	act->restart = 0;
	return ENOERROR;
}



int kdecl
ker_msg_readiov(THREAD *act, struct kerargs_msg_readiov *kap) {
	CONNECT	*cop;
	THREAD	*thp;
	int		n;

	if((cop = lookup_rcvid((KERARGS *)(void *)kap, kap->rcvid, &thp)) == NULL) {
		return ENOERROR;
	}

	// Make sure thread is replied blocked on a channel owned by this process.
	if(thp->state != STATE_REPLY && thp->state != STATE_NET_REPLY) {
			return ESRCH;
	}
	
	// Verify that the message has been fully received, and that the receiving
	// thread has completed any specialret() processing that needs to be done.
	if ( thp->internal_flags & _NTO_ITF_SPECRET_PENDING ) {
		return ESRCH;
	}
	
	if(thp->blocked_on != cop) {
		CONNECT *cop1 = cop;
		cop = thp->blocked_on;
		if((cop->flags & COF_VCONNECT) == 0 || cop->un.lcl.cop != cop1) {
			return ESRCH;
		}
	}
	
	if(kap->flags & _NTO_READIOV_REPLY) {
		n = viov_to_piov(thp->process, kap->iov, kap->parts, thp->args.ms.rmsg,
							thp->args.ms.rparts, kap->offset);
	} else {
		n = viov_to_piov(thp->process, kap->iov, kap->parts, thp->args.ms.smsg,
							thp->args.ms.sparts, kap->offset);
	}
	if(n < 0) {
		return -n;
	}

	lock_kernel();
	SETKSTATUS(act, n);
	return ENOERROR;
}



int kdecl
ker_msg_info(THREAD *act, struct kerargs_msg_info *kap) {
	THREAD	*thp;
	CONNECT	*cop;

	WR_VERIFY_PTR(act, kap->info, sizeof(*kap->info));
	WR_PROBE_OPT(act, kap->info, sizeof(*kap->info) / sizeof(int));

	if((cop = lookup_rcvid(NULL, kap->rcvid, &thp)) == NULL) {
		return ENOERROR;
	}

	// Make sure thread is replied blocked on a channel owned by this process.
	if(thp->state != STATE_REPLY && thp->state != STATE_NET_REPLY) {
			return ESRCH;
	}
	
	// Verify that the message has been fully received, and that the receiving
	// thread has completed any specialret() processing that needs to be done.
	if ( thp->internal_flags & _NTO_ITF_SPECRET_PENDING ) {
		return ESRCH;
	}
	
	if(thp->blocked_on != cop) {
		CONNECT *cop1 = cop;
		cop = thp->blocked_on;
		if((cop->flags & COF_VCONNECT) == 0 || cop->un.lcl.cop != cop1) {
			return ESRCH;
		}
	}

	get_rcvinfo(thp, -1, cop, kap->info);

	lock_kernel();
	return EOK;
}


int kdecl
ker_msg_deliver_event(THREAD *act, struct kerargs_msg_deliver_event *kap) {
	THREAD	*thp;
	CONNECT	*cop;
	int		 status;
	struct sigevent event = *kap->event;

	RD_VERIFY_PTR(act, kap->event, sizeof(*kap->event));
	// Make sure connection is valid and extract destination thread.
	if((cop = lookup_rcvid((KERARGS *)(void *)kap, kap->rcvid, &thp)) == NULL) {
		return ENOERROR;
	}

	lock_kernel();

	// Since this is a user request for an event and can handle
	// an error return, we shouldn't delve into the critical list
	// for any pulse that we need to allocate for the event - save
	// the critical heap for important pulses like hwintrs and
	// CPU exception fault delivery
	pulse_souls.flags &= ~SOUL_CRITICAL;
	status = sigevent_exe(&event, thp, 1);
	pulse_souls.flags |= SOUL_CRITICAL;
	
	if (status != 0) {
		return status;
	}

	return EOK;
}


int kdecl
ker_msg_verify_event(THREAD *act, struct kerargs_msg_verify_event *kap) {
	THREAD	*thp;
	int		 status;
	struct sigevent event = *kap->event;

	RD_VERIFY_PTR(act, kap->event, sizeof(*kap->event));

	// Use calling thread, or extract destination thread if rcvid given.
	thp = act;
	if(kap->rcvid && lookup_rcvid((KERARGS *)(void *)kap, kap->rcvid, &thp) == NULL) {
		return ENOERROR;
	}

	lock_kernel();

	if((status = sigevent_exe(&event, thp, 0)) != 0) {
		return status;
	}

	return EOK;
}


int kdecl
ker_msg_keydata(THREAD *act, struct kerargs_msg_keydata *kap) {
	THREAD	*thp;
	CONNECT	*cop;
	int		key_in;

	if(!kerisroot(act))
		return ENOERROR;

	WR_VERIFY_PTR(act, kap->newkey, sizeof(*kap->newkey));
	WR_PROBE_OPT(act, kap->newkey, sizeof(*kap->newkey) / sizeof(int));

	// Make sure connection is valid and extract destination thread.
	if((cop = vector_lookup(&act->process->chancons, MCINDEX(kap->rcvid))) == NULL
	|| (cop->type != TYPE_CONNECTION)
	|| (thp = vector_lookup(&cop->process->threads, MTINDEX(kap->rcvid))) == NULL) {
		return ESRCH;
	}

	key_in = kap->key;
	if(kap->op != _NTO_KEYDATA_VERIFY) {		// Calculate key.
		if(key_in == 0) {
			// Generate a random key value
			key_in = (uintptr_t)act ^ pid_unique ^ (int)qtimeptr->nsec;
		}
		*kap->newkey = keygen(kap->msg, kap->parts, thp->key = key_in);
	} else {				// Verify key
		*kap->newkey = (keygen(kap->msg, kap->parts, thp->key) == key_in) ? 0 : -1;
	}

	return EOK;
}

/* This routine is not in use currently */
int kdecl
ker_msg_readwritev(THREAD *act, struct kerargs_msg_readwritev *kap) {
#if 1
	return ENOSYS;
#else
	CONNECT	*src_cop, *dst_cop;
	THREAD	*src_thp, *dst_thp;
	int		xferstat;
	
	if((src_cop = lookup_rcvid((KERARGS *)(void *)kap, kap->src_rcvid, &src_thp)) == NULL) {
		return ENOERROR;
	}

	// Make sure src thread is replied blocked on a channel owned by this process.
	if(src_thp->state != STATE_REPLY || src_thp->blocked_on != src_cop) {
		return ESRCH;
	}

	if((dst_cop = lookup_rcvid(NULL, kap->dst_rcvid, &dst_thp)) == NULL) {
		return ENOERROR;
	}

	// Make sure dst thread is replied blocked on a channel owned by this process.
	if(dst_thp->state != STATE_REPLY || dst_thp->blocked_on != dst_cop) {
		return ESRCH;
	}

	src_thp->args.ms.smsg = kap->src_msg;
	src_thp->args.ms.sparts = kap->src_parts;
	dst_thp->args.ms.smsg = kap->dst_msg;
	dst_thp->args.ms.sparts = kap->dst_parts;
	xferstat = xfermsg(dst_thp, src_thp, kap->dst_offset, kap->src_offset);

	if(xferstat) {
		lock_kernel();
		kererr(act, (xferstat & XFER_SRC_FAULT) ? EFAULT : ESRVRFAULT);
	} else {
		lock_kernel();
		SETKSTATUS(act, src_thp->args.ms.msglen);
	}

	act->restart = NULL;
	return ENOERROR;
#endif
}

__SRCVERSION("ker_message.c $Rev: 162922 $");
