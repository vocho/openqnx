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


int kdecl
ker_channel_create(THREAD *act, struct kerargs_channel_create *kap) {
	PROCESS	*prp;
	CHANNEL	*chp;
	int		 chid;
	SOUL	*souls;
	VECTOR	*chvec;

	prp = act->process;
	if((kap->flags & (_NTO_CHF_THREAD_DEATH|_NTO_CHF_COID_DISCONNECT))  &&  prp->death_chp) {
		return EBUSY;
	}

	souls = &channel_souls;
	chvec = &prp->chancons;
	if(kap->flags & (_NTO_CHF_ASYNC | _NTO_CHF_GLOBAL)) {
		souls = &chasync_souls;
		if(kap->event != NULL) {
			RD_PROBE_INT(act, kap->event, sizeof(*kap->event)/sizeof(int));
		}
		if(kap->flags & _NTO_CHF_GLOBAL) {
			prp = NULL;
			souls = &chgbl_souls;
			chvec = &chgbl_vector;
			if(kap->cred != NULL) {
				RD_PROBE_INT(act, kap->cred, sizeof(*kap->cred)/sizeof(int));
			}
		}
	}

	lock_kernel();
	// Allocate a channel entry.
	if((chp = object_alloc(prp, souls)) == NULL) {
		return EAGAIN;
	}

	chp->type = TYPE_CHANNEL;
	chp->process = prp;
	chp->flags = kap->flags;

	if(kap->flags & _NTO_CHF_GLOBAL) {
		if(chp->reply_queue) crash();
	}	
	// Check if this is the network managers msg channel.
	// Don't do this until we know the channel's fully created, so
	// we (mostly) don't have to worry about net.* fields not getting cleared
	// on an error.
	if(kap->flags & _NTO_CHF_NET_MSG) {
		if(!kerisroot(act)) {
			object_free(prp, &channel_souls, chp);
			return EPERM;
		}
		if(net.prp  ||  net.chp) {
			object_free(prp, &channel_souls, chp);
			return EAGAIN;
		}
		net.prp = prp;
		net.chp = chp;
	}

	// Add the channel to the channels vector.
	if((chid = vector_add(chvec, chp, 1)) == -1) {
		object_free(prp, &channel_souls, chp);
		if(kap->flags & _NTO_CHF_NET_MSG) {
			net.prp = NULL;
			net.chp = NULL;
		}
		return EAGAIN;
	}

	if(kap->flags & (_NTO_CHF_THREAD_DEATH | _NTO_CHF_COID_DISCONNECT)) {
		if(prp) { /* not global channel */
			prp->death_chp = chp;
		}
	}

	if(kap->flags & (_NTO_CHF_ASYNC | _NTO_CHF_GLOBAL)) {
		if(kap->flags & _NTO_CHF_GLOBAL) {
			CHANNELGBL *chgbl = (CHANNELGBL*)chp;

			chid |= _NTO_GLOBAL_CHANNEL;
			chgbl->event.sigev_notify = SIGEV_NONE;
			chgbl->max_num_buffer = kap->maxbuf;
			chgbl->buffer_size = kap->bufsize;
			if(kap->event != NULL) {
				memcpy(&chgbl->event, kap->event, sizeof(*kap->event));
				chgbl->ev_prp = act->process;
			}
			if(kap->cred) {
				memcpy(&chgbl->cred, kap->cred, sizeof(chgbl->cred));
			}
		} else {
			((CHANNELASYNC*)chp)->event.sigev_notify = SIGEV_NONE;
			if(kap->event != NULL) {
				memcpy(&((CHANNELASYNC*)chp)->event, kap->event, sizeof(*kap->event));
				((CHANNELASYNC*)chp)->ev_prp = act->process;
			}
		}
	}
	chp->chid = chid;

	SETKSTATUS(act, chid);
	return ENOERROR;
}


#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
static void 
clear_msgxfer(THREAD *thp) {

	switch(thp->state) {
	case STATE_REPLY: 
		thp->flags &= ~_NTO_TF_UNBLOCK_REQ;
		/* fall through */

	case STATE_SEND: {
		CONNECT *cop = thp->blocked_on;
		if(--cop->links == 0) {
			connect_detach(cop, thp->priority);
		}
	}

		/* fall through */
	case STATE_RECEIVE:
		thp->internal_flags &= ~_NTO_ITF_RCVPULSE;
		thp->restart = NULL;
		break;
	default:
#ifndef NDEBUG
		kprintf("\nclear_msgxfer: should not reach here. thread state %x\n",thp->state);
#endif
		crash(); 
		/* NOTREACHED */
		return;
	}
	if(thp->state == STATE_SEND) {
		pril_rem(&((CHANNEL *)thp->blocked_on)->send_queue, thp);
	} else {
		LINKPRIL_REM(thp);
	}

	thp->state = STATE_READY;
}
#define CLR_MSGXFER(thp) if((thp)->internal_flags & _NTO_ITF_MSG_DELIVERY) clear_msgxfer(thp)
#else
#define CLR_MSGXFER(thp)
#endif


int kdecl
ker_channel_destroy(THREAD *act, struct kerargs_channel_destroy *kap) {
	PROCESS	*prp;
	CHANNEL	*chp;
	CONNECT	*cop;
	THREAD	*thp;
	int		i, chid;
	SOUL	*souls;
	VECTOR	*chvec;

	prp = act->process;

	chid = kap->chid;
	chvec = &prp->chancons;
	if(chid & _NTO_GLOBAL_CHANNEL) {
		chid &= ~_NTO_GLOBAL_CHANNEL;
		chvec = &chgbl_vector;
	}
	
	// Make sure the channel is valid.
	chp = vector_lookup2(chvec, chid);
	if((chp == NULL) || (chp->type != TYPE_CHANNEL)) {
		return EINVAL;
	}

	lock_kernel();

	// Mark channel as unavailable.
	vector_flag(chvec, chid, 1);
	// Check if the network manager is releasing his message channel.
	if((prp == net.prp) && (chp == net.chp)) {
		static unsigned	retries;

		for(i = 0; i < vthread_vector.nentries; ++i) {
			VTHREAD		*vthp;

			vthp = vector_rem(&vthread_vector, i);
			if(vthp != NULL) {
				unsigned saved_flags;
				saved_flags = vthp->flags;
				vthp->flags &= ~_NTO_TF_KILLSELF;
				if(!force_ready((THREAD *)(void *)vthp, EINTR|_FORCE_SET_ERROR)) {
					// We couldn't force_ready the thread, we must
					// have been in the middle of a SMP_MSGOPT message
					// pass. The force_ready() call would have invoked
					// force_ready_msgxfer() to abort the transfer, so let's
					// get out of the kernel and let that be processed.
					// If we restart 1 million times without getting out
					// of here, something's gone wrong....
					if(++retries == 1000000) crash();
					vthp->flags = saved_flags;
					vector_add(&vthread_vector, vthp, i);
					KERCALL_RESTART(act);
					return ENOERROR;
				}
				object_free(NULL, &vthread_souls, vthp);
			}
			KER_PREEMPT(act, ENOERROR);
		}
		retries = 0;
		net.prp = NULL;
		net.chp = NULL;
	}

	// Check for destruction of the death channel.
	if(chp == prp->death_chp) {
		prp->death_chp = NULL;
	}

	// Unblock all threads send blocked on the channel
	// We have to do this before we NULL the cop->channel pointers
	// below so that force_ready can find the appropriate PRIL_HEAD structure.
	while((thp=pril_first(&chp->send_queue))) {
		switch(TYPE_MASK(thp->type)) {
		case TYPE_PULSE:	
		case TYPE_VPULSE:
			pril_rem(&chp->send_queue, thp);
			object_free(chp->process, &pulse_souls, thp);
			break;
		case TYPE_VTHREAD:	
			// Turn on _NTO_TF_KERERR_SET so that if net_send2() calls
			// kererr(), it won't advance the IP - we're going to be
			// restarting the kernel call so that qnet has a chance
			// to process the pulse
			act->flags |= _NTO_TF_KERERR_SET;
			(void)net_send2((KERARGS *)(void *)kap, thp->un.net.vtid, 
								thp->blocked_on, thp);
			CLR_MSGXFER(thp);
			KERCALL_RESTART(act);
			return ENOERROR;
		case TYPE_THREAD:	
			force_ready(thp, ESRCH);
			CLR_MSGXFER(thp);
			break;
		default:
			crash();
			break;
		}
		KER_PREEMPT(act, ENOERROR);
	}

	// Unblock all threads receive blocked on the channel
	while((thp=chp->receive_queue)) {
		CRASHCHECK((TYPE_MASK(thp->type) != TYPE_THREAD) && (TYPE_MASK(thp->type) != TYPE_VTHREAD));
		force_ready(thp, ESRCH);
		CLR_MSGXFER(thp);
		KER_PREEMPT(act, ENOERROR);
	}

	if(!(chp->flags & (_NTO_CHF_ASYNC | _NTO_CHF_GLOBAL))) {
		// Unblock all threads reply blocked on the channel
		while((thp = chp->reply_queue)) {
			switch(TYPE_MASK(thp->type)) {
			case TYPE_VTHREAD:	
				// Turn on _NTO_TF_KERERR_SET so that if net_send2() calls
				// kererr(), it won't advance the IP - we're going to be
				// restarting the kernel call so that qnet has a chance
				// to process the pulse
				act->flags |= _NTO_TF_KERERR_SET;
				(void)net_send2((KERARGS *)(void *)kap, thp->un.net.vtid, 
									thp->blocked_on, thp);
				CLR_MSGXFER(thp);
				KERCALL_RESTART(act);
				return ENOERROR;
			case TYPE_THREAD:	
				// Disable unblock pulse since we don't want to add more stuff
				// to this channel that we're in the process of destroying...
				force_ready(thp, ESRCH | _FORCE_SET_ERROR | _FORCE_NO_UNBLOCK);	
				CLR_MSGXFER(thp);
				break;
			default:
				crash();
			}
			KER_PREEMPT(act, ENOERROR);
		}
	} else {
		// release all packages/send requests
		if(chp->flags & _NTO_CHF_GLOBAL) {
			struct gblmsg_entry *gblmsg;
			CHANNELGBL *chgbl = (CHANNELGBL*)chp;

			if(chgbl->free != NULL) {
				_sfree(chgbl->free, chgbl->buffer_size + sizeof(*gblmsg));
			}
			while((gblmsg = (struct gblmsg_entry*) chp->reply_queue)) {
				chp->reply_queue = (void*)gblmsg->next;
				_sfree(gblmsg, chgbl->buffer_size + sizeof(*gblmsg));

			}
		}
	}

	// Find all server connections and mark their channels as NULL.
	if(!(chp->flags & _NTO_CHF_GLOBAL)) {
		for(i = 0 ; i < prp->chancons.nentries ; ++i) {
			if((cop = VECP2(cop, &prp->chancons, i))) {
				if(cop->type == TYPE_CONNECTION  &&  cop->channel == chp  &&  cop->scoid == i) {
	
					// Remove the server connection from the servers channels vector.
					vector_rem(&prp->chancons, i);
	
					if(cop->links == 0) {
						// Remove server connection object.
						object_free(prp, &connect_souls, cop);
					} else {
						do {
							if(cop->process  &&  cop->process->death_chp) {
								connect_coid_disconnect(cop, cop->process->death_chp,
										act->priority);
							}
							// Null channel pointer since it is gone.
							cop->channel = NULL;
						} while((cop = cop->next) && (!(chp->flags & _NTO_CHF_ASYNC)));
					}
				}
			}
			KER_PREEMPT(act, ENOERROR);
		}
	} else {
		cop = ((CHANNELGBL*) chp)->cons;
		while(cop) {
			CONNECT* next = cop->next;
			cop->channel = NULL;
			cop->next = NULL;
			cop = next;
		}
	}

	// Remove the channel from the servers channels vector.
	if((chp = vector_rem(chvec, chid)) == NULL) {
		return EINVAL;
	}

	souls = &channel_souls;
	if(chp->flags & (_NTO_CHF_ASYNC | _NTO_CHF_GLOBAL)) {
		souls = &chasync_souls;
		if(chp->flags & _NTO_CHF_GLOBAL) {
			souls = &chgbl_souls;
		}
	}

	// Release the channel entry.
	object_free(prp, souls, chp);

	return EOK;
}

__SRCVERSION("ker_channel.c $Rev: 209291 $");
