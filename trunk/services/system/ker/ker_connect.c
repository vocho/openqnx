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
#include <sys/procmsg.h>

int kdecl
ker_connect_attach(THREAD *act, struct kerargs_connect_attach *kap) {
	PROCESS				*prpc, *prps;
	CHANNEL				*chp;
	CONNECT				*cop, *cop_master = NULL;
	VECTOR				*vec, *newvec;
	int					i, coid, scoid, chid, special;
	unsigned			status;

	prpc = act->process;
	prps = kap->pid ? lookup_pid(kap->pid) : prpc;
	special = prpc->pid == PROCMGR_PID && !kap->flags;

	if (kap->flags & ~(_NTO_COF_CLOEXEC|_NTO_COF_DEAD |_NTO_COF_NOSHARE |
						_NTO_COF_NETCON |_NTO_COF_NONBLOCK |_NTO_COF_ASYNC |
						_NTO_COF_GLOBAL)) {
		/* invalid flags specified */
		return EINVAL;
	}

	// Don't let them pick an index for side channels. They always get first avail.
	if(kap->index & _NTO_SIDE_CHANNEL) {
		kap->index = _NTO_SIDE_CHANNEL;
	// Make sure they don't pick one beyond their limits.
	} else if(kap->index >= prpc->rlimit_vals_soft[RLIMIT_NOFILE]) {
		return EINVAL;
	}
	// Make sure they are allowed more connections.
	// Note: use '>' instead of '>=' to take into account the proc channel
	// that is created for each process.
	// This allows a process to open RLIMIT_NOFILE connections, even though
	// the proc channel is accounted for in prpc->nfds
	if(!special && prpc->nfds > prpc->rlimit_vals_soft[RLIMIT_NOFILE]) {
		return EMFILE;
	}

	// Check for a remote node.
	if(ND_NODE_CMP(kap->nd, ND_LOCAL_NODE) != 0) {
		if((prps = net.prp) == 0   ||  (chp = net.chp) == 0) {
			return ENOREMOTE;
		}
		goto common;
	}

	// Make sure process is valid.
	if(prps == NULL) {
		return ESRCH;
	}

	kap->pid = prps->pid;

	vec = &prps->chancons;
	if((chid = kap->chid) & _NTO_GLOBAL_CHANNEL) {
		vec = &chgbl_vector;
		chid &= ~_NTO_GLOBAL_CHANNEL;
	}

	// Make sure channel is valid.
	if((chp = vector_lookup(vec, chid)) == NULL  ||  chp->type != TYPE_CHANNEL) {
		return ESRCH;
	}

common:
	if(prps->flags & (_NTO_PF_TERMING | _NTO_PF_ZOMBIE | _NTO_PF_COREDUMP)) {
		return ENXIO;
	}

	newvec = (kap->index & _NTO_SIDE_CHANNEL)	? &prpc->chancons
												: &prpc->fdcons;

	// The network manager uses this to prevent connection sharing.
	if((kap->flags & (_NTO_COF_NOSHARE | _NTO_COF_NETCON)) == 0) {
		// Scan for an existing connection in either channel vector.
		for(vec = &prpc->fdcons;;) {
			for(i = 0; i < vec->nentries; ++i) {
				if((cop = VECP2(cop, vec, i))) {
					if(cop->type == TYPE_CONNECTION  &&
					   cop->process == prpc &&
					   cop->channel &&
					   cop->links &&
					   (cop->flags & COF_NETCON) == 0 &&
					   cop->un.lcl.pid == kap->pid  &&
					   cop->un.lcl.chid == kap->chid &&
					   ND_NODE_CMP(cop->un.lcl.nd, kap->nd) == 0) {
						if(cop->un.lcl.nd != kap->nd) {
							/* same connection, different quality of service */
							if(cop_master == NULL) {
								cop_master = (cop->flags & COF_VCONNECT)?cop->un.lcl.cop:cop;
							}
						} else {							
							lock_kernel();
							if((coid = vector_add(newvec, cop, kap->index & ~_NTO_SIDE_CHANNEL)) == -1) {
								return EAGAIN;
							}

							// Check coid is allowed
							if((kap->index & _NTO_SIDE_CHANNEL) == 0 && !special && coid >= prpc->rlimit_vals_soft[RLIMIT_NOFILE]) {
								vector_rem(newvec, coid);
								return EMFILE;
							}
		
							++cop->links;
		
							if(kap->flags & _NTO_COF_CLOEXEC) {
								vector_flag(newvec, coid, 1);
							}
		
							status = coid | (kap->index & _NTO_SIDE_CHANNEL);
							SETKSTATUS(act, status);
							prpc->nfds++;
		
							return ENOERROR;
						}
					}
				}
			}
	
			if(vec == &prpc->chancons) {
				break;
			}
	
			vec = &prpc->chancons;
		}
	}	
	
	if(chp->flags & _NTO_CHF_ASYNC) {
		WR_PROBE_INT(act, kap->cd, sizeof(struct _asyncmsg_connection_descriptor) / sizeof(int));
	}

	lock_kernel();

	// Allocate a new conection.
	if((cop = object_alloc(prpc, &connect_souls)) == NULL) {
		return ENFILE;
	}

	// Add connect to the clients channel vector.
	if((coid = vector_add(newvec, cop, kap->index & ~_NTO_SIDE_CHANNEL)) == -1) {
		object_free(prpc, &connect_souls, cop);
		return EAGAIN;
	}

	// Check coid is allowed
	if((kap->index & _NTO_SIDE_CHANNEL) == 0 && !special && coid >= prpc->rlimit_vals_soft[RLIMIT_NOFILE]) {
		vector_rem(newvec, coid);
		object_free(prpc, &connect_souls, cop);
		return EMFILE;
	}

	// Add connect to the servers channel vector.
scoid = 0;
if(!(chp->flags & _NTO_CHF_GLOBAL)) {
	if(cop_master) {
		// same connection, different quality of service
		scoid = cop_master->scoid;
		cop_master->links++;
		cop->flags |= COF_VCONNECT;
		cop->un.lcl.cop = cop_master;
		LINK1_BEG(cop_master->next, cop, CONNECT);
	} else {
		if((scoid = vector_add(&prps->chancons, cop, 1)) == -1) {
			vector_rem(newvec, coid);
			object_free(prpc, &connect_souls, cop);
			return EAGAIN;
		}
		cop->next = NULL;
	}
	if(chp->flags & _NTO_CHF_ASYNC) {
		cop->cd = kap->cd;
		cop->sendq_size = cop->cd->sendq_size;
	}
} else {
	// link the connection to the global channel queue
	LINK1_BEG(((CHANNELGBL*)chp)->cons, cop, CONNECT);
	cop->flags |= _NTO_COF_GLOBAL;
}

	cop->type = TYPE_CONNECTION;
	cop->process = prpc;
	cop->links = 1;
	cop->channel = chp;
	cop->scoid = scoid;
	cop->infoscoid = -1;
	if(ND_NODE_CMP(kap->nd, ND_LOCAL_NODE) == 0) {
		// In the remote case we don't know this information at this time.
		cop->infoscoid = scoid;
	}

	// Network manager allocates a place to hold remote process info.
	if(kap->flags & _NTO_COF_NETCON) {
		if(prpc != net.prp || (kap->index & _NTO_SIDE_CHANNEL) == 0) {
			vector_rem(&prps->chancons, scoid);
			vector_rem(newvec, coid);
			object_free(prpc, &connect_souls, cop);
			return EINVAL;
		}
		cop->flags |= COF_NETCON;
		cop->un.net.coid = coid;
		// cred, nd, pid, sid, flags will be set with ker_net_cred.
	} else {
		cop->un.lcl.nd = kap->nd;
		cop->un.lcl.pid = kap->pid;
		cop->un.lcl.chid = kap->chid;
	}

	if(kap->flags & _NTO_COF_CLOEXEC) {
		vector_flag(newvec, coid, 1);
	}

	status = coid | (kap->index & _NTO_SIDE_CHANNEL);
	SETKSTATUS(act, status);

	prpc->nfds++;

	return ENOERROR;
}


int kdecl
ker_connect_detach(THREAD *act, struct kerargs_connect_detach *kap) {
	PROCESS	*prp = act->process;
	CONNECT	*cop;
	CHANNEL	*chp;
	VECTOR	*vec;
	int		coid;

	// Make sure the connection is valid.
	vec = (kap->coid & _NTO_SIDE_CHANNEL) ? &prp->chancons : &prp->fdcons;
	coid = kap->coid & ~_NTO_SIDE_CHANNEL;
	if((cop = vector_lookup2(vec, coid)) == NULL  ||  cop->type != TYPE_CONNECTION  ||
	   ((chp = cop->channel) && chp->process == prp  &&
	   kap->coid == (cop->scoid | _NTO_SIDE_CHANNEL)  &&  cop->links != 0)) {
		return EINVAL;
	}
		
	lock_kernel();


	// unblock any other threads that are blocked on the same connection with EBADF
	// we don't need to check for scoid because scoid can not be in send/reply queues.
	// COF_NETCON connections store the local coid in args.ms.coid.
	if(prp->num_active_threads > 1) {
		int			tid;

		for(tid = 0; tid < prp->threads.nentries; tid++) {
			THREAD		*thp;

			if((thp = VECP(thp, &prp->threads, tid)) &&
					((thp->args.ms.coid == kap->coid) &&
					 (thp->state == STATE_SEND || 
						(thp->state == STATE_REPLY &&
						!(((CONNECT *)thp->blocked_on)->flags & COF_NETCON)))
					 || (thp->state == STATE_RECEIVE && thp->args.ri.id == kap->coid
						&& (((CONNECT *)thp->blocked_on)->type == TYPE_CONNECTION)
						&& (((CONNECT *)thp->blocked_on)->flags & _NTO_COF_GLOBAL)))) {
				force_ready(thp, EBADF);
			}
			KER_PREEMPT(act, ENOERROR);
		}
	}
	if(cop->flags & COF_NETCON) {
		int			i, prio;
		THREAD		*thp;

		for(i = 0; i < vthread_vector.nentries; ++i) {
			VTHREAD		*vthp;
			CONNECT 	*cop1;

			KER_PREEMPT(act, ENOERROR);

			vthp = VECP(vthp, &vthread_vector, i);
			if(vthp == NULL) continue;
			if(vthp->args.ms.coid != kap->coid) continue;
			switch(vthp->state) {
			case STATE_SEND:
			case STATE_REPLY:
				break;
			default:
				continue;
			}
			cop1 = vthp->blocked_on;
			if(cop1 != cop) {
				if(!(cop1->flags & COF_VCONNECT)) continue;
				if(cop1->un.lcl.cop != cop) continue;
			}
			vector_rem(&vthread_vector, i);
			if(force_ready((THREAD *)(void *)vthp, EBADF)) {
				object_free(NULL, &vthread_souls, vthp);
			}
		}
		
		// connect detach pulse for QNET
		if((act->process == net.prp) && ((cop->channel && cop->channel->process != net.prp) ||  (cop->scoid != coid))) {
			prio = act->priority;
			if(cop->channel) {
				for(i = 0 ; i < cop->channel->process->threads.nentries ; ++i) {
					thp = VECP(thp, &cop->channel->process->threads, i);
					if((thp != NULL) && (thp->state == STATE_REPLY) && (thp->blocked_on == cop)) {
						if(prio > thp->priority) {
							prio = thp->priority;
						}
						force_ready(thp, ESRCH | _FORCE_NO_UNBLOCK);
					}
				}
			} 
			if(net.chp) {
				pulse_deliver(net.chp, prio, _PULSE_CODE_NET_DETACH, kap->coid, 0, 0);
			}	
		}
	}

	vector_rem(vec, coid);
	if(cop->links == 0) {
		// It is a server connect, just free it.
        	if (cop->channel && (cop->channel->flags & _NTO_CHF_ASYNC)) {
			LINK1_REM( *(CONNECT **)(&((CHANNELASYNC *)cop->channel)->ch.reply_queue), cop, CONNECT);
		}
		object_free(prp, &connect_souls, cop);
	} else {
		// Decrement the link count and return if > 0.
		if(!(cop->flags & _NTO_COF_GLOBAL)) { // no scoid and disconnect pulse for gbl channel connections
			if(--cop->links == 0) {
					connect_detach(cop, act->real_priority);
			}
		} else {
			if(cop->channel && (((CHANNELGBL*)cop->channel)->ev_coid == kap->coid)) {
				/* get rid of any event registered */
				((CHANNELGBL*)cop->channel)->ev_prp = NULL;
			}
			if(--cop->links == 0) {
				if(cop->channel) {
					LINK1_REM(((CHANNELGBL*)cop->channel)->cons, cop, CONNECT);
				}
				object_free(cop->process, &connect_souls, cop);
			}
		}
		act->process->nfds--;
	}

	SETKSTATUS(act, 0);

	return ENOERROR;
}



int kdecl
ker_connect_server_info(THREAD *act, struct kerargs_connect_server_info *kap) {
	PROCESS	*prp;
	VECTOR	*vec;
	CONNECT	*cop;
	CHANNEL	*chp;
	unsigned coid;
	unsigned flags;
	struct _server_info *sep;

	// Verify the target process exists.
	prp = kap->pid ? lookup_pid(kap->pid) : act->process;
	if(prp == NULL) {
		return ESRCH;
	}

	// Get correct connection vector.
	vec = (kap->coid & _NTO_SIDE_CHANNEL) ? &prp->chancons : &prp->fdcons;
	coid = kap->coid & ~_NTO_SIDE_CHANNEL;


	// Search for next connection (making sure to avoid server connections).
	chp = NULL;
	for(;;) {
		while((cop = vector_search(vec, coid, &coid))) {
			if(cop->type == TYPE_CONNECTION  &&  !((chp = cop->channel)  &&
			   chp->process == prp  &&  coid == cop->scoid  &&  vec == &prp->chancons)) {
				// Only report connection if channel is valid or info is requested
				if(chp || kap->info) {
					break;
				}
			}
			coid++;
		}

		if(cop) {
			break;
		}

		if(vec == &prp->fdcons) {
			vec = &prp->chancons;
			coid = 0;
		} else {
			// No more connections found
			return EINVAL;
		}
	}

	// save flags for later
	flags = VECAND(VEC(vec, coid), 2) ? _NTO_COF_CLOEXEC : 0;
	if(!chp) {
		flags |= _NTO_COF_DEAD;
	}

	// Was it from the side channel vector?
	if(vec == &prp->chancons) {
		coid |= _NTO_SIDE_CHANNEL;
	}


	if((sep = kap->info)) {
		WR_VERIFY_PTR(act, sep, sizeof(*sep));
		WR_PROBE_OPT(act, sep, sizeof(*sep) / sizeof(int));
		if(cop->flags & COF_NETCON) {
			sep->nd = cop->un.net.nd;
			sep->pid = cop->un.net.pid;
			sep->chid = -1;
		} else {
			sep->nd = cop->un.lcl.nd;
			sep->pid = cop->un.lcl.pid;
			sep->chid = cop->un.lcl.chid;
		}
		if(cop->flags & COF_VCONNECT) {
			sep->scoid = cop->un.lcl.cop->infoscoid | _NTO_SIDE_CHANNEL;
		} else {
			sep->scoid = cop->infoscoid | _NTO_SIDE_CHANNEL;
		}
		sep->coid = coid;
		sep->flags = flags;
	}

	lock_kernel();
	SETKSTATUS(act, coid);
	return ENOERROR;
}



int kdecl
ker_connect_client_info(THREAD *act, struct kerargs_connect_client_info *kap) {
	PROCESS	*prp = act->process;
	CONNECT	*cop;
	CHANNEL	*chp;
	CLIENT	*clp;
	int		scoid = kap->scoid & ~_NTO_SIDE_CHANNEL;

	// If scoid is -1 it get info on calling process
	cop = NULL;
	if(kap->scoid != -1) {
		if((kap->scoid & _NTO_SIDE_CHANNEL) == 0
		|| (cop = vector_lookup(&prp->chancons, scoid)) == NULL
		|| (cop->type != TYPE_CONNECTION)
		|| (chp = cop->channel) == NULL
		|| (chp->process != prp)
		|| (cop->scoid != scoid)) {
			return EINVAL;
		}

		prp = cop->process;
	}

	if((clp = kap->info)) {
		struct _cred_info	*cip;
		unsigned			 ngroups;

		WR_VERIFY_PTR(act, clp, sizeof(*clp));
		WR_PROBE_OPT(act, clp, sizeof(*clp) / sizeof(int));

		if(cop && (cop->flags & COF_NETCON) != 0) {
			CREDENTIAL			*crp;

			if(!(crp = cop->un.net.cred)) {
				// Not a complete connection yet!!
				return EINVAL;
			}
			cip = &crp->info;
			clp->nd = cop->un.net.nd;
			clp->pid = cop->un.net.pid;
			clp->sid = cop->un.net.sid;
			clp->flags = cop->un.net.flags;
		} else {
			SESSION				*sep;

			clp->nd = ND_LOCAL_NODE;
			clp->pid = prp->pid;
			clp->sid = 0;
#if defined(__BIGENDIAN__)
			clp->flags = _NTO_CI_ENDIAN_BIG;
#elif defined(__LITTLEENDIAN__)
			clp->flags = 0;
#else
#error ENDIAN Not defined for system
#endif
			if((sep = prp->session)) {
				clp->sid = sep->leader;
				if(sep->pgrp != prp->pgrp) {
					clp->flags |= _NTO_CI_BKGND_PGRP;
				}
			}
			if(prp->flags & _NTO_PF_ORPHAN_PGRP) {
				clp->flags |= _NTO_CI_ORPHAN_PGRP;
			}
			if(prp->flags & _NTO_PF_STOPPED) {
				clp->flags |= _NTO_CI_STOPPED;
			}
			cip = &prp->cred->info;
		}
		ngroups = min(kap->ngroups, cip->ngroups);
		memcpy(&clp->cred, cip,
			offsetof(struct _cred_info, grouplist) +
			ngroups * sizeof(clp->cred.grouplist[0]));
		//If users passes 0 for number of groups, return the count
		clp->cred.ngroups = (kap->ngroups) ? ngroups : cip->ngroups;
	}

	return EOK;
}


int kdecl
ker_connect_flags(THREAD *act, struct kerargs_connect_flags *kap) {
	PROCESS		*prp;
	VECTOR		*vec;
	void		*ptr;
	unsigned 	old;
	unsigned	coid;

	// Lookup process.
	prp = kap->pid ? lookup_pid(kap->pid) : act->process;
	if(prp == NULL) {
		return ESRCH;
	}

	// Check for perms.
	if(kap->mask && !kerisusr(act, prp)) {
		return ENOERROR;
	}

	// Lookup connect.
	vec = (kap->coid & _NTO_SIDE_CHANNEL) ? &prp->chancons : &prp->fdcons;
	coid = kap->coid & ~_NTO_SIDE_CHANNEL;
	if(coid >= vec->nentries  ||  VECAND(ptr = VEC(vec, coid), 1)) {
		return EBADF;
	}

	old = VECAND(ptr, 2) ? _NTO_COF_CLOEXEC : 0;

	lock_kernel();

	if(kap->mask & _NTO_COF_CLOEXEC) {
		vector_flag(vec, coid, (kap->bits & _NTO_COF_CLOEXEC) ? 1 : 0);
	}

	SETKSTATUS(act, old);
	return ENOERROR;
}

int kdecl
ker_channel_connect_attrs(THREAD *act, struct kerargs_channel_connect_attr *kap) {

	if(kap->old_attrs) {
		WR_VERIFY_PTR(act, kap->old_attrs, sizeof(*kap->old_attrs));
	}

	if(kap->new_attrs) {
		RD_VERIFY_PTR(act, kap->new_attrs, sizeof(*kap->new_attrs));
		RD_PROBE_INT(act, kap->new_attrs, sizeof(*kap->new_attrs)/sizeof(int));
	}

	if(kap->flags != _NTO_CHANCON_ATTR_CONFLAGS) {
		CHANNELGBL *chgblp;

		if(!(kap->id & _NTO_GLOBAL_CHANNEL)) {
			return EINVAL;
		}
			
		if((chgblp = (CHANNELGBL*)vector_lookup2(&chgbl_vector, kap->id & ~_NTO_GLOBAL_CHANNEL)) == NULL  ||
			chgblp->ch.type != TYPE_CHANNEL) {
			return EINVAL;
		}

		switch(kap->flags) {
			case _NTO_CHANCON_ATTR_CURMSGS:
				if(kap->old_attrs != NULL) {
					kap->old_attrs->num_curmsgs = chgblp->num_curr_msg;
				} 
				break;

			case _NTO_CHANCON_ATTR_EVENT:
				if(kap->old_attrs != NULL) {
					kap->old_attrs->ev.event = chgblp->event;
					kap->old_attrs->ev.coid = chgblp->ev_coid;
				}
				if(kap->new_attrs != NULL) {
					if(chgblp->ev_prp) {
						if((SIGEV_GET_TYPE(&kap->new_attrs->ev.event) != SIGEV_NONE)
						  || (chgblp->ev_prp != act->process)
						  /*|| (chgblp->ev_coid != kap->new_attrs->ev.coid)*/) {
							  return EBUSY;
						  }
					} else {
						if(SIGEV_GET_TYPE(&kap->new_attrs->ev.event) == SIGEV_NONE) {
							return EINVAL;
						}
					}
					lock_kernel();
					chgblp->ev_prp = act->process;
					chgblp->event = kap->new_attrs->ev.event;
					chgblp->ev_coid = kap->new_attrs->ev.coid;
					if(SIGEV_GET_TYPE(&chgblp->event) == SIGEV_NONE) {
						chgblp->ev_prp = NULL;
					}
				}
				break;

			case _NTO_CHANCON_ATTR_CHANFLAGS:
				if(kap->old_attrs != NULL) {
					kap->old_attrs->flags = chgblp->ch.flags & _NTO_CHF_ASYNC_NONBLOCK;
				}

				if(kap->new_attrs != NULL) {
					lock_kernel();
					if(kap->new_attrs->flags & _NTO_CHF_ASYNC_NONBLOCK) {
						chgblp->ch.flags |= _NTO_CHF_ASYNC_NONBLOCK;
					} else {
						chgblp->ch.flags &= ~_NTO_CHF_ASYNC_NONBLOCK;
					}
				}
				break;

			default:
				return EINVAL;
		}

	} else { /* only connection attributes */
		CONNECT *cop;
		VECTOR *vec;
		PROCESS *prp;

		prp = act->process;
		vec = (kap->id & _NTO_SIDE_CHANNEL) ? &prp->chancons : &prp->fdcons;
		if((cop = vector_lookup2(vec, kap->id & ~_NTO_SIDE_CHANNEL)) == NULL  ||  cop->type != TYPE_CONNECTION) {
			return EINVAL;
		}

		if(kap->old_attrs != NULL) {
			kap->old_attrs->flags = cop->flags & _NTO_COF_NONBLOCK;
		}

		if(kap->new_attrs != NULL) {
			lock_kernel();
			if(kap->new_attrs->flags & _NTO_COF_NONBLOCK) {
				cop->flags |= _NTO_COF_NONBLOCK;
			} else {
				cop->flags &= ~_NTO_COF_NONBLOCK;
			}
		}
	}

	return EOK;

}


__SRCVERSION("ker_connect.c $Rev: 169918 $");
