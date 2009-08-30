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
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <atomic.h>
#include <string.h>
#include "dispatch.h"

#define MSG_MAX_SIZE		sizeof(struct _pulse)
#define GROW_VEC			4

int select_attach(void *dpp, select_attr_t *attr, int fd, unsigned flags,
		int (*func)(select_context_t *ctp, int fd, unsigned flags, void *handle),
		void *handle) {
	_select_control 		*ctrl;
	select_vec_t			*vec, *new_vec;
	unsigned				num_elem, num_used;
	int						rc;

	if(!_DPP(dpp)->select_ctrl) {

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

		//ctrl->nparts_max = attr->nparts_max;
		ctrl->msg_max_size = max(MSG_MAX_SIZE, sizeof(resmgr_iomsgs_t));

		ctrl->context_size = sizeof(select_context_t) + ctrl->msg_max_size;
			
		if(_dispatch_attach(dpp, ctrl, DISPATCH_SELECT) == -1) {
			pthread_mutex_destroy(&ctrl->mutex);
			free(ctrl);
			return -1;
		}

		if((ctrl->coid = message_connect(dpp, MSG_FLAG_SIDE_CHANNEL)) == -1) { 
			pthread_mutex_destroy(&ctrl->mutex);
			free(ctrl);
			return -1;
		}

		// Attach message type as well
		// @@@ attach if sigwait type
		if((ctrl->code = pulse_attach(dpp, 0, SI_NOTIFY, _select_msg_handler, (void *)NULL)) == -1) {
			pthread_mutex_destroy(&ctrl->mutex);
			free(ctrl);
			return -1;
		}

		ctrl->rearm_func = _select_rearm_all;
	}

	ctrl = _DPP(dpp)->select_ctrl;
	// Now attach fd
	pthread_mutex_lock(&ctrl->mutex);

	vec = new_vec = _DPP(dpp)->select_ctrl->select_vec;
	num_elem = _DPP(dpp)->select_ctrl->num_elements;
	num_used = _DPP(dpp)->select_ctrl->num_entries;

	// Attach message type to message vector
	if(!vec || num_elem == num_used) {
		new_vec = realloc(vec, (num_elem + GROW_VEC) * sizeof(*vec));
		if(!new_vec) {
			errno = ENOMEM;
			pthread_mutex_unlock(&ctrl->mutex);
			return -1;
		}
		memset(&new_vec[num_elem], 0, GROW_VEC * sizeof *vec);
		_DPP(dpp)->select_ctrl->num_elements = num_elem = GROW_VEC + num_elem;
		_DPP(dpp)->select_ctrl->select_vec = new_vec;
	}

	vec = _dispatch_vector_find(new_vec, num_elem);
	/* @@@
	 If multiple threads were attaching to the vector all concurrently and 
	 only one of them filled memory then others pre-empted we could end up
	 with a bad situation.  There is also the possibility for two threads 
	 using the same vector as well.  For now error out if we can't find
	 a good vector, but we should use some atomic operations to prevent this.
	*/
	if(!vec) {
		pthread_mutex_unlock(&ctrl->mutex);
		errno = ENOMEM;
		return -1;
	}

	vec->fd = fd;
	vec->flags = _VEC_VALID | _SELECT_ARM_FIRST | (flags & _NOTIFY_COND_MASK) | (ctrl->sernum++ & _SELECT_SN_MASK);
	if (flags & SELECT_FLAG_NOREARM)
		vec->flags |= _SELECT_FLAG_NOREARM;
	if (flags & SELECT_FLAG_SRVEXCEPT) {
		vec->flags |= _SELECT_SRVEXCEPT;
		if(!(ctrl->flags & _SELECT_SRVEXCEPT)) {
			// Attach coiddeath pulse code
			if(pulse_attach(dpp, 0, _PULSE_CODE_COIDDEATH, _select_msg_handler, (void *)NULL) == -1) {
				vec->flags = 0;
				pthread_mutex_unlock(&ctrl->mutex);
				errno = EBUSY;
				return -1;
			}
			ctrl->flags |= _SELECT_SRVEXCEPT;
		}
	}
	vec->handle = handle;
	vec->func = func;
	_DPP(dpp)->select_ctrl->num_entries++;
	
	pthread_mutex_unlock(&ctrl->mutex);

	// Send magic pulse to arm 
	// @@@ SIGWAIT case
	(void)MsgSendPulse(ctrl->coid, -1, ctrl->code, -1);

	return 0;
}


dispatch_context_t *
select_rearm(dispatch_context_t *dctp, int fd)
{
	return _select_rearm_how(dctp, fd);
}

dispatch_context_t *
_select_rearm_all(dispatch_context_t *dctp)
{
	return _select_rearm_how(dctp, SEL_REARM_ALL);
}

dispatch_context_t *_select_rearm_how(dispatch_context_t *dctp, int fd) {

	select_context_t	*ctp = &dctp->select_context;
	_select_control 	*ctrl = _DPP(ctp->dpp)->select_ctrl;
	select_vec_t		*vec;
	int					i;

	pthread_mutex_lock(&ctrl->mutex);
	vec = ctrl->select_vec;


	// Go through list and rearm
	for(i = 0; i < ctrl->num_elements; i++, vec++) {
		if((vec->flags & _VEC_VALID) && !(vec->flags & (_SELECT_ARMED | _SELECT_EVENT))) {
			struct _io_notify_reply 	msgo;
			struct _io_notify 			msgi;

			if(fd == SEL_REARM_ALL) {
				if((vec->flags & (_SELECT_FLAG_NOREARM | _SELECT_ARM_FIRST)) ==
				    _SELECT_FLAG_NOREARM)
					continue;
			}
			else if (vec->fd != fd)
				continue;

			vec->flags &= ~_SELECT_ARM_FIRST;


			vec->flags |= _SELECT_ARMED;
			msgi.type = _IO_NOTIFY;
			msgi.combine_len = sizeof msgi;
			msgi.action = _NOTIFY_ACTION_POLLARM;
			msgi.event.sigev_notify = SIGEV_PULSE;
			// @@ check if block is sigwait or receive
			msgi.event.sigev_code = ctrl->code;
			//msgi.event.sigev_code = SI_NOTIFY;
			msgi.event.sigev_coid = ctrl->coid;
			msgi.event.sigev_priority = -1;
			msgi.flags = ~_VEC_VALID & (vec->flags & _NOTIFY_COND_MASK);
			msgi.event.sigev_value.sival_int = _SELECT_SIGEV(i,(vec->flags & _SELECT_SN_MASK));	

			if(MsgSend(vec->fd, &msgi, sizeof msgi, &msgo, sizeof msgo) == -1) {
				/*
				 As soon as we error (invalid fd, no select handler etc) then
				 we mark this vector as being invalid.  User notification would
				 be nice, but there is no way to give that feedback currently.
				*/
				vec->flags &= ~_VEC_VALID;
				ctrl->num_entries--;
			}
			// Check if we succeded as a poll
			if(msgo.flags) {
				// @@@ do case where we are using sigs; stuff siginfo instead
				struct _pulse		*pulse = (struct _pulse *) ctp->msg;

				vec->flags |= _SELECT_EVENT;
				// Fake up pulse?
				ctp->rcvid = 0;
				memset(pulse, 0, sizeof(*pulse));	//Clear the pulse/msg values out
				pulse->code = ctrl->code;
				pulse->value.sival_int = _SELECT_SIGEV(i, (vec->flags & _SELECT_SN_MASK)) | msgo.flags & _NOTIFY_COND_MASK;

				pthread_mutex_unlock(&ctrl->mutex);
				return (dispatch_context_t *)ctp;
			}
		}
	}

	pthread_mutex_unlock(&ctrl->mutex);
			
	return 0;
}

int _select_msg_handler(message_context_t *ctp, int code, unsigned _flags, void *_handle) {
	int					fd, ret, index;
	unsigned			flags;
	int			(*func)(select_context_t *ctp, int fd, unsigned flags, void *handle);
	void				*handle;

	// Check pulse code

	if((index = _select_query((select_context_t *)ctp, &fd, &flags, &func, &handle, 0)) != -1 && func) {

		ret = func((select_context_t *) ctp, fd, flags, handle);
		// Clear the event bit so it gets rearmed
		(void)_select_query((select_context_t *)ctp, &fd, &flags, &func, &handle, 1);
		return ret;
	}
	return 0;
}


int _select_query(select_context_t *ctp, int *fd, unsigned *flags,
		int (**func)(select_context_t *ctp, int fd, unsigned flags, void *handle),
		void **handle, unsigned clear_event) {
	select_vec_t			*vec;
	struct _pulse			*pulse = (struct _pulse *) ctp->msg;
	unsigned				index = _SELECT_SIGEV_INDEX(pulse->value.sival_int);
	unsigned				sn = _SELECT_SIGEV_SN((unsigned)pulse->value.sival_int);
	pthread_mutex_t			*mutex = &_DPP(ctp->dpp)->select_ctrl->mutex; 
	int						i;

	pthread_mutex_lock(mutex);
	vec = _DPP(ctp->dpp)->select_ctrl->select_vec;

	/*
	 * Check to see if the pulse is an exception condition
 	 */
	if(pulse->code == _PULSE_CODE_COIDDEATH) {
		for(i = 0; i < _DPP(ctp->dpp)->select_ctrl->num_elements; i++, vec++) {
			if((vec->flags & (_VEC_VALID | _SELECT_SRVEXCEPT)) == (_VEC_VALID | _SELECT_SRVEXCEPT)) {
				*fd = vec->fd; 
				*handle = vec->handle;	
				*func = vec->func;
				if(clear_event) { 
					vec->flags &= ~(_SELECT_ARMED | _SELECT_EVENT);
				} else {
					vec->flags |= _SELECT_EVENT;
					vec->flags &= ~(_SELECT_ARMED);
				}
				pthread_mutex_unlock(mutex);
				return i;
			}
		}
	} else if (pulse->code != _DPP(ctp->dpp)->select_ctrl->code) {
		/* This was the arming condition and should be ignored */
		pthread_mutex_unlock(mutex);
		return -1;
	}

	// Pulse value has the index into our vec array, with upper bits indicating event
	vec = _DPP(ctp->dpp)->select_ctrl->select_vec;

	/*
	 Once we find and extract a request, unless we have been asked to clear the
	 request entirely, we clear the ARMED bit but make sure that the EVENT bit
	 is on so that we don't get multiple notifications.  This avoids thread race 
	 conditions with the _select_rearm() function (called by dispatch_block_*()).
	*/
	if((index < _DPP(ctp->dpp)->select_ctrl->num_elements) && 
	   (vec[index].flags & _VEC_VALID) && sn == (_SELECT_SN_MASK & vec[index].flags)) {
		*flags = (vec[index].flags & pulse->value.sival_int) & _NOTIFY_COND_MASK;
		*fd = vec[index].fd; 
		*handle = vec[index].handle;	
		*func = vec[index].func;
		if(clear_event) {
			vec[index].flags &= ~(_SELECT_ARMED | _SELECT_EVENT);
		} else {
			vec[index].flags |= _SELECT_EVENT;
			vec[index].flags &= ~(_SELECT_ARMED);
		}
		pthread_mutex_unlock(mutex);
		return index;
	}

	pthread_mutex_unlock(mutex);
	return -1;
}

int select_query(select_context_t *ctp, int *fd, unsigned *flags,
		int (**func)(select_context_t *ctp, int fd, unsigned flags, void *handle),
		void **handle) {
	return _select_query(ctp, fd, flags, func, handle, 0);
}

int select_detach(void *dpp, int fd) {
	select_vec_t			*vec;
	pthread_mutex_t			*mutex; 
	unsigned				num_elements;
	int						i;
	
	// Check to see if we've ever attached anything...
	if(!_DPP(dpp)->select_ctrl) {
		return -1;
	}

	mutex = &_DPP(dpp)->select_ctrl->mutex; 
	pthread_mutex_lock(mutex);
	vec = _DPP(dpp)->select_ctrl->select_vec;
	num_elements = _DPP(dpp)->select_ctrl->num_elements;

	for(i = 0; i < num_elements; i++, vec++) {
		if((vec->flags & _VEC_VALID) && (vec->fd == fd)) {
			vec->flags = 0;
			_DPP(dpp)->select_ctrl->num_entries--;
			pthread_mutex_unlock(mutex);
			return 0;
		}
	}
		
	pthread_mutex_unlock(mutex);
	return -1;

}
void _select_disarm(dispatch_t *dpp, int fd) {

	// Disarm handle


}


__SRCVERSION("dispatch_select.c $Rev: 159388 $");
