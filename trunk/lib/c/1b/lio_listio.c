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




#undef  _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#undef  _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 32
#include <aio.h>
#include <atomic.h>
#include <errno.h>
#include <sys/siginfo.h>
#include <aio_priv.h>
#include <string.h>

int lio_listio64(int mode, struct aiocb64 * __const list[], int nent,
	struct sigevent *sig)
{
	unsigned i, err;
	struct aiocb *aiocbp;
	struct _aio_waiter waiter;
	int policy;
	struct sched_param  param;
	extern void _aio_clean_up(void *mutex);
	
	if (mode != LIO_WAIT && mode != LIO_NOWAIT) {
		errno = EINVAL;
		return -1;
	}

	if (!_aio_cb && _aio_init(NULL)) {
		return -1;
	}

	if ((errno = pthread_cond_init(&waiter.w_cond, 0)) != EOK) {
		return -1;
	}
	waiter.w_count = 0;
	
	/* get param */
	if ((errno = pthread_getschedparam(pthread_self(), &policy, &param)) != EOK) {
		pthread_cond_destroy(&waiter.w_cond);
		return -1;
	}

	err = 0;
	_mutex_lock(&_aio_cb->cb_mutex);
	for (i = 0; i < nent; i++) {
		if ((aiocbp = (struct aiocb *)list[i]) == NULL)
		  continue;
		
		if (aiocbp->aio_lio_opcode == LIO_READ) {
			aiocbp->_aio_iotype = _AIO_OPCODE_READ;
		} else if (aiocbp->aio_lio_opcode == LIO_WRITE) {
			aiocbp->_aio_iotype = _AIO_OPCODE_WRITE;
		} else if (aiocbp->aio_lio_opcode == LIO_NOP) {
			continue;
		} else {
			aiocbp->_aio_error = EINVAL;
			aiocbp->_aio_result = (unsigned)-1;
			err = EIO;
			continue;
		}

		aiocbp->_aio_policy = policy;
		memcpy(&aiocbp->_aio_param, &param, sizeof param);
		if (aiocbp->_aio_param.__sched_priority <= aiocbp->aio_reqprio || aiocbp->aio_reqprio < 0) {
			aiocbp->_aio_result = (unsigned)-1;
			aiocbp->_aio_error = EINVAL;
			err = EIO;
			continue;
		}
		aiocbp->_aio_param.__sched_priority -= aiocbp->aio_reqprio;
		aiocbp->_aio_result = 0;
		aiocbp->_aio_error = EINPROGRESS;
		aiocbp->_aio_flag = _AIO_FLAG_ENQUEUE;
		aiocbp->_aio_next = NULL;

		if (mode == LIO_WAIT) {
			aiocbp->_aio_suspend = &waiter;
			aiocbp->_aio_flag |= _AIO_FLAG_SUSPEND;
			waiter.w_count++;
			aiocbp->aio_sigevent.sigev_notify = SIGEV_NONE;
		} else {
			if (sig) {
				aiocbp->aio_sigevent = *sig;
			} else {
				aiocbp->aio_sigevent.sigev_notify = SIGEV_NONE;
			}
		}

		if (_aio_insert_prio(aiocbp) == -1) {
			atomic_set(&aiocbp->_aio_flag, _AIO_FLAG_DONE);
			aiocbp->_aio_result = (unsigned)-1;
			aiocbp->_aio_error = EAGAIN;
			aiocbp->_aio_next = NULL;
			if (aiocbp->_aio_suspend == &waiter) {
				atomic_clr(&aiocbp->_aio_flag, _AIO_FLAG_SUSPEND);
				aiocbp->_aio_suspend = NULL;
				waiter.w_count--;
			}
			err = EAGAIN;
		}
	}

	
	/* wakeup the working threads since we've put some on queue,
	 * wait for all waiter reference come back.
	 */
	pthread_cond_broadcast(&_aio_cb->cb_cond);
	while (waiter.w_count > 0) {
		pthread_cleanup_push(_aio_clean_up, &_aio_cb->cb_mutex);
		err = pthread_cond_wait(&waiter.w_cond, &_aio_cb->cb_mutex);
		pthread_cleanup_pop(0);
		if (err != EOK) {
			break;
		}
	}

	if (err != 0) {
		for (i = 0; i < nent && waiter.w_count; i++) {
			if ((aiocbp = (struct aiocb *)list[i]) == NULL ||
				aiocbp->_aio_suspend != &waiter)
			  continue;
			
			if (aiocbp->_aio_suspend == &waiter) {
				atomic_clr(&aiocbp->_aio_flag, _AIO_FLAG_SUSPEND);
				aiocbp->_aio_suspend = NULL;
				waiter.w_count--;
			}
		}
	}
	pthread_cond_destroy(&waiter.w_cond);
	_mutex_unlock(&_aio_cb->cb_mutex);

	/*
	 * Check status of completed operations if LIO_WAIT was used
	 */
	if (mode == LIO_WAIT) {
		for (i = 0; i < nent; i++) {
			if ((aiocbp = (struct aiocb *)list[i]) == NULL) {
				continue;
			}
			if (aiocbp->_aio_result == (unsigned)-1) {
				err = EIO;
			}
		}
	}
	errno = err;
	return (err == EOK) ? 0 : -1;
}

int lio_listio(int mode, struct aiocb * __const list[], int nent,
	struct sigevent *sig)
{
	int i;
	
	for (i = 0; i < nent; i++) {
		list[i]->aio_offset_hi = (list[i]->aio_offset < 0) ? -1 : 0;
	}
	return lio_listio64(mode, (struct aiocb64 **)list, nent, sig);
}

__SRCVERSION("lio_listio.c $Rev: 159798 $");
