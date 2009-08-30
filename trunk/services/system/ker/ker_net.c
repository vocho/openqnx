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
ker_net_cred(THREAD *act, struct kerargs_net_cred *kap) {
	CONNECT		*cop;
	CLIENT		*cip;

	// Lookup src connect.
	if((cop = lookup_connect(kap->coid)) == NULL  ||
		cop->type != TYPE_CONNECTION  ||  (cop->flags & COF_NETCON) == 0  ||
		act->process != net.prp) {
		return EBADF;
	}

	if((cip = kap->info)) {
		unsigned	size;

		size = offsetof(CLIENT, cred.grouplist) +
			cip->cred.ngroups * sizeof(cip->cred.grouplist);

		RD_VERIFY_PTR(act, cip, size);
		RD_PROBE_INT(act, cip, size / sizeof(int));

		lock_kernel();

		if(cred_set(&cop->un.net.cred, &cip->cred) != EOK) {
			return EAGAIN;
		}

		cop->un.net.nd = cip->nd;
		cop->un.net.pid = cip->pid;
		cop->un.net.sid = cip->sid;
		cop->un.net.flags = cip->flags;
	}
	return EOK;
}


int kdecl
ker_net_vtid(THREAD *act, struct kerargs_net_vtid *kap) {
	VTHREAD		*vthp;
	int			id;
	int			vtid = kap->vtid;	// kap->vtid may be corrupted by SETKSTATUS

	if(kap->info) {
		RD_VERIFY_PTR(act, kap->info, sizeof(*kap->info));
		RD_PROBE_INT(act, kap->info, sizeof(*kap->info) / sizeof(int));
	}
	if(act->process != net.prp) {
		return EINVAL;
	}

	lock_kernel();
	SETKSTATUS(act, 0);

	// If info pointer is NULL we remove the the vtid.
	if(kap->info == NULL) {
	 	if((vthp = vector_rem(&vthread_vector, vtid))) {
			force_ready((THREAD *)(void *)vthp, EINTR | _FORCE_SET_ERROR);
			object_free(NULL, &vthread_souls, vthp);
		}

		return ENOERROR;
	}

	// This should never occur, but it would be a bug in the
	// network manager, so don't crash the kernel.
	if((vthp = vector_lookup(&vthread_vector, vtid))) {
#ifndef NDEBUG
		crash();
		/* NOTREACHED */
#endif
		return EBUSY;
	}

 	// Add a new vtid and its associated info.
	if((vthp = object_alloc(NULL, &vthread_souls)) == NULL) {
		return EAGAIN;
	}

	vthp->next.thread = NULL;
	vthp->type = TYPE_VTHREAD;
	vthp->dpp = net.prp->default_dpp;
	vthp->tid = kap->info->tid - 1;
	vthp->flags |=_NTO_TF_TO_BE_STOPPED;	// To stop it from ever running
	vthp->state = STATE_STOPPED;
	vthp->args.ms.coid = kap->info->coid;
	vthp->process = act->process;
	vthp->aspace_prp = act->aspace_prp;
	vthp->un.net.vtid = vtid;

 	if((id = vector_add(&vthread_vector, vthp, vtid)) != vtid) {
 		if(id == -1) {
 			kererr(act, EAGAIN);
 		} else {
 			vector_rem(&vthread_vector, id);
 			kererr(act, EBUSY);
		}
		object_free(NULL, &vthread_souls, vthp);
	}
#if defined(VARIANT_instr)
	  else {
		_TRACE_TH_EMIT_CREATE(vthp);
		_TRACE_TH_EMIT_STATE(vthp, STOPPED);
	}
#endif
	return ENOERROR;
}


int kdecl
ker_net_unblock(THREAD *act, struct kerargs_net_unblock *kap) {
	VTHREAD	*vthp;
	int		status;

	// if not called by net, any errno will do as long as it fails...
	if(		act->process != net.prp ||
			(vthp = vector_lookup(&vthread_vector, kap->vtid)) == NULL) {
		return ESRCH;
	}
	if(vthp->state != STATE_SEND && vthp->state != STATE_REPLY) {
		return EBUSY;
	}

	lock_kernel();
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
	vthp->internal_flags |= _NTO_ITF_NET_UNBLOCK;
#endif
	// Don't do force_ready in SETKSTATUS - might be invoked twice
	status = force_ready(vthp, EINTR);
	SETKSTATUS(act, status);
#if defined(VARIANT_smp) && defined(SMP_MSGOPT)
	vthp->internal_flags &= ~_NTO_ITF_NET_UNBLOCK;
#endif
	return ENOERROR;
}


int kdecl
ker_net_infoscoid(THREAD *act, struct kerargs_net_infoscoid *kap) {
	CONNECT	*cop;

	// if not called by net, any errno will do as long as it fails...
	if(		act->process != net.prp ||
			// @@@ This should be "kap->scoid ^ _NTO_SIDE_CHANNEL", but old kernels used
			// @@@ rcvid. "kap->scoid & 0xffff" handles old rcvid and new scoid...
			(cop = vector_lookup(&act->process->chancons, kap->scoid & 0xffff)) == NULL ||
			(cop->type != TYPE_CONNECTION) ||
			(cop->process == NULL)) {
		return ESRCH;
	}

	lock_kernel();

	SETKSTATUS(act, cop->infoscoid);
	cop->infoscoid = kap->infoscoid;

	if(cop->links && kap->infoscoid == -1) {
		// Null channel pointer since it is gone on the other end.
		if(cop->channel != NULL) {
			PROCESS				*prp = cop->channel->process;
			CONNECT				*cop2;
			THREAD *thp;
			int tid, fd;

			// Network manager should think all connections are disconnected,
			// and the client should think the channel is detached.
			// We do this by using two connect objects, one for the network manager
			// and one for the client.
			vector_rem(&prp->chancons, cop->scoid);

			// Allocate a new conect entry for network manager.
			if((cop2 = object_alloc(prp, &connect_souls)) == NULL) {
				crash();
			}
			*cop2 = *cop;
			cop2->process = 0;
			cop2->links = 0;
			vector_add(&prp->chancons, cop2, cop2->scoid);

			// Connections were removed, so cause disconnect pulse to net
			connect_detach(cop2, act->priority);

			// Unblock all threads send/reply blocked on the cop
			// Have to do this before we start NULL'ing out the cop->channel
			// pointers so that force_ready can find the appropriate
			// PRIL_HEAD structure.
			for(tid = 0 ; tid < cop->process->threads.nentries ; ++tid) {
				if((thp = VECP(thp, &cop->process->threads, tid))  &&
					((thp->state == STATE_SEND) || (thp->state == STATE_REPLY) ||
					 (thp->state == STATE_NET_SEND) || (thp->state == STATE_NET_REPLY))) {
						if((thp->blocked_on == cop) ||
							((((CONNECT*)thp->blocked_on)->type == TYPE_CONNECTION) &&
							 (((CONNECT*)thp->blocked_on)->un.lcl.cop == cop))) {
							if(thp->state == STATE_REPLY) {
								thp->state = STATE_NET_REPLY;
							}
							force_ready(thp, ESRCH);
						}
				}
			}

			// Now make any client accesses fail
			for(fd = 0 ; fd < cop->process->fdcons.nentries ; ++fd) {
				if(VECAND(cop2 = VEC(&cop->process->fdcons, fd), 1) == 0) {
					cop2 = VECAND(cop2, ~3);
					if(cop2->scoid == (kap->scoid & 0xffff)
					  && cop2->channel == net.chp) {
						cop2->channel = NULL;
					}
				}
			}

			for(fd = 0 ; fd < cop->process->chancons.nentries ; ++fd) {
				if(VECAND(cop2 = VEC(&cop->process->chancons, fd), 1) == 0) {
					cop2 = VECAND(cop2, ~3);
					if((cop2->type == TYPE_CONNECTION) && (cop2->scoid == (kap->scoid & 0xffff))
					  && cop2->channel == net.chp) {
						cop2->channel = NULL;
					}
				}
			}

			cop->channel = NULL;
		}
		// Channel was removed, so cause coiddeath pulses
		if(cop->process->death_chp) {
			connect_coid_disconnect(cop, cop->process->death_chp, act->priority);
		}
	}

	return ENOERROR;
}

int kdecl
ker_net_signal_kill(THREAD *act, struct kerargs_net_signal_kill *kap) {
	PROCESS *prp;
	struct kerargs_signal_kill args;
	struct _cred_info *src, *dst;

	/* this is a struct agreed between QNET and kernel */
	struct signal_info {
		unsigned nd;
		pid_t    pid;
		int32_t  tid;
		int32_t  signo;
		int32_t  code;
		int32_t  value;
	} *info;

	if(act->process != net.prp) {
		return EINVAL;
	}

	info = kap->sigdata;
	args.pid = info->pid;
	args.tid = info->tid;
	args.signo = info->signo;
	args.code = info->code;
	args.value = info->value;

	// Verify the target process exists
	if(args.pid == -1  ||  (prp = lookup_pid(args.pid < 0 ? -args.pid : args.pid)) == NULL) {
		return ESRCH;
	}

	// Verify we have the right to hit the target process
	src = kap->cred;
	dst = &prp->cred->info;
	if(!(src->euid == 0  ||
	   src->ruid == dst->ruid  ||
	   src->ruid == dst->suid  ||
	   src->euid == dst->ruid  ||
	   src->euid == dst->suid)) {
		return EPERM;
	}

	// send the signal
	return do_ker_signal_kill(act, &args);
}

__SRCVERSION("ker_net.c $Rev: 207484 $");
