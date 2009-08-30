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



PROCESS * rdecl
lookup_pid(pid_t pid) {
	PROCESS *prp;
	unsigned id;

	if((id = PINDEX(pid)) < process_vector.nentries  &&
	   VECP(prp, &process_vector, id)  &&  prp->pid == pid) {
		return(prp);
	}

	return(NULL);
}


CONNECT * rdecl
lookup_connect(int coid) {
	VECTOR	*vec;
	CONNECT	*cop;

	vec = &actives[KERNCPU]->process->fdcons;
	if(coid & _NTO_SIDE_CHANNEL) {
		coid &= ~_NTO_SIDE_CHANNEL;
		vec = &actives[KERNCPU]->process->chancons;
	}

	/* Do unsigned compare with 'coid' to catch negative values as well */
	if(vec  &&  (unsigned)coid < vec->nentries  &&  VECAND(cop = VEC(vec, coid), 1) == 0) {
		return(VECAND(cop, ~3));
	}
	
	return(NULL);
}


//
// This routine looks up a connection and then selects a thread from one
//
CONNECT * rdecl
lookup_rcvid(KERARGS *kap, int rcvid, THREAD **thpp) {
	CONNECT	*cop;
	THREAD	*thp, *act = actives[KERNCPU];
	int		 tid;

	if((cop = vector_lookup(&act->process->chancons, MCINDEX(rcvid))) == NULL
	|| (cop->type != TYPE_CONNECTION)
	|| (cop->process == NULL)) {
		kererr(act, ESRCH);
		return(NULL);
	}

	tid = MTINDEX(rcvid);
	if(cop->flags & COF_NETCON) {
		if(cop->un.net.cred == NULL) {
			kererr(act, EBADF);
			return(NULL);
		}
		if(act->process == net.prp  &&  (cop->scoid != MCINDEX(rcvid)  ||  cop->channel->process != net.prp)) {
			thp = cop->channel ? vector_lookup(&cop->channel->process->threads, tid) : NULL;
		} else {
			thp = vector_lookup(&vthread_vector, tid);
			if(kap) {
				*thpp = thp;
				return(net_send2(kap, tid, cop, thp));
			}
		}
	} else {
		thp = vector_lookup(&cop->process->threads, tid);
	}

	if(thp == NULL) {
		if(kap && (KTYPE(act) == __KER_MSG_DELIVER_EVENT || KTYPE(act) == __KER_MSG_VERIFY_EVENT)) {
			// If the event is to be delivered to a process and the initial
			// thread if the rcvid doesn't exist, use the valid thread.
			switch(SIGEV_GET_TYPE(kap->msg_deliver_event.event)) {
			case SIGEV_SIGNAL_THREAD:
			case SIGEV_UNBLOCK:
			case SIGEV_INTR:
				break;

			default:
				thp = cop->process->valid_thp;
			}
		}

		if(thp == NULL) {
			kererr(act, ESRCH);
			return(NULL);
		}
	}

	*thpp = thp;
	return(cop);
}


LIMITS * rdecl
lookup_limits(uid_t uid) {
	LIMITS	*lip;

	for(lip = limits_list ; lip ; lip = lip->next) {
		if(lip->uid == uid) {
			break;
		}
	}

	return(lip);
}

__SRCVERSION("nano_lookup.c $Rev: 153052 $");
