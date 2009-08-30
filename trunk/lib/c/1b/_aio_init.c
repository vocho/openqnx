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




#undef  _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#include <aio.h>
#include <atomic.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <sys/iomsg.h>
#include <aio_priv.h>

struct _aio_control_block *_aio_cb = NULL;

struct _aio_context {
	struct _aio_context *next;
	struct aiocb        *curr_list;
	int                 policy;
	struct sched_param  param;
	pthread_t           tid;
};

static int _aio_wakeup(struct aiocb *aiocbp) {
	struct sigevent *ev = &aiocbp->aio_sigevent;
	int flag, ret = EOK;
	
	/* let's see if there is an suspend waiting us */
	flag = atomic_set_value(&aiocbp->_aio_flag, _AIO_FLAG_DONE);
	if (flag & _AIO_FLAG_SUSPEND) {
		struct _aio_waiter *w;
		
		_mutex_lock(&_aio_cb->cb_mutex);
		if ((w = aiocbp->_aio_suspend) != NULL) {
			atomic_clr(&aiocbp->_aio_flag, _AIO_FLAG_SUSPEND);
			if ((ret = pthread_cond_signal(&w->w_cond)) != EOK) {
				aiocbp->_aio_result = (unsigned)-1;
				aiocbp->_aio_error = ret;
			}
			w->w_count--;
			aiocbp->_aio_suspend = NULL;
		}
		_mutex_unlock(&_aio_cb->cb_mutex);
		/* shall it be a wakeup even if there is an suspend ? */
		return ret;
	}
		
	switch (SIGEV_GET_TYPE(ev)) {
	  case SIGEV_SIGNAL:
	  case SIGEV_SIGNAL_CODE:
	  case SIGEV_SIGNAL_THREAD:
		ret = SignalKill_r(0, getpid(), 0, ev->sigev_signo, SI_ASYNCIO, 
						   ev->sigev_value.sival_int);
		break;
	  case SIGEV_PULSE:
		ret = MsgSendPulse_r(ev->sigev_coid, ev->sigev_priority, 
							 ev->sigev_code, ev->sigev_value.sival_int);
		break;
	  case SIGEV_THREAD:
		ret = pthread_create(NULL, ev->sigev_notify_attributes,
							 (void *)ev->sigev_notify_function, 
							 ev->sigev_value.sival_ptr);
		break;
	  case SIGEV_NONE:
		ret = EOK;
		break;
	  case SIGEV_UNBLOCK:
	  case SIGEV_INTR:
	  default:
		/* don't know how to deliever these 2 */
		ret = EOPNOTSUPP;
		break;
	}
	
	if (ret != EOK) {
		/* what if the io operation itself failed ? */
		aiocbp->_aio_result = (unsigned)-1;
		aiocbp->_aio_error = ret;
	}
	
	return ret;
}

void _aio_clean_up(void *mutex) {
	_mutex_unlock((pthread_mutex_t *)mutex);
}

static struct _aio_context *_aio_block(struct _aio_context *ctp)
{
	struct _aio_control_block *cbp = _aio_cb;
	struct _aio_context *cp;
	struct _aio_prio_list *plist;
	struct aiocb *curr, *ap, **app;
	int ret;

	if (!cbp->tp) {
		errno = EINVAL;
		return NULL;
	}

	_mutex_lock(&cbp->cb_mutex);
	do {
		while ((plist = cbp->cb_plist) == NULL) {
			pthread_cleanup_push(_aio_clean_up, &cbp->cb_mutex);
			ret = pthread_cond_wait(&cbp->cb_cond, &cbp->cb_mutex);
			pthread_cleanup_pop(0);
			if (ret != EOK) {
				_mutex_unlock(&cbp->cb_mutex);
				errno = ret;
				return NULL;
			}
		}
		
		/* 
		 * If there is an empty priority list entry, remove it;
		 */
		if ((curr = plist->head) == NULL) {
			cbp->cb_plist = plist->next;
			if (cbp->cb_nfree == _AIO_PRIO_LIST_LOW) {
				free(plist);
			} else {
				plist->next = cbp->cb_plist_free;
				cbp->cb_plist_free = plist;
				cbp->cb_nfree++;
			}
			continue;
		}
		
		/* 
		 * Put request on to process queue
		 */
		if ((plist->head = curr->_aio_next) == NULL)
			plist->tail = &plist->head;
		curr->_aio_next = NULL;
		curr->_aio_plist = NULL;
		atomic_set(&curr->_aio_flag, _AIO_FLAG_IN_PROGRESS);
		
		/* check if another thread already handling this fd */
		for (cp = _aio_cb->ct_list; cp; cp = cp->next) {
			if (!cp->curr_list)
			  continue;
			if (cp->curr_list->aio_fildes == curr->aio_fildes)
			  break;
		}

		if (!cp) {
			/* if nobody handling this fd, we will do it */
			ctp->curr_list = curr;
		} else {
			/* another thread is handling this fd, insert at right priority */
			for (app = &cp->curr_list, ap = *app; ap; app = &ap->_aio_next, ap = *app) {
				if (ap->_aio_param.__sched_priority > curr->_aio_param.__sched_priority)
				  break;
			}
			curr->_aio_next = *app;
			*app = curr;
		}
	} while (!ctp->curr_list);
	_mutex_unlock(&cbp->cb_mutex);
	return ctp;
}

static void _aio_unblock(struct _aio_context *ctp)
{
	struct _aio_context *cp;
	
	_mutex_lock(&_aio_cb->cb_mutex);
	for (cp = _aio_cb->ct_list; cp; cp = cp->next)
	  if (cp->tid != ctp->tid)
		(void)pthread_cancel(cp->tid);
	_mutex_unlock(&_aio_cb->cb_mutex);
	return;
}

static int _aio_handler(struct _aio_context *ctp)
{
	struct aiocb *curr;
	io_lseek_t	 lmsg;
	union {
		io_read_t   rmsg;
		io_write_t  wmsg;
	} msg;
	iov_t iov[3];
	int niov;

	if (!ctp)
	  return -1;
	
	while ((curr = ctp->curr_list) != NULL) 
	{
		/* see if we need to change sched_param */
		if (ctp->policy != curr->_aio_policy || memcmp(&ctp->param, &curr->_aio_param, sizeof(ctp->param)))
		{
			curr->_aio_error = pthread_setschedparam(ctp->tid, curr->_aio_policy, (struct sched_param *)&curr->_aio_param);
			if (curr->_aio_error != EOK) {
				curr->_aio_result = (unsigned)-1;
				_aio_wakeup(curr);
				continue;
			}
			ctp->policy = curr->_aio_policy;
			memcpy(&ctp->param, &curr->_aio_param, sizeof ctp->param);
		}
		niov = 0;
		switch (curr->_aio_iotype) {
		  case _AIO_OPCODE_READ:
		  case _AIO_OPCODE_WRITE:
			/* creat a lseek + read/write combined message */
			lmsg.i.type = _IO_LSEEK;
			lmsg.i.combine_len = sizeof(lmsg) | _IO_COMBINE_FLAG;
			lmsg.i.offset = curr->aio_offset;
			lmsg.i.whence = SEEK_SET;
			lmsg.i.zero = 0;
			SETIOV(iov + niov, &lmsg, sizeof(lmsg));
			niov++;
			
			if (curr->_aio_iotype == _AIO_OPCODE_READ) {
				msg.rmsg.i.type = _IO_READ;
				msg.rmsg.i.combine_len = sizeof(msg.rmsg);
				msg.rmsg.i.nbytes = curr->aio_nbytes;
				msg.rmsg.i.xtype = _IO_XTYPE_NONE;
				msg.rmsg.i.zero = 0;
				SETIOV(iov + niov, &msg, sizeof(msg.rmsg));
				niov++;
				curr->_aio_result = MsgSendvs(curr->aio_fildes, iov, niov,
											  (void *)curr->aio_buf, curr->aio_nbytes);
			} else {
				msg.wmsg.i.type = _IO_WRITE;
				msg.wmsg.i.combine_len = sizeof(msg.wmsg);
				msg.wmsg.i.xtype = _IO_XTYPE_NONE;
				msg.wmsg.i.nbytes = curr->aio_nbytes;
				msg.wmsg.i.zero = 0;
				SETIOV(iov + niov, &msg, sizeof(msg.wmsg));
				SETIOV(iov + niov + 1, curr->aio_buf, curr->aio_nbytes);
				niov += 2;
				curr->_aio_result = MsgSendv(curr->aio_fildes, iov, niov, 0, 0);
			}
			break;
		  case _AIO_OPCODE_SYNC:
			curr->_aio_result = fsync(curr->aio_fildes);
			break;
		  case _AIO_OPCODE_DSYNC:
			curr->_aio_result = fdatasync(curr->aio_fildes);
			break;
		  default:
			break;
		}
		
		_mutex_lock(&_aio_cb->cb_mutex);
		ctp->curr_list = curr->_aio_next;
		curr->_aio_next = NULL;
		_mutex_unlock(&_aio_cb->cb_mutex);
		
		if (curr->_aio_result == -1U) {
			curr->_aio_error = errno;
		} else {
			curr->_aio_error = EOK;
		}

		_aio_wakeup(curr);
	}
	return 0;
}

static struct _aio_context *_aio_context_alloc(struct _aio_control_block *handle)
{
	struct _aio_context *ctp;
	struct _aio_control_block *cbp = _aio_cb;
	int                 policy;
	struct sched_param  param;
	pthread_t           tid;
	
	tid = pthread_self();

	if ((errno = pthread_getschedparam(tid, &policy, &param)) != EOK) {
		return NULL;
	}
	
	_mutex_lock(&cbp->cb_mutex);
	if ((ctp = cbp->ct_free) == NULL && ((ctp = malloc(sizeof(*ctp))) == NULL)) {
		_mutex_unlock(&cbp->cb_mutex);
		return NULL;
	}
	cbp->ct_free = ctp->next;
	ctp->next = cbp->ct_list;
	cbp->ct_list = ctp;
	_mutex_unlock(&cbp->cb_mutex);
	
	ctp->curr_list = NULL;
	ctp->tid = tid;
	ctp->policy = policy;
	ctp->param = param;

	return ctp;
}

static void _aio_context_free(struct _aio_context *ctp)
{
	struct _aio_control_block *cbp = _aio_cb;
	struct _aio_context **ctpp, *cp;
	struct aiocb *curr, *curr_list;

	_mutex_lock(&cbp->cb_mutex);
	for (ctpp = &cbp->ct_list, cp = *ctpp; cp && cp != ctp; ctpp = &cp->next, cp = *ctpp) {
		/* nothing to do */
	}
	if (!cp) {
		_mutex_unlock(&cbp->cb_mutex);
		return;
	}
	*ctpp = cp->next;
	curr_list = cp->curr_list;
	cp->next = cbp->ct_free;
	cbp->ct_free = cp;
	_mutex_unlock(&cbp->cb_mutex);
	
	/* in case there are still aiocbp on us */
	while ((curr = curr_list)) {
		curr_list = curr->_aio_next;
		curr->_aio_result = (unsigned)-1;
		curr->_aio_error = ECANCELED;
		_aio_wakeup(curr);
	}
	return;
}

static pthread_mutex_t _aio_init_mutex = PTHREAD_MUTEX_INITIALIZER;
int _aio_init(thread_pool_attr_t *pool_attr)
{
	static thread_pool_attr_t default_pool_attr = {
		  NULL,
		  _aio_block,
		  _aio_unblock,
		  _aio_handler,
		  _aio_context_alloc,
		  _aio_context_free,
		  NULL,
		  3,
		  1,
		  8,
		  10
	};
	struct _aio_control_block *cb;
	struct _aio_context *ctp;
	struct _aio_prio_list *plist;
	sigset_t set, oset;
	int i;
	  
	_mutex_lock(&_aio_init_mutex);
	if (_aio_cb) {
		_mutex_unlock(&_aio_init_mutex);
		return 0;
	}
	
	if ((cb = malloc(sizeof(*_aio_cb))) == NULL) {
		_mutex_unlock(&_aio_init_mutex);
		return -1;
	}
	memset(cb, 0, sizeof(*cb));
	pthread_mutex_init(&cb->cb_mutex, 0);
	(void)pthread_cond_init(&cb->cb_cond, 0);
	
	if (pool_attr == NULL) {
		pool_attr = &default_pool_attr;
	} else {
		pool_attr->block_func = _aio_block;
		pool_attr->context_alloc = _aio_context_alloc;
		pool_attr->unblock_func = _aio_unblock;
		pool_attr->handler_func = _aio_handler;
		pool_attr->context_free = _aio_context_free;
	}
	pool_attr->handle = (void *)cb;

	/* prepare some priority list entries */
	for (i = 0; i < _AIO_PRIO_LIST_LOW; i++) {
		plist = (struct _aio_prio_list *)malloc(sizeof(*plist));
		if (!plist) {
			goto err_ret;
		}
		plist->next = cb->cb_plist_free;
		cb->cb_plist_free = plist;
	}
	cb->cb_nfree = _AIO_PRIO_LIST_LOW;
	
	/* prepare the context */
	cb->ct_free = NULL;
	for (i = 0; i < pool_attr->maximum; i++) {
		if ((ctp = malloc(sizeof(*ctp))) == NULL) {
			goto err_ret;
		}
		ctp->next = cb->ct_free;
		cb->ct_free = ctp;
	}
	
	cb->tp = thread_pool_create(pool_attr, 0);
	if (cb->tp == NULL) {
		goto err_ret;
	}

	/* we can hook _aio_cb now */
	_aio_cb = cb;

	/* start the pool with all sigal blocked */
	if (sigfillset(&set) != 0 || (errno = pthread_sigmask(SIG_BLOCK, &set, &oset)) != EOK) {
		goto err_ret;
	}
	if (thread_pool_start(cb->tp) != EOK) {
		pthread_sigmask(SIG_SETMASK, &oset, NULL);
		goto err_ret;
	}
	pthread_sigmask(SIG_SETMASK, &oset, NULL);
	_mutex_unlock(&_aio_init_mutex);
	return 0;
		
err_ret:
	_mutex_lock(&cb->cb_mutex);
	
	if (cb->tp) {
		(void)thread_pool_destroy(cb->tp);
	}
	
	while ((plist = cb->cb_plist_free)) {
		cb->cb_plist_free = plist->next;
		free(plist);
	}
	
	while ((ctp = cb->ct_free)) {
		cb->ct_free = ctp->next;
		free(ctp);
	}

	pthread_cond_destroy(&cb->cb_cond);
	pthread_mutex_destroy(&cb->cb_mutex);
	
	free(cb);
	_mutex_unlock(&_aio_init_mutex);
	return -1;
}

/* insert an aiocbp into the list according its priority */
int _aio_insert_prio(struct aiocb *aiocbp)
{
	struct _aio_prio_list *plist, **pplist;
	
	for (pplist = &_aio_cb->cb_plist, plist = *pplist; plist; 
		 pplist = &plist->next, plist = *pplist) {
		if (plist->priority <= aiocbp->_aio_param.__sched_priority)
		  break;
	}
	
	if (!plist || plist->priority != aiocbp->_aio_param.__sched_priority) {
		if ((plist = _aio_cb->cb_plist_free) == NULL) {
			if ((plist = (struct _aio_prio_list *)malloc(sizeof(*plist))) == NULL)
			{
				errno = EAGAIN;
				return -1;
			}
		} else {
			_aio_cb->cb_plist_free = plist->next;
			_aio_cb->cb_nfree--;
		}
		
		plist->head = NULL;
		plist->tail = &plist->head;
		plist->priority = aiocbp->_aio_param.__sched_priority;
		plist->next = *pplist;
		*pplist = plist;
	}
	
	aiocbp->_aio_plist = (void *)plist;
	aiocbp->_aio_next = NULL;
	*plist->tail = aiocbp;
	plist->tail = &aiocbp->_aio_next;
	return 0;
}

int _aio_insert(struct aiocb *aiocbp)
{
	if ((errno = pthread_getschedparam(pthread_self(), &aiocbp->_aio_policy, (struct sched_param *)&aiocbp->_aio_param)) != EOK) {
		errno = EAGAIN;
		aiocbp->_aio_flag = 0;
		return -1;
	}
	if (aiocbp->_aio_iotype == _AIO_OPCODE_READ || aiocbp->_aio_iotype == _AIO_OPCODE_WRITE) {
		if (aiocbp->_aio_param.__sched_priority <= aiocbp->aio_reqprio || aiocbp->aio_reqprio < 0) {
			errno = EINVAL;
			aiocbp->_aio_flag = 0;
			return -1;
		}
		aiocbp->_aio_param.__sched_priority -= aiocbp->aio_reqprio;
	}
	aiocbp->_aio_result = 0;
	aiocbp->_aio_error = EINPROGRESS;
	aiocbp->_aio_flag = _AIO_FLAG_ENQUEUE;

	/* insert it */
	_mutex_lock(&_aio_cb->cb_mutex);
	if (_aio_insert_prio(aiocbp) == -1) {
		_mutex_unlock(&_aio_cb->cb_mutex);
		return -1;
	}
	pthread_cond_signal(&_aio_cb->cb_cond);
	_mutex_unlock(&_aio_cb->cb_mutex);

	return 0;
}

int _aio_destroy() 
{
	struct _aio_control_block *cb;
	struct _aio_context *ctp;
	struct _aio_prio_list *plist;
	thread_pool_t       *tp;
	
	cb = _aio_cb;
	if ((tp = (thread_pool_t *)_smp_xchg((unsigned *)&_aio_cb->tp, 0)) == 0)
	  return 0;

	/* kill every thread */
	if (thread_pool_destroy(tp) != 0) 
	  return -1;

	while ((plist = cb->cb_plist)) {
		cb->cb_plist = plist->next;
		free(plist);
	}
	
	while ((plist = cb->cb_plist_free)) {
		cb->cb_plist_free = plist->next;
		free(plist);
	}
	
	while ((ctp = cb->ct_free)) {
		cb->ct_free = ctp->next;
		free(ctp);
	}
	pthread_mutex_destroy(&cb->cb_mutex);
	pthread_cond_destroy(&cb->cb_cond);
	free(cb);

	pthread_mutex_destroy(&_aio_init_mutex);
	return 0;
}

__SRCVERSION("_aio_init.c $Rev: 168079 $");
