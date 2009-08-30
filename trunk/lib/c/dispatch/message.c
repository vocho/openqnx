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




#include <sys/dispatch.h>
#include <sys/resmgr.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <process.h>
#include <string.h>
#include <gulliver.h>
#include "dispatch.h"

#define GROW_VEC	4
#define MSG_MAX_SIZE 	sizeof(struct _pulse)


int message_attach(dispatch_t *dpp, message_attr_t *attr, int low, int high,
		int (*func)(message_context_t *ctp, int fd, unsigned flags, void *handle),
		void *handle) {
	message_vec_t		*vec, *new_vec;
	int					num_elem, num_used, rc = 0;
	_message_control	*ctrl;

	if(!_DPP(dpp)->message_ctrl) {

		if((ctrl = malloc(sizeof *ctrl)) == NULL) {
			errno = ENOMEM;
			return -1;
		}
		memset(ctrl, 0, sizeof *ctrl);

		if((rc = pthread_mutex_init(&ctrl->mutex, 0)) != EOK) {
			free(ctrl);
			errno = rc;
			return -1;
		}

		ctrl->nparts_max = max(attr ? attr->nparts_max : 0, 1);
		ctrl->msg_max_size = max(attr ? attr->msg_max_size : 0, max(MSG_MAX_SIZE, sizeof(resmgr_iomsgs_t)));

		ctrl->context_size = sizeof(message_context_t) + ctrl->nparts_max * sizeof(((resmgr_context_t *)0)->iov[0]) + ctrl->msg_max_size;

		if(_dispatch_attach(dpp, ctrl, DISPATCH_MESSAGE) == -1) {
			pthread_mutex_destroy(&ctrl->mutex);
			free(ctrl);
			return -1;
		}
	}

	ctrl = _DPP(dpp)->message_ctrl;
	pthread_mutex_lock(&ctrl->mutex);

	vec = new_vec = _DPP(dpp)->message_ctrl->message_vec;
	num_elem = _DPP(dpp)->message_ctrl->num_elements;
	num_used = _DPP(dpp)->message_ctrl->num_entries;

	// Attach message type to message vector
	if(!vec || num_elem == num_used) {
		new_vec = realloc(vec, (num_elem + GROW_VEC) * sizeof(*vec));
		if(!new_vec) {
			errno = ENOMEM;
			pthread_mutex_unlock(&ctrl->mutex);
			return -1;
		}
		memset(&new_vec[num_elem], 0, GROW_VEC * sizeof(*vec));
		_DPP(dpp)->message_ctrl->num_elements = num_elem = GROW_VEC + num_elem;
		_DPP(dpp)->message_ctrl->message_vec = new_vec;
	}

	vec = _dispatch_vector_find(new_vec, num_elem);
	if(!vec) {
		// @@@
		//fprintf(stderr, "error\n");
		errno = ENOMEM;
		pthread_mutex_unlock(&ctrl->mutex);
		return -1;
	}

	if (attr && (attr->flags & MSG_FLAG_TYPE_PULSE) && (attr->flags & MSG_FLAG_ALLOC_PULSE)) {
		// We need to allocate a pulse code
		message_vec_t		*vec2;
		int					i;
		low = _PULSE_CODE_MINAVAIL;

		while(low <= _PULSE_CODE_MAXAVAIL) {

			high = low;
			vec2 = _DPP(dpp)->message_ctrl->message_vec;

			for(i = 0; i < _DPP(dpp)->message_ctrl->num_elements; vec2++, i++) {
				if((vec2->flags & _VEC_VALID) && (vec2->flags & _MESSAGE_PULSE_ENTRY) && (low >= vec2->lo && low <= vec2->high)) {
					low ++;
					break;
				}
			}
			if(low == high) break;			
		}

		if(low > _PULSE_CODE_MAXAVAIL) {
			// Could not allocate a pulse
			pthread_mutex_unlock(&ctrl->mutex);
			errno = EAGAIN;
			return -1;
		}
		rc = low;

	} else if (attr && (attr->flags & MSG_FLAG_TYPE_PULSE) && (low < SCHAR_MIN) && (high > SCHAR_MAX)) {
		pthread_mutex_unlock(&ctrl->mutex);
		errno = EINVAL;
		return -1;
	} else if (attr && (attr->flags & MSG_FLAG_TYPE_PULSE) && (low >= _PULSE_CODE_COIDDEATH) && (high <= _PULSE_CODE_COIDDEATH) && !(dpp->flags & _DISPATCH_CHANNEL_COIDDEATH)) {
		pthread_mutex_unlock(&ctrl->mutex);
		errno = EBUSY;
		return -1;
	}

	vec->lo = low;
	vec->high = high;
	vec->flags = _VEC_VALID;
	vec->handle = handle;
	vec->func = func;
	_DPP(dpp)->message_ctrl->num_entries++;
	
	if(attr) {		
		if(attr->flags & MSG_FLAG_TYPE_RESMGR) {
			vec->flags |= _MESSAGE_RESMGR_ENTRY;
		} else if (attr->flags & MSG_FLAG_TYPE_SELECT) {
			vec->flags |= (_MESSAGE_SELECT_ENTRY | _MESSAGE_PULSE_ENTRY);
		}

		if (attr->flags & MSG_FLAG_TYPE_PULSE) {
			vec->flags |= _MESSAGE_PULSE_ENTRY;
			rc = low;
		}

		if(attr->flags & MSG_FLAG_DEFAULT_FUNC) {
			vec->flags |= _MESSAGE_DEFAULT_ENTRY;
		}

		if(attr->flags & MSG_FLAG_CROSS_ENDIAN) {
			vec->flags |= _MESSAGE_CROSS_ENDIAN;
		}
	}

	pthread_mutex_unlock(&ctrl->mutex);

	return rc;
}

int pulse_attach(dispatch_t *dpp, int flags, int code,
		int (*func)(message_context_t *ctp, int fd, unsigned flags, void *handle),
		void *handle) {
	message_attr_t		attr = {
		0,
		0,
		0
	};

	if(flags & MSG_FLAG_ALLOC_PULSE) {
		attr.flags = (MSG_FLAG_TYPE_PULSE | MSG_FLAG_ALLOC_PULSE);
	} else {
		attr.flags = MSG_FLAG_TYPE_PULSE;
	}

	return message_attach(dpp, &attr, code, code, func, handle);
}

int message_detach(dispatch_t *dpp, int low, int high, int flags) {
	_message_control	*ctrl = _DPP(dpp)->message_ctrl;

	if(ctrl) {
		message_vec_t		*vec;
		int					i;

		pthread_mutex_lock(&ctrl->mutex);

		vec = ctrl->message_vec;

		// Search for a matching entry
		for(i = 0; i < _DPP(dpp)->message_ctrl->num_elements; vec++, i++) {

			if(vec->flags & _VEC_VALID) {
				if(((low == vec->lo && high == vec->high) && 
						!(flags ^ (vec->flags & MSG_FLAG_TYPE_PULSE)))
					|| (flags & (vec->flags & _MESSAGE_DEFAULT_ENTRY))) {
					vec->flags &= ~_VEC_VALID;
					_DPP(dpp)->message_ctrl->num_entries--;
					pthread_mutex_unlock(&ctrl->mutex);
					return 0;
				}
					
			}
		}
		pthread_mutex_unlock(&ctrl->mutex);
	}
	return -1;
}

int pulse_detach(dispatch_t *dpp, int code, int flags) {

	return message_detach(dpp, code, code, MSG_FLAG_TYPE_PULSE);
}

int message_connect(dispatch_t *dpp, int flags) {

	return ConnectAttach(0, 0, _DPP(dpp)->chid, (flags & MSG_FLAG_SIDE_CHANNEL) ? _NTO_SIDE_CHANNEL : 0, 0);

}


message_context_t *message_block(message_context_t *ctp) {

	ctp->id = -1;
	ctp->info.msglen = _DPP(ctp->dpp)->msg_max_size;
	ctp = (message_context_t *) dispatch_block((dispatch_context_t *)ctp);
	return ctp;
}

void _message_unblock(dispatch_context_t *ctp) {
	dispatch_t				*dpp = ((message_context_t *) ctp)->dpp;
	struct sched_param		param;
	int						coid;


	if ((coid = ConnectAttach(0, 0, dpp->chid, _NTO_SIDE_CHANNEL, 0)) == -1) {
		return;		//We should indicate an error, but we can't
	}

	memset(&param, 0, sizeof(param));
	if (SchedGet(0, 0, &param) == -1) {
		param.sched_priority = 1;
	}

	/* An unblock pulse with rcvid == 0 should be treated as a noop */
	//MsgSendPulse(coid, param.sched_priority, _PULSE_CODE_UNBLOCK, getpid());
	(void)MsgSendPulse(coid, param.sched_priority, _PULSE_CODE_UNBLOCK, 0);

	ConnectDetach(coid);
}

//
// Convenience macros. We can avoid locking the list of registered message
// types if the NOLOCK flag was specified to dispatch_create_channel(). This
// is used by Proc, where we know the message vector will never change.
//

#define LOCK(dpp,mutex)		\
	if(!(_DPP(dpp)->flags & DISPATCH_FLAG_NOLOCK)) pthread_mutex_lock(mutex)
#define UNLOCK(dpp,mutex)		\
	if(!(_DPP(dpp)->flags & DISPATCH_FLAG_NOLOCK)) pthread_mutex_unlock(mutex)

int _message_handler(dispatch_context_t *dctp) {
	message_context_t	*ctp = &dctp->message_context;
	int					i;
	message_vec_t		*vec, *def_vec;
	unsigned short		code;
	pthread_mutex_t		*mutex = &_DPP(ctp->dpp)->message_ctrl->mutex;

	if(ctp->rcvid == -1) return -1;

	/*TODO: Check for pulses with not enough data for a pulse */

/* Great debate ... to allow or disallow one byte messages ... */
#if 0
	if (ctp->rcvid) {
		switch(ctp->info.msglen) {
		case 0:
			MsgError(ctp->rcvid, EINVAL);
			return -1;
		case 1:
			code = (unsigned short)(*((char *)ctp->msg));
			break;
		default:
			code = ctp->msg->type;
			break;
		}
	}
#else
	if (ctp->rcvid && ctp->info.msglen < 2) {
		MsgError(ctp->rcvid, EINVAL);
		return -1;
	}
	code = ctp->msg->type;
#endif

	if((ctp->info.flags & _NTO_MI_ENDIAN_DIFF) && ctp->rcvid) {
		code = ENDIAN_RET16(code);
	}

	def_vec = NULL;

	// Loop through the vectors, and call a registered function
	LOCK(_DPP(ctp->dpp), mutex);

	vec = _DPP(ctp->dpp)->message_ctrl->message_vec;

	for(i = 0; i < _DPP(ctp->dpp)->message_ctrl->num_elements; vec++, i++) {
		if(vec->flags & _VEC_VALID) {

			/* Check if we have a match 
			 Pulse codes are treated as int8 entities, while the message codes
			 are treated as uint16 entities.  As a result we leave the range
			 defined as a int16 (in dispatch.h) but cast it to a uint16 when 
			 we check the message range.
			*/
			if(ctp->rcvid == 0 && (vec->flags & _MESSAGE_PULSE_ENTRY) && code == _PULSE_TYPE && ctp->msg->pulse.subtype == _PULSE_SUBTYPE) {
				if(ctp->msg->pulse.code >= vec->lo && ctp->msg->pulse.code <= vec->high) {
					UNLOCK(_DPP(ctp->dpp), mutex);
					return vec->func(ctp, ctp->msg->pulse.code, 0, vec->handle);
				}
			} else if(ctp->rcvid && !(vec->flags & (_MESSAGE_DEFAULT_ENTRY | _MESSAGE_PULSE_ENTRY)) && code >= (uint16_t)vec->lo && code <= (uint16_t)vec->high) {
				UNLOCK(_DPP(ctp->dpp), mutex);
				if((ctp->info.flags & _NTO_MI_ENDIAN_DIFF) && !(vec->flags & _MESSAGE_CROSS_ENDIAN)) {
					MsgError(ctp->rcvid, EENDIAN);
					return -1;
				}
				return vec->func(ctp, code, 0, vec->handle);
			} else if (vec->flags & _MESSAGE_DEFAULT_ENTRY) {
				def_vec = vec;
			}			
		}
	}		

	UNLOCK(_DPP(ctp->dpp), mutex);

	// Nothing matched, and we have a default message handler
	if(def_vec) {
		return def_vec->func(ctp, code, 0, def_vec->handle);
	} else if(ctp->rcvid) {
		MsgError(ctp->rcvid, ENOSYS);
	}

	return -1;
}

__SRCVERSION("message.c $Rev: 171666 $");
