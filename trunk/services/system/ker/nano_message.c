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
#include <unistd.h>

uint32_t rdecl
keygen(IOV *msg, int32_t parts, uint32_t key1) {
	int32_t	key2, i;
	char	*cp, *cpend;

	key2 = 0;
	for(i = 0 ; i < parts ; ++i) {
		cp = msg[i].iov_base;
		for(cpend = cp + msg[i].iov_len ; cp < cpend ; ++cp) {
			key2 = ((key2 << 1) ^ key1 ) + *cp;
		}
	}

	return(key2);
}


void rdecl
get_rcvinfo(THREAD *thp, int tid, CONNECT *cop, struct _msg_info *rep) {
	PROCESS	*prp;

#ifndef NDEBUG
	if(thp == NULL) crash();
#endif

	// thp->process == cop->process if we used net_send2()
	if((cop->flags & COF_NETCON) && TYPE_MASK(thp->type) == TYPE_VTHREAD) {
		rep->srcnd = thp->un.net.srcnd;
		rep->nd = cop->un.net.nd;
		rep->pid = cop->un.net.pid;
#if _NTO_MI_ENDIAN_BIG != _NTO_CI_ENDIAN_BIG
#error _NTO_MI_ENDIAN_BIG != _NTO_CI_ENDIAN_BIG
#endif
		if((rep->flags = (cop->un.net.flags & _NTO_MI_ENDIAN_BIG))) {
#if defined(__BIGENDIAN__)
			/* Endian's match.... */
		} else {
			rep->flags |= _NTO_MI_ENDIAN_DIFF;
		}
#elif defined(__LITTLEENDIAN__)
			rep->flags |= _NTO_MI_ENDIAN_DIFF;
		} else {
			/* Endian's match.... */
		}
#else
#error ENDIAN Not defined for system
#endif
	} else {
		rep->srcnd = cop->un.lcl.nd;
		rep->nd = ND_LOCAL_NODE;
		rep->pid = cop->process->pid;
#if defined(__BIGENDIAN__)
		rep->flags = _NTO_MI_ENDIAN_BIG;
#elif defined(__LITTLEENDIAN__)
		rep->flags = 0;
#else
#error ENDIAN Not defined for system
#endif
	}
	rep->chid = cop->channel ? cop->channel->chid : -1;
	rep->scoid = cop->scoid | _NTO_SIDE_CHANNEL;
	rep->msglen = thp->args.ms.msglen;
	rep->srcmsglen = (thp->flags & _NTO_TF_BUFF_MSG) ?
				thp->args.ms.msglen : thp->args.ms.srcmsglen;
	rep->dstmsglen = thp->args.ms.dstmsglen;
	rep->coid = thp->args.ms.coid;
	rep->priority = thp->priority;
	if(tid == -1) {
		tid = thp->tid;
	}
	rep->tid = tid + 1;

	// Is there an unblock pending on the thread?
	if(thp->flags & _NTO_TF_UNBLOCK_REQ) {
		rep->flags |= _NTO_MI_UNBLOCK_REQ;
	}

	// In the case of Net doing an info request we let him know
	// if the process credentials have changed.
	if(TYPE_MASK(thp->type) == TYPE_THREAD) {
		prp = thp->process;

		if(actives[KERNCPU]->process == net.prp  &&
				rep->chid == net.chp->chid  &&
				(cop->flags & COF_NETCON) == 0) {
			if(cop->flags & COF_VCONNECT) {
				cop = cop->un.lcl.cop;
			}
			if(prp->seq != cop->un.lcl.seq) {
				cop->un.lcl.seq = prp->seq;
				rep->flags |= _NTO_MI_NET_CRED_DIRTY;
			}
			rep->msglen = thp->key;
		}
	}
}



int
viov_to_piov(PROCESS *prp, IOV *piop, int pparts, IOV *viop, int vparts, int voffset) {
	char	*vaddr;
	paddr_t	paddr;
	int		 vlen, nparts, n;

	// Skip over the offset specified in the virtual iov.
	while((voffset -= GETIOVLEN(viop)) >= 0) {
		++viop;
		if(--vparts == 0) {
			return(0);
		}
	}
	// Set vaddr and vlen to point within the current viop entry.
	vlen = -voffset;
	vaddr = (GETIOVLEN(viop) - vlen) + (char *)GETIOVBASE(viop);

	// Step through each virtual part producing one or more physical parts.
	for(nparts = 0 ; pparts ;) {

		for(;pparts  &&  vlen ; vlen -= n, vaddr += n) {

			// Limit steps to 1 page.
			n = min(vlen, memmgr.pagesize);

			// Convert virtual addr to physical addr.
			if(memmgr.vaddrinfo(prp, (uintptr_t)vaddr, &paddr, NULL, VI_NORMAL) == PROT_NONE) {
				paddr = ~(paddr_t)0;
			}

			// Check if address is contiguious.
			if(nparts  &&  paddr == (paddr_t)(uintptr_t)GETIOVBASE(piop-1) + GETIOVLEN(piop-1)) {
				SETIOV(piop-1, GETIOVBASE(piop-1), GETIOVLEN(piop-1) + n);
				continue;
			}

#if _PADDR_BITS+0 > __INT_BITS__
			if(paddr + n > UINTPTR_MAX) {
				return -EOVERFLOW;
			}
#endif
			// Page not contiguious in physical memory so create new part.
			SETIOV(piop, (intptr_t)paddr, n);
			++piop;
			--pparts;
			++nparts;
		}

		// Finished current virtual part. Any more parts?
		if(--vparts == 0) {
			break;
		}

		// Move to next virtual part.
		++viop;
		vaddr = GETIOVBASE(viop);
		vlen = GETIOVLEN(viop);
	}

	return(nparts);
}


//
// The server ends up reply blocked on the
// network thread who will msgreadv/msgreadiov/msgwritev the data in/out.
//
CONNECT * rdecl
net_send2(KERARGS *kap, int vtid, CONNECT *cop, THREAD *thp) {
	THREAD				*act = actives[KERNCPU];
	int					 type = KTYPE(act);
	int					 status;
	unsigned			 prio;

	if(!net.chp) {
		// qnet is either dead or never started. For dead case, have to repaire vthreads
		if(type == __KER_CHANNEL_DESTROY) {
			if(thp->args.ms.server != 0) {
				thp->args.ms.server->client = 0;
				thp->args.ms.server = 0;
			}
			switch(thp->state) {
			case STATE_SEND:
			case STATE_NET_SEND:
				// Remove vthread from send queue
				pril_rem(&cop->channel->send_queue, thp);
				break;
			default:
				// Remove vthread from reply queue
				LINKPRIL_REM(thp);
				break;
			}
			thp->state = STATE_STOPPED;
			snap_time(&thp->timestamp_last_block,0);

			_TRACE_TH_EMIT_STATE(thp, STOPPED);
		}
		kererr(act, ENOREMOTE);
		return(NULL);
	}

	switch(type) {
	case __KER_MSG_VERIFY_EVENT:
		// @@@ Not supported yet
		kererr(act, ENOREMOTE);
		return(NULL);

	case __KER_MSG_DELIVER_EVENT:
	case __KER_CHANNEL_DESTROY:
		// These are the only two that don't need a reply blocked connection
		break;

	default:
		if(thp->state != STATE_REPLY  ||  thp->blocked_on != cop) {
			kererr(act, ESRCH);
			return(NULL);
		}
	}

	act->args.ms.rparts = 0;

	switch(type) {
	case __KER_MSG_READV:
		act->args.ms.srcmsglen = xferlen(act, kap->msg_readv.rmsg, kap->msg_readv.rparts);
		act->args.ms.rmsg = kap->msg_readv.rmsg;
		act->args.ms.rparts = kap->msg_readv.rparts;
		act->args.ms.sparts = 0;
		act->args.ms.msglen = kap->msg_readv.offset;
		break;

	case __KER_MSG_READWRITEV:
		kererr(act, ENOREMOTE);
		return(NULL);

	case __KER_MSG_WRITEV:
		act->args.ms.srcmsglen = xferlen(act, kap->msg_writev.smsg, kap->msg_writev.sparts);
		act->args.ms.smsg = kap->msg_writev.smsg;
		act->args.ms.sparts = kap->msg_writev.sparts;
		act->args.ms.msglen = kap->msg_writev.offset;
		break;

	case __KER_MSG_REPLYV:
		act->args.ms.srcmsglen = xferlen(act, kap->msg_replyv.smsg, kap->msg_replyv.sparts);
		act->args.ms.smsg = kap->msg_replyv.smsg;
		act->args.ms.sparts = kap->msg_replyv.sparts;
		act->args.ms.msglen = kap->msg_replyv.status;
		break;

	case __KER_MSG_DELIVER_EVENT:
		act->args.msbuff.msglen = sizeof *kap->msg_deliver_event.event;
		memcpy(act->args.msbuff.buff, kap->msg_deliver_event.event, sizeof *kap->msg_deliver_event.event);
		lock_kernel();
		act->flags |= _NTO_TF_BUFF_MSG;
		break;

	case __KER_CHANNEL_DESTROY:
		act->args.msbuff.msglen = sizeof(int);
		*(int *)act->args.msbuff.buff = ESRCH;
		lock_kernel();
		act->flags |= _NTO_TF_BUFF_MSG;
		type = __KER_MSG_ERROR;
		break;

	case __KER_MSG_ERROR:
		act->args.msbuff.msglen = sizeof kap->msg_error.err;
		memcpy(act->args.msbuff.buff, &kap->msg_error.err, sizeof kap->msg_error.err);
		lock_kernel();
		act->flags |= _NTO_TF_BUFF_MSG;
		break;

	case __KER_MSG_CURRENT:
		return cop;

	default:
		crash();
	}

	lock_kernel();

	if(type == __KER_MSG_REPLYV  ||  type == __KER_MSG_ERROR) {
//		if(thp->internal_flags & _NTO_ITF_UNBLOCK_QUEUED)
//			remove_unblock(thp, cop, kap->msg_replyv.rcvid);

		if(thp->args.ms.server != 0) {
			thp->args.ms.server->client = 0;
			thp->args.ms.server = 0;
		}
		switch(thp->state) {
		case STATE_SEND:
		case STATE_NET_SEND:
			// Remove vthread from send queue
			pril_rem(&cop->channel->send_queue, thp);
			break;
		default:
			// Remove vthread from reply queue
			LINKPRIL_REM(thp);
			break;
		}
		thp->state = STATE_STOPPED;

		_TRACE_TH_EMIT_STATE(thp, STOPPED);
	} else {
		++cop->links;		// Since qnet's reply will decrement it
	}

	/* this is not a client blocked for server, no onbehalf */
	act->args.ms.server = 0;
	act->args.ms.coid = vtid;
	act->state = STATE_REPLY;
	act->restart = 0;

	_TRACE_TH_EMIT_STATE(act, REPLY);
	block();
	act->blocked_on = cop;
	LINKPRIL_BEG(net.chp->reply_queue, act, THREAD);

	prio = act->priority;
	//Might be coming from a system with more priority levels than us
	if(prio > NUM_PRI - 1) prio = NUM_PRI - 1;
	if((status = pulse_deliver(net.chp, prio | _PULSE_PRIO_VTID,
					type, (act->tid << 16) | (cop->un.net.coid & 0xffff),
					(uintptr_t)act,0)) != EOK) {
		force_ready(act, status | _FORCE_SET_ERROR);
	}

	return(NULL);
}


VTHREAD * rdecl
net_send1(int vtid, struct _vtid_info *vtp) {
	VTHREAD				*thp;
	unsigned			prio;

	thp = vector_lookup(&vthread_vector, vtid);
#ifndef NDEBUG
	if(thp == NULL || thp->un.net.vtid != vtid) crash();
#endif

	thp->args.ms.coid = vtp->coid;
	prio = vtp->priority;
	//Might be coming from a system with more priority levels than us
	if(prio > NUM_PRI - 1) prio = NUM_PRI - 1;
	thp->priority = prio;
	thp->key = vtp->keydata;
	thp->un.net.srcnd = vtp->srcnd;

	return(thp);
}


void rdecl
remove_unblock(THREAD *thp, CONNECT *cop, int rcvid) {
	PULSE	*pup, *pup2;
	CHANNEL	*chp = cop->channel;

	if(chp == NULL) {
		return;
	}

	for(pup = pril_first(&chp->send_queue); pup; pup = pup2) {
		pup2 = pril_next(pup);
		if((TYPE_MASK(pup->type) == TYPE_PULSE)
			&& (pup->code == _PULSE_CODE_UNBLOCK)
			&& (pup->value == rcvid)) {
			lock_kernel();
			pril_rem(&chp->send_queue, pup);
			object_free(chp->process, &pulse_souls, pup);
		}
	}

	thp->internal_flags &= ~_NTO_ITF_UNBLOCK_QUEUED;
}

int rdecl
net_sendmsg(THREAD *act, CONNECT *cop, int prio) {
	CHANNEL	*chp;
	THREAD	*thp;

	// Get dst channel.
	if((chp = cop->channel) == NULL) {
		return EBADF;
	}

	act->args.msbuff.rmsg = (void*) -1;
	act->args.msbuff.rparts = 0;

	lock_kernel();
	act->flags |= _NTO_TF_BUFF_MSG;

	thp = chp->receive_queue;
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
	while((thp != NULL) && (thp->internal_flags & _NTO_ITF_MSG_DELIVERY)) {
		thp = thp->next.thread;
	}
#endif
	if((thp != NULL) && !(thp->internal_flags & _NTO_ITF_RCVPULSE) ) {
		thp->flags |= _NTO_TF_SHORT_MSG;
		thp->blocked_on = act;
		// Indicate that there is a receiver depending on data in our THREAD object.
		act->internal_flags |= _NTO_ITF_SPECRET_PENDING;

		SETKSTATUS(thp,  -((act->tid << 16) | cop->scoid));

		thp->restart = NULL;
		if(thp->args.ri.info)  {
			thp->args.ri.cop  = cop;
			thp->args.ri.thp  = act;
			thp->flags |= _NTO_TF_RCVINFO;
		}

		// Unlink receive thread from the receive queue.
		LINKPRIL_REM(thp);

		// By default the receiver runs with message driven priority.
		if((chp->flags & _NTO_CHF_FIXED_PRIORITY) == 0) {
			thp->real_priority = thp->priority = prio;
		}

		/* this is not a client blocked for server, no onbehalf */
		act->args.ms.server = 0;

		// Block the active thread and ready the receiver thread
		act->state = STATE_NET_REPLY;	// Must be set before calling block_and_ready()

		_TRACE_TH_EMIT_STATE(act, NET_REPLY);
		block_and_ready(thp);
		act->blocked_on = cop;

		// Link the now reply blocked sending thread in the reply queue
		LINKPRIL_BEG(chp->reply_queue, act, THREAD);
		++cop->links;

		return ENOERROR;
	}

	if(chp->process->flags & (_NTO_PF_TERMING | _NTO_PF_ZOMBIE | _NTO_PF_COREDUMP)) {
		return ENXIO;
	}
	act->state = STATE_NET_SEND;	// Must be set before calling block()
	_TRACE_TH_EMIT_STATE(act, NET_SEND);
	block();
	act->blocked_on = cop;
	pril_add(&chp->send_queue, act);
	++cop->links;

	// To prevent priority inversion, boost all threads in the server
	if((chp->flags & _NTO_CHF_FIXED_PRIORITY) == 0) {
		int i;

		for(i = 0; i < chp->process->threads.nentries; ++i) {
			if(VECP(thp, &chp->process->threads, i)  &&  thp->priority < prio
			&&  thp->last_chid == chp->chid) {
				adjust_priority(thp, prio, thp->dpp, 1);
				thp->real_priority = thp->priority;
			}
		}
	}

	return ENOERROR;
}

int rdecl
net_send_pulse(THREAD *act, CONNECT* cop, int coid, int prio, int code, int value) {
	struct _pulse *p;

	prio = 0xff & prio;
	if( prio == 0 || prio > NUM_PRI - 1) {
		return EINVAL;
	}
	act->args.msbuff.coid = coid;
	act->args.msbuff.msglen = sizeof(int) + sizeof(struct _pulse);
	act->args.msbuff.dstmsglen = __KER_MSG_SEND_PULSE;
	*(int*)(&act->args.msbuff.buff[0]) = prio;
	p = (struct _pulse*)&act->args.msbuff.buff[sizeof(int)];
	p->type = _PULSE_TYPE;
	p->subtype = _PULSE_SUBTYPE;
	p->code = code;
	p->value.sival_int = value;
	p->scoid = cop->scoid;

	return net_sendmsg(act, cop, prio);
}

__SRCVERSION("nano_message.c $Rev: 207484 $");
