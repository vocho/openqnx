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

// will be moved to an independant file
#define LINK_PRI_MSG(head, tail, msg) { \
	struct gblmsg_entry *m;\
\
	if ((m = (void*)(head)) == NULL) {\
		(msg)->next = NULL, (head) = (void*)((tail) = (msg));\
	}\
	else if ((msg)->priority <= (tail)->priority) {\
		(msg)->next = NULL, (tail)->next = (msg), (tail) = (msg);\
	}\
	else if ((msg)->priority > m->priority) {\
		(msg)->next = m, (head) = (void*)(msg);\
	}\
	else {\
		while (m->next->priority >= (msg)->priority)\
			m = m->next;\
		(msg)->next = m->next, m->next = (void*)(msg);\
	}\
}

int rdecl
msgsend_gbl(THREAD *act, CONNECT *cop, void *msg, size_t size, unsigned priority, int coid) {
	CHANNELGBL *chp = (CHANNELGBL*) cop->channel;
	THREAD *thp;
	int xferstat;


	if(size > chp->buffer_size) {
		return EMSGSIZE;
	}

	// check boundry
	RD_VERIFY_PTR(act, msg, size);
	if(chp->num_curr_msg >= chp->max_num_buffer) {
		if(cop->flags & _NTO_COF_NONBLOCK) {
			return EAGAIN;
		}
		
		if(IMTO(act, STATE_SEND)) {
			return ETIMEDOUT;
		}

		//block waiting, or return EAGAIN
		act->args.ms.smsg= msg;
		act->args.ms.sparts= -size;
		lock_kernel();
		act->state = STATE_SEND;	
		block();
		act->args.ms.coid = coid;
		act->blocked_on = cop;
		pril_add(&chp->ch.send_queue, act);
		++cop->links;
	} else {
		struct gblmsg_entry *gblmsg;
		
re_send:
		if((thp = chp->ch.receive_queue)) {
			// somebody is waiting, deliver the message directly
			act->args.ms.smsg = msg;
			act->args.ms.sparts = -size;
			xferstat = xfermsg(thp, act, 0, 0);
			lock_kernel();
			act->restart = 0;
			if(xferstat) {
				if(xferstat & XFER_SRC_FAULT) {
					return EFAULT;
				}
				kererr(thp, EFAULT);
				LINKPRIL_REM(thp);
				ready(thp);
				unlock_kernel();
				KER_PREEMPT(act, ENOERROR);
				goto re_send;
			}
			SETKSTATUS(thp, size);
			thp->restart = NULL;
			LINKPRIL_REM(thp);
			ready(thp);

			if(thp->args.ri.info)  {
				thp->args.ri.cop  = cop;
				thp->args.ri.thp  = act;
				thp->args.ri.value = priority; // use value to store priority
				thp->args.ri.id = act->args.ms.msglen; // use id to store msglen
				thp->flags |= _NTO_TF_RCVINFO;
				// indicate that there is a receiver depending on data in our THREAD object
				act->internal_flags |= _NTO_ITF_SPECRET_PENDING;
			}

		} else {
			// put the message into a kernel queue
			// allocate a buffer
			if((gblmsg = chp->free) == NULL) {
				lock_kernel();
				gblmsg = (struct gblmsg_entry *)_smalloc(chp->buffer_size + sizeof(*gblmsg));
				if(!gblmsg) {
					return ENOMEM;
				}
				chp->free = gblmsg;
				unlock_kernel();
			}
			// queue a msg
			gblmsg->priority = priority;
			gblmsg->size = size;
			xferstat = xfer_memcpy((char*)gblmsg + sizeof(*gblmsg), msg, size);
			if(xferstat) {
				return EFAULT;
			}

			lock_kernel();

			if(chp->ch.reply_queue == NULL && chp->ev_prp) {
				// send out the event if any
				if(chp->ev_prp->valid_thp) {
					sigevent_exe(&chp->event, chp->ev_prp->valid_thp, 1);
				}
				chp->ev_prp = NULL;
			}
			LINK_PRI_MSG(chp->ch.reply_queue, chp->tail, gblmsg);
			chp->num_curr_msg++;
			chp->free = NULL;
		}
	}
	return ENOERROR;
}

int rdecl
msgreceive_gbl(THREAD *act, CHANNELGBL *chp, void *msg, size_t size, struct _msg_info *info, CONNECT *cop, int coid) {


	if(size < chp->buffer_size) {
		return EMSGSIZE;
	}
	
	// check boundry
	RD_VERIFY_PTR(act, msg, size);

	memset(info, 0, sizeof(*info));

	if(chp->num_curr_msg) {
		// receive msg
		struct gblmsg_entry *gblmsg;
		int xferstat;
		THREAD *thp;

		gblmsg = (struct gblmsg_entry *)chp->ch.reply_queue;
		if(gblmsg == NULL) crash();
		info->priority = gblmsg->priority;
		info->msglen = gblmsg->size;
		xferstat = xfer_memcpy(msg, (char*)gblmsg + sizeof(*gblmsg), gblmsg->size);

		if(xferstat) {
			return EFAULT;
		}

		lock_kernel();

		if((chp->ch.reply_queue = (void*)gblmsg->next) == NULL) {
			 chp->tail = NULL;
		}

		chp->num_curr_msg--;
		if(chp->free == NULL) {
			chp->free = gblmsg;
		} else {
			_sfree(gblmsg, chp->buffer_size + sizeof(*gblmsg));
		}

		// unblock a sender if necessary
		thp = pril_first(&chp->ch.send_queue);
		if(thp != NULL) {
			pril_rem(&chp->ch.send_queue, thp);
			ready(thp);
			KERCALL_RESTART(thp);
		}

		SETKSTATUS(act, info->msglen);
		return ENOERROR;
	} else {
		if(chp->ch.flags & _NTO_CHF_ASYNC_NONBLOCK) {
			return EAGAIN;
		}

		if(IMTO(act, STATE_RECEIVE)) {
			return ETIMEDOUT;
		}

		// block waiting
		act->args.ri.rmsg= msg;
		act->args.ri.rparts= -size;
		act->args.ri.info = info;
		lock_kernel();
		act->state = STATE_RECEIVE;
		block();
		act->blocked_on = cop?cop : (void*)chp;
		act->args.ri.id = coid;
		LINKPRIL_BEG(chp->ch.receive_queue, act, THREAD);
	}

	return ENOERROR;
}

int rdecl
msgsend_async(THREAD *act, CONNECT *cop) {

	if(!cop->cd) { 
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
		/* if the cop don't have a cd pointer, then another thread is 
	 	* in the middle of xfer this cop. This is a poor solution, but
	 	* restart this thread's kernel call
	 	*/
	 	KERCALL_RESTART(act); 
	 	return EOK; 
#else
		//it's never valid for cop->cd to be null on a single processor machine
		crash();
#endif
	}

	if(cop->cd->num_curmsg) {
		CHANNELASYNC *chp;
		THREAD *thp;
		
		chp = (CHANNELASYNC *)cop->channel;
		lock_kernel();
 		if(!(cop->flags & COF_ASYNC_QUEUED)) {
			CONNECT **owner;
			CONNECT	*curr;

			owner = (CONNECT**) &chp->ch.reply_queue;
			for( ;; ) {
				curr = *owner;
				if(curr == NULL) break;
				owner = &curr->next;
			}
			cop->next = NULL;
			*owner = cop;
 			cop->flags |= COF_ASYNC_QUEUED;
 		}

		// if there is already a receiver
		if((thp = chp->ch.receive_queue)) {
			LINKPRIL_REM(thp);
			ready(thp);
			KERCALL_RESTART(thp);
		} else {
			// kick the receiver queue if any
			if (chp->ev_prp) {
				if (chp->ev_prp->valid_thp) {
					sigevent_exe(&chp->event, chp->ev_prp->valid_thp, 1);
				}
			}
		}
	}

	return EOK;
}

#define CHECKBOUND(prp, p, size)      (((uintptr_t)(p) + (size) >= (uintptr_t)(p)) && WITHIN_BOUNDRY((uintptr_t)(p),(uintptr_t)(p)+(size),(prp)->boundry_addr))

int rdecl
msgreceive_async(THREAD *act, CHANNELASYNC *chp, iov_t *iov, unsigned parts) {

	CONNECT *cop;
	int status;
	struct _asyncmsg_connection_descriptor *cd, *lcd;
	struct _asyncmsg_get_header *ghp;
	struct _asyncmsg_put_header *php, *sendq;
	CONNECT **q;
	IOV     dstiov[1], srciov[1], *srcp;
	int     dparts, sparts, soff, nbytes;
	

	/* scan iov list to find out memory errors in header */
	if (!act->restart) {
		act->args.ms.sparts = 0;
	}

restart:
	/* xfer msg */
	while((cop = (CONNECT*)chp->ch.reply_queue) && (act->args.ms.sparts < parts)) {

		lcd = cop->cd;

#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
		/* if the cop don't have a cd pointer, then another thread is 
		 * in the middle of xfer this cop. We move to next cop.
		 */
		
		if (lcd == NULL) {
			continue;
		}
#endif

		if (act->process == cop->process) {
			cd = lcd;
		} else {
			SETIOV(srciov, lcd, sizeof(*lcd) + sizeof(struct _asyncmsg_put_header) * cop->sendq_size);
			if(!CHECKBOUND(cop->process, lcd, GETIOVLEN(srciov))) {
				return EFAULT;
			} 
			srcp = srciov;
			sparts = 1;
			soff = 0;
			dparts = 1;
			SETIOV(dstiov, 0, 0);
			nbytes = memmgr.map_xfer(act->process,
									 cop->process, 
									 (IOV **)&srcp,
									 &sparts,
									 &soff,
									 dstiov,
									 &dparts,
									 MAPADDR_FLAGS_IOVKERNEL);
			if(nbytes < 0) {
				return EFAULT;			
			}
			cd = dstiov[0].iov_base;
		}
		sendq = (struct _asyncmsg_put_header *)((char *)cd + sizeof(*cd));
		
		/*
		 * Check make sure *cd is read/writeable
		 */
		RD_PROBE_INT(act, cd, sizeof(*cd) / sizeof(int));
		WR_PROBE_INT(act, cd, sizeof(*cd) / sizeof(int));
		
		/* if act has been preempted, and somebody else touched cop, or
		 *    cop is not the one last time act operated,
		 * reset the args.ms.msglen so we start from a fresh xfer.
		 */
		if ((THREAD *)cop->restart != act || act->restart != (THREAD *)cop)
		{
			act->args.ms.msglen = 0;			
		}
		
		act->restart = (THREAD *)cop;
		cop->restart = act;
		
		q = (CONNECT **)&chp->ch.reply_queue;
		iov += act->args.ms.sparts;
		while((cd->sendq_head != cd->sendq_tail) && (act->args.ms.sparts < parts)) {
			ghp = (struct _asyncmsg_get_header *)GETIOVBASE(iov);
			php = &sendq[cd->sendq_head];

			if (!WITHIN_BOUNDRY((uintptr_t)(ghp),(uintptr_t)(ghp)+(sizeof(*ghp)),act->process->boundry_addr))
			{
				lock_kernel();
				act->restart = NULL;
				cop->restart = NULL;
				/* if some message already transfered, let sender know */
				if (act->args.ms.sparts) {
					if (cop->process->valid_thp) {
						sigevent_exe(&cd->ev, cop->process->valid_thp, 1);
					}
					SETKSTATUS(act, act->args.ms.sparts);
					unlock_kernel();
					return ENOERROR;
				} else {
					unlock_kernel();
					return EFAULT;
				}
			}

			/* see if we need to set srcmsglen */
			memset(&ghp->info, 0, sizeof(struct _msg_info));
			
			/* deliver a msg */
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
			act->args.ms.rparts = (uintptr_t)lcd; //BUG? Dead code? We dont seem to read lcd back out of rparts later. Broken  restart mechanism?
			*(volatile int_fl_t*) &(act->internal_flags) |= _NTO_ITF_MSG_DELIVERY;
			cop->cd = NULL;
			//BUG? what if we are prempted at this point. will cop->cd ever have it's value restored? Could explain loss of 
			//messages in SMP. 
#endif
			status = rcvmsg(act, cop->process, ghp->iov, ghp->parts, php->iov, php->parts);
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
			cop->cd = lcd;
			*(volatile int_fl_t*) &(act->internal_flags) &= ~_NTO_ITF_MSG_DELIVERY;
#endif
			/* restore cd mapping in case it is invalid */
			/* FUTURE: can check the address range to see if it is in one to one mapping area. 
				   Later will get a permanent mapping area for it */
			if (act->process != cop->process) {
				SETIOV(srciov, lcd, sizeof(*lcd) + sizeof(struct _asyncmsg_put_header) * cop->sendq_size);
				if(!CHECKBOUND(cop->process, lcd, GETIOVLEN(srciov))) {
					return EFAULT;
				} 
				srcp = srciov;
				sparts = 1;
				soff = 0;
				dparts = 1;
				SETIOV(dstiov, 0, 0);
				nbytes = memmgr.map_xfer(act->process,
									 cop->process, 
									 (IOV **)&srcp,
									 &sparts,
									 &soff,
									 dstiov,
									 &dparts,
									 MAPADDR_FLAGS_IOVKERNEL);
				if(nbytes < 0) {
					return EFAULT;			
				}
				cd = dstiov[0].iov_base;
				sendq = (struct _asyncmsg_put_header *)((char *)cd + sizeof(*cd));
			}
		
			if(status) {
				if(status & XFER_SRC_FAULT) {
					/* sender address fault */
					php->err = EFAULT;
					lock_kernel();
					/* adjust header: put into a macro later */
					cd->num_curmsg --;
					if(cd->sendq_head + 1 < cd->sendq_size) {
						cd->sendq_head ++;
					} else {
						cd->sendq_head = 0;
					}
					/* sender is wrong, take the connection off, let send know */
					LINK1_REM(*q, cop, CONNECT);
					act->restart = NULL;
					cop->restart = NULL;
 					cop->flags &= ~COF_ASYNC_QUEUED;
					if (cop->process->valid_thp) {
						sigevent_exe(&cd->ev, cop->process->valid_thp, 1);
					}
					
					/* 
					 * if receiver already got some messages, return it.
					 * if receiver got nothing, go try next cop
					 */
					if (act->args.ms.sparts) {
						if (cop->process->valid_thp) {
							sigevent_exe(&cd->ev, cop->process->valid_thp, 1);
						}
						SETKSTATUS(act, act->args.ms.sparts);
						unlock_kernel();
						return ENOERROR;
					} else {
						unlock_kernel();
						goto restart;
					}
				} else {
					/* receiver address fault */
					ghp->err = EFAULT;
					lock_kernel();
					act->restart = NULL;
					cop->restart = NULL;
					/* if some message already transfered, let sender know */
					if (act->args.ms.sparts) {
						if (cop->process->valid_thp) {
							sigevent_exe(&cd->ev, cop->process->valid_thp, 1);
						}
						/* increment receiver count, so recever would see EFAULT */
						act->args.ms.sparts++;
						SETKSTATUS(act, act->args.ms.sparts);
						unlock_kernel();
						return ENOERROR;
					} else {
						unlock_kernel();
						return EFAULT;
					}
				}
			}

			/* fill in msg info */
			ghp->info.pid = cop->process->pid;
			ghp->info.chid = chp->ch.chid;
			ghp->info.msglen = act->args.ms.msglen;

			lock_kernel();
			/* set restart point */
			act->args.ms.msglen = 0;
			act->args.ms.sparts++;
			iov++;
			/* adjust header */
			cd->num_curmsg --;
			if(cd->sendq_head + 1 < cd->sendq_size) {
				cd->sendq_head ++;
			} else {
				cd->sendq_head = 0;
			}
			/* check preemption */
			if(NEED_PREEMPT(act)) {
				if (cop->process->valid_thp) {
					sigevent_exe(&cd->ev, cop->process->valid_thp, 1);
				}
				SETKSTATUS(act, act->args.ms.sparts);
				act->restart = NULL;
				cop->restart = NULL;
				return ENOERROR;
			}
			unlock_kernel();
		}

		lock_kernel();
		if(cd->sendq_head == cd->sendq_tail) {
			/* done with this cop */
			LINK1_REM(*q, cop, CONNECT);
			cop->restart = NULL;
 			cop->flags &= ~COF_ASYNC_QUEUED;
		}	
		if (cop->process->valid_thp) {
			sigevent_exe(&cd->ev, cop->process->valid_thp, 1);
		}
		unlock_kernel();
	}

	lock_kernel();
	if(!act->args.ms.sparts) {
		if(chp->ch.flags & _NTO_CHF_ASYNC_NONBLOCK) {
			return EAGAIN;
		}

		if(IMTO(act, STATE_RECEIVE)) {
			return ETIMEDOUT;
		}

		// block waiting
		act->args.ri.rmsg = iov;
		act->args.ri.rparts = parts;
		act->args.ri.info = NULL;
		lock_kernel();
		act->state = STATE_RECEIVE;
		block();
		act->blocked_on = (void*)chp;
		LINKPRIL_BEG(chp->ch.receive_queue, act, THREAD);
	} else {
		SETKSTATUS(act, act->args.ms.sparts);
		act->restart = NULL;
		if(cop) {
			cop->restart = NULL;
		}
	}
	return ENOERROR;
}

__SRCVERSION("nano_asyncmsg.c $Rev: 169208 $");
