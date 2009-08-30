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




#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <unistd.h>
#include <sys/dispatch.h>
#include <stdio.h>
#include <string.h>
#include "dispatch.h"

static int _dispatch_coiddeath_attached = 0;

dispatch_t *dispatch_create_channel(int chid, unsigned flags) {
	dispatch_t		*dpp;

	if(!(dpp = malloc(sizeof *dpp))) {
		return NULL;
	}
	memset(dpp, 0x00, sizeof *dpp);

	dpp->chid = chid;
	dpp->flags = flags;

	return dpp;
}

dispatch_t *dispatch_create(void) {

	return dispatch_create_channel(-1, 0);
}

dispatch_context_t *dispatch_context_alloc(dispatch_t *dpp) {
	dispatch_context_t		*ctp;

	if(_DPP(dpp)->block_type == _DISPATCH_BLOCK_RECEIVE) {

		if(!_DPP(dpp)->message_ctrl) {
			errno = EINVAL;
			return NULL;
		}

		if((ctp = calloc(1, _DPP(dpp)->context_size)) == NULL) {
			errno = ENOMEM;
			return NULL;
		}
		if((ctp->message_context.extra = calloc(1, sizeof(struct _extended_context))) == NULL) {
			free(ctp);
			errno = ENOMEM;
			return NULL;
		}
		ctp->message_context.extra->length = sizeof(struct _extended_context);

		ctp->message_context.dpp = (dispatch_t *) dpp;
		ctp->message_context.msg = (resmgr_iomsgs_t *)((char *)ctp + offsetof(message_context_t, iov) + _DPP(dpp)->nparts_max * sizeof(ctp->message_context.iov[0]));
		ctp->message_context.msg_max_size = _DPP(dpp)->context_size - (offsetof(message_context_t, iov) + _DPP(dpp)->nparts_max * sizeof(ctp->message_context.iov[0]));

		dpp->flags |= _DISPATCH_CONTEXT_ALLOCED;
		return ctp;	

	} else if(_DPP(dpp)->block_type == _DISPATCH_BLOCK_SIGWAIT) {
		// @@@ for now
		return NULL;
	}

	errno = EINVAL;
	return NULL;
}

void dispatch_context_free(dispatch_context_t *ctp) {

	free(ctp->message_context.extra);
	free(ctp);
}

int _dispatch_attach(dispatch_t *dpp, void *ctrl, unsigned attach_type) {
	void			**ctrlptr = NULL;
	unsigned 		new_context_size = 0;
	unsigned 		new_msg_size = 0;

	/*
	 * Check validity of type and make sure it is compatible with other
	 * attached types. We cannot mix signals and receives at this point,
	 * but if we have only select types attached, we can migrate from one
	 * to the other.
	 */

	if(dpp->block_type) {
		switch (attach_type) {
			case DISPATCH_MESSAGE:
				dpp->flags &= ~_DISPATCH_ONLY_RESMGR;
				/* Fall Through */
			case DISPATCH_RESMGR:
				if(dpp->flags & _DISPATCH_ONLY_SELECT) {
					dpp->flags &= ~_DISPATCH_ONLY_SELECT;
					//dpp->type = _DISPATCH_BLOCK_MESSAGE;
					// Disarm all select handles so they get rearmed properly
					//@@@ NYI _select_disarm(dpp);
				} else if(dpp->block_type == _DISPATCH_BLOCK_SIGWAIT) {
					errno = EINVAL;
					return -1;
				}
				dpp->block_type = _DISPATCH_BLOCK_RECEIVE;
				break;

			case DISPATCH_SIGWAIT:
				if(dpp->block_type == _DISPATCH_BLOCK_RECEIVE) {
					errno = EINVAL;
					return -1; 
				}
				dpp->block_type = _DISPATCH_BLOCK_SIGWAIT;
				break;

			case DISPATCH_SELECT:
				break;
			default:
				break;
		}
	} else {
		switch(attach_type) {
			case DISPATCH_SELECT:
				dpp->flags |= _DISPATCH_ONLY_SELECT;
				dpp->block_type = _DISPATCH_BLOCK_RECEIVE;	
				break;

			case DISPATCH_RESMGR:
				dpp->flags |= _DISPATCH_ONLY_RESMGR;
				/* Fall Through */
			case DISPATCH_MESSAGE:
				dpp->block_type = _DISPATCH_BLOCK_RECEIVE;	
				break;

			case DISPATCH_SIGWAIT:
				dpp->block_type = _DISPATCH_BLOCK_SIGWAIT;	
				break;
			default:
				break;
		}
	}

	switch (attach_type) {
		case DISPATCH_SELECT:
			ctrlptr = (void **)&dpp->select_ctrl;
			new_context_size = ((_select_control *) ctrl)->context_size;
			//new_msg_size = ((select_ctrl_t *) ctrl)->msg_max_size;
			break;
				
		case DISPATCH_RESMGR:
			ctrlptr = (void **)&dpp->resmgr_ctrl;
			new_context_size = ((_resmgr_control *) ctrl)->context_size;
			new_msg_size = ((_resmgr_control *) ctrl)->msg_max_size;
			break;

		case DISPATCH_MESSAGE:
			ctrlptr = (void **)&dpp->message_ctrl;
			new_context_size = ((_message_control *) ctrl)->context_size;
			new_msg_size = ((_message_control *) ctrl)->msg_max_size;
			break;

		case DISPATCH_SIGWAIT:
			ctrlptr = (void **)&dpp->sigwait_ctrl;
			new_context_size = ((_sigwait_control *) ctrl)->context_size;
			break;

		default:
			errno = EINVAL;
			return -1;
	}

	// @@@ Could have channel flags for the chid?!
	if(dpp->block_type == _DISPATCH_BLOCK_RECEIVE && dpp->chid == -1) {
		unsigned flags = _NTO_CHF_UNBLOCK | _NTO_CHF_DISCONNECT;

		if(!_dispatch_coiddeath_attached) {
			flags |= _NTO_CHF_COID_DISCONNECT;
		}
		if((dpp->chid = ChannelCreate(flags)) == -1) {
			return -1;
		}
		if(!_dispatch_coiddeath_attached) {
			_dispatch_coiddeath_attached = 1;
			dpp->flags |= _DISPATCH_CHANNEL_COIDDEATH;
		}
	}

	*ctrlptr = ctrl;
	dpp->context_size = max(dpp->context_size, new_context_size); 
	dpp->msg_max_size = max(dpp->msg_max_size, new_msg_size);
	dpp->nparts_max = max(dpp->nparts_max, ((_resmgr_control *) ctrl)->nparts_max);
	return 0;
}

static dispatch_context_t *dispatch_block_receive_all(dispatch_context_t *ctp, 
                                                      int (*block_func)(int chid, void *msg, 
													                    int bytes, struct _msg_info *info)) {
	dispatch_t				*dpp = ((message_context_t *) ctp)->dpp;

	if(dpp->flags & _DISPATCH_TIMEOUT) {
		if(timer_timeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_RECEIVE, 0, &dpp->timeout, 0) == -1) {
				return NULL;
		}
	}

	// Set up the select blocking, and rearm any fd's
	if((dpp->select_ctrl) &&  dpp->select_ctrl->rearm_func && dpp->select_ctrl->rearm_func(ctp)) {
	
		return ctp;
	}

	ctp->message_context.id = -1;
	ctp->message_context.info.msglen = 0;

again:
	if((ctp->message_context.rcvid = block_func(dpp->chid, ctp->message_context.msg, ctp->message_context.msg_max_size, &ctp->message_context.info)) == -1 && errno != ETIMEDOUT) {
		return NULL;
	}
	// Doing a network transaction and not all the message was send, so get the rest...
	if(ctp->message_context.rcvid > 0 && ctp->message_context.info.srcmsglen > ctp->message_context.info.msglen && ctp->message_context.info.msglen < ctp->message_context.msg_max_size) {
		int						n;

		if((n = MsgRead_r(ctp->message_context.rcvid, (char *)ctp->message_context.msg + ctp->message_context.info.msglen,
				ctp->message_context.msg_max_size - ctp->message_context.info.msglen, ctp->message_context.info.msglen)) < 0) {
			MsgError(ctp->message_context.rcvid, -n);
			goto again;
		}
		ctp->message_context.info.msglen += n;
	}

	return ctp;
}

dispatch_context_t *dispatch_block_receive_pulse(dispatch_context_t *ctp) {
	return dispatch_block_receive_all(ctp, MsgReceivePulse);
}

dispatch_context_t *dispatch_block_receive(dispatch_context_t *ctp) {
	return dispatch_block_receive_all(ctp, MsgReceive);
}

dispatch_context_t *dispatch_block_sigwait(dispatch_context_t *ctp) {
	dispatch_t				*dpp = ((sigwait_context_t *) ctp)->dpp;

	if(dpp->flags & _DISPATCH_TIMEOUT) {
		if(timer_timeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_SIGWAITINFO, 0, &dpp->timeout, 0) == -1) {
				return NULL;
		}
	}
	// @@@ Get set from ctrl;
	if((ctp->sigwait_context.signo = SignalWaitinfo(&ctp->sigwait_context.set, &ctp->sigwait_context.info.siginfo)) == -1 && errno != ETIMEDOUT) {
		return NULL;
	}
	// Need to stuff vectors with appropriate info
	return ctp;	
}

dispatch_context_t *dispatch_block(dispatch_context_t *ctp) {
	dispatch_t				*dpp = ((message_context_t *) ctp)->dpp;

	if(dpp->block_type == _DISPATCH_BLOCK_RECEIVE) {
		return dispatch_block_receive(ctp);
	} else if(dpp->block_type == _DISPATCH_BLOCK_SIGWAIT) {
		return dispatch_block_sigwait(ctp);
	} else if(dpp->block_type == _DISPATCH_BLOCK_TIMEOUT) {
		if(timer_timeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_NANOSLEEP, 0, &dpp->timeout, 0) == -1) {
			return NULL;
		}
		return ctp;
	}
	errno = EINVAL;
	return NULL;
}

void dispatch_unblock(dispatch_context_t *ctp) {
	dispatch_t				*dpp = ((message_context_t *) ctp)->dpp;

	/* We only really handle the receive case right now */
	if(dpp->block_type == _DISPATCH_BLOCK_RECEIVE) {
		_message_unblock(ctp);
	} else if(dpp->block_type == _DISPATCH_BLOCK_SIGWAIT) {
		return;
	} else if(dpp->block_type == _DISPATCH_BLOCK_TIMEOUT) {
		return;
	}
}

int dispatch_handler(dispatch_context_t *ctp) {
	dispatch_t				*dpp;

	/*
	 * We special-case a few of the more common cases, for speed
	 */	
#if 0
	if(dpp->flags & _DISPATCH_ONLY_RESMGR) {
		return resmgr_handler((resmgr_context_t *)ctp);
	} else if (dpp->flags & _DISPATCH_ONLY_SELECT) {
		return select_handler((select_context_t *)ctp);
	}
#endif
	/*	ctp == NULL can occur if the blocking function came out early
		due to a signal or other error condition and we should return -1.

		In the case of a thread pool, a -1 return from the handler function
		indicates that a reblock should occur (what else can we do?).
	*/	
	if (!ctp) {
		return -1;
	}
	
	dpp = ((message_context_t *)ctp)->dpp;
	switch(dpp->block_type) {
		case _DISPATCH_BLOCK_RECEIVE:
			return _message_handler(ctp);
		case _DISPATCH_BLOCK_SIGWAIT:
			return _sigwait_handler(ctp);
		default:
			break;
	}

	return -1;
}

int dispatch_timeout(dispatch_t *dpp, struct timespec *timeout) {
	if(timeout) {
//		dpp->timeout_type = _NTO_TIMEOUT_NANOSLEEP;
//			_NTO_TIMEOUT_SIGWAITINFO
//			_NTO_TIMEOUT_RECEIVE
 		_DPP(dpp)->flags |= _DISPATCH_TIMEOUT;
		_DPP(dpp)->timeout = *timeout;
	}
	return 0;
}

int dispatch_destroy(dispatch_t *dpp) {

	if(_DPP(dpp)->chid != -1) {
		(void)ChannelDestroy(_DPP(dpp)->chid);
	}
	
	if(dpp->flags & _DISPATCH_CHANNEL_COIDDEATH) {
		_dispatch_coiddeath_attached = 0;
	}

	/**
	 We have to go through each of the control structures here and make
	 sure that we release the resources that each one acquired.  This is
	 different from what happens in a xxxx_detach() since at that point 
	 we are only cleaning up a single service, not the whole of dispatch.
    **/
	if(dpp->resmgr_ctrl) {
		pthread_mutex_destroy(&dpp->resmgr_ctrl->mutex);
		free(dpp->resmgr_ctrl);
	}

	if(dpp->message_ctrl) {
		//@@@ No need for locking since someone must sync this dispatch operation.
		//pthread_mutex_lock(&dpp->message_ctrl->mutex);
		if(dpp->message_ctrl->message_vec) {
			free(dpp->message_ctrl->message_vec);
		}
		//pthread_mutex_unlock(&dpp->message_ctrl->mutex);
		pthread_mutex_destroy(&dpp->message_ctrl->mutex);
		free(dpp->message_ctrl);
	}

	if(dpp->select_ctrl) {
		//@@@ Same locking argument as for message_ctrl
		if(dpp->select_ctrl->select_vec) {
			free(dpp->select_ctrl->select_vec);
		}
		pthread_mutex_destroy(&dpp->select_ctrl->mutex);
		free(dpp->select_ctrl);
	}

	if(dpp->sigwait_ctrl) {
		//@@@ Same locking argument as for message_ctrl
		if(dpp->sigwait_ctrl->sigwait_vec) {
			free(dpp->sigwait_ctrl->sigwait_vec);
		}
		pthread_mutex_destroy(&dpp->sigwait_ctrl->mutex);
		free(dpp->sigwait_ctrl);
	}

	//Just be sure we don't do this again.
	memset(dpp, 0, sizeof(dpp));

	free(dpp);
	return 0;
}

int _dispatch_set_contextsize(dispatch_t *dpp, unsigned attach_type) {
	unsigned new_size = 0, new_msg_size = 0;

	switch (attach_type) {
		case DISPATCH_SELECT:
			new_size = _DPP(dpp)->select_ctrl->context_size;
			//new_msg_size = ((select_ctrl_t *) ctrl)->msg_max_size;
			break;
				
		case DISPATCH_RESMGR:
			new_size = _DPP(dpp)->resmgr_ctrl->context_size;
			new_msg_size = _DPP(dpp)->resmgr_ctrl->msg_max_size;
			break;

		case DISPATCH_MESSAGE:
			new_size = _DPP(dpp)->message_ctrl->context_size;
			new_msg_size = _DPP(dpp)->message_ctrl->msg_max_size;
			break;

		case DISPATCH_SIGWAIT:
			new_size = _DPP(dpp)->sigwait_ctrl->context_size;
			break;
		default:
			return -1;
	}

	if((dpp->flags & _DISPATCH_CONTEXT_ALLOCED) && (new_size > dpp->context_size || new_msg_size > dpp->msg_max_size)) {
		return -1;
	} else {
		// @@@ Could have channel flags for the chid?!
		_DPP(dpp)->context_size = max(_DPP(dpp)->context_size, new_size); 
		_DPP(dpp)->msg_max_size = max(_DPP(dpp)->msg_max_size, new_msg_size);
	}

	return 0;
}

__SRCVERSION("dispatch.c $Rev: 171666 $");
