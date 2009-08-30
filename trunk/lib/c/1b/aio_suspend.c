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




#include <aio.h>
#include <atomic.h>
#include <errno.h>
#include <aio_priv.h>

int aio_suspend(__const struct aiocb * __const list[], int nent,
	__const struct timespec *timeout)
{
	unsigned i, j, flag;
	struct _aio_waiter waiter;
	struct aiocb *aiocbp;
	extern void _aio_clean_up(void *mutex);
	int ret = EOK;
	
	if (!_aio_cb && _aio_init(NULL)) {
		return -1;
	}
	
	if ((errno = pthread_cond_init(&waiter.w_cond, 0)) != EOK) {
		return -1;
	}
	waiter.w_count = 0;

	_mutex_lock(&_aio_cb->cb_mutex);
	for (i = 0; i < nent; i++) {
		if (!(aiocbp = (struct aiocb *)list[i]))
		  continue;

		aiocbp->_aio_suspend = &waiter;
		flag = atomic_set_value(&aiocbp->_aio_flag, _AIO_FLAG_SUSPEND);
		if (!(flag & _AIO_FLAG_QMASK)) {
			/* Haven't seen this one, or it is canceled, ignore */
			continue;
		}
		if (flag & _AIO_FLAG_DONE) {
			atomic_clr(&aiocbp->_aio_flag, _AIO_FLAG_SUSPEND);
			aiocbp->_aio_suspend = NULL;
			break;
		}
		atomic_add(&waiter.w_count, 1);
	}

	if (i == nent && waiter.w_count) {
		/* do the waiting */
		pthread_cleanup_push(_aio_clean_up, &_aio_cb->cb_mutex);
		if (timeout) {
			/* POSIX says we need to use a MONOTONIC clock (since we support it) */
			struct timespec mono_timeout = *timeout;

			mono_timeout.tv_sec += time(0);
			ret = pthread_cond_timedwait(&waiter.w_cond, &_aio_cb->cb_mutex, &mono_timeout);
		} else {
			ret = pthread_cond_wait(&waiter.w_cond, &_aio_cb->cb_mutex);
		}
		pthread_cleanup_pop(0);
		if (ret != EOK) {
			/* POSIX says we need to return EAGAIN if timedout */
			if (ret == ETIMEDOUT)
			  errno = EAGAIN;
			else
			  errno = ret;
		}
	}

	/* cancle any still outstanding waiter references */
	for (j = 0; j < nent && waiter.w_count; j++) {
		if (!(aiocbp = (struct aiocb *)list[j]) || aiocbp->_aio_suspend != &waiter)
		  continue;
		
		if (aiocbp->_aio_suspend != &waiter)
		  continue;

		atomic_clr(&aiocbp->_aio_flag, _AIO_FLAG_SUSPEND);
		aiocbp->_aio_suspend = NULL;
		waiter.w_count--;
	}

	pthread_cond_destroy(&waiter.w_cond);
	_mutex_unlock(&_aio_cb->cb_mutex);
	return (ret == EOK) ? 0 : -1;
}

__SRCVERSION("aio_suspend.c $Rev: 153052 $");
