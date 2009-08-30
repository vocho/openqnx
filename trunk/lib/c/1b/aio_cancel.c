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
#include <fcntl.h>
#include <aio_priv.h>

int aio_cancel(int fd, struct aiocb *aiocbp)
{
	unsigned flag, ret;
	int ncancled, ndone, nprogress;
	struct aiocb *ap, **app;
	struct _aio_prio_list *plist, **pplist;
	struct _server_info sinfo;
	
	if (!_aio_cb && _aio_init(NULL)) {
		return -1;
	}

	_mutex_lock(&_aio_cb->cb_mutex);
	/* a single aiocbp */
	if (aiocbp) {
		flag = atomic_clr_value(&aiocbp->_aio_flag, _AIO_FLAG_QMASK);
		if (flag & _AIO_FLAG_DONE) {
			ret = AIO_ALLDONE;
		} else if (flag & _AIO_FLAG_IN_PROGRESS) {
			ret = AIO_NOTCANCELED;
		} else if (flag & _AIO_FLAG_ENQUEUE) {
			if ((plist = (struct _aio_prio_list *)aiocbp->_aio_plist) == NULL) {
				errno = EINVAL;
				return -1;
			}

			/* find the aiocbp */
			for (app = &plist->head, ap = *app; ap; app = &ap->_aio_next, ap = ap->_aio_next) {
				if (ap == aiocbp && (*app = ap->_aio_next) == NULL) {
					plist->tail = app;
					/* Even if there is no request hang on this priority list,
					 * we will not remove it now. remove a priority list entry
					 * causing a single list search, and we have reason to
					 * believe the entry will be reused soon.
					 * The empty entry will be removed later on, by the
					 * _aio_block() internal function, when it try to get a
					 * request.
					 */
#if 0
					if (plist->head == NULL) {
						for (pplist = &_aio_cb->cb_plist; *pplist; pplist = &((*pplist)->next)) {
							if (*pplist == plist) {
								*pplist = plist->next;
								plist->next = _aio_cb->cb_plist_free;
								_aio_cb->cb_plist_free = plist;
								break;
							}
						}
					}
#endif
				}
				break;
			}
			
			if (flag & _AIO_FLAG_SUSPEND) {
				struct _aio_waiter *w = aiocbp->_aio_suspend;
				
				aiocbp->_aio_suspend = NULL;
				if (w && atomic_sub_value(&w->w_count, 1) == 0) {
					  pthread_cond_signal(&w->w_cond);
				}
			}
			aiocbp->_aio_flag = 0;
			aiocbp->_aio_error = ECANCELED;
			aiocbp->_aio_result = (unsigned)-1;
			ret = AIO_CANCELED;
		} else {
			/* we haven't seen this yet */
			errno = EINVAL;
			ret = (unsigned)-1;
		}
		_mutex_unlock(&_aio_cb->cb_mutex);
		return ret;
	}

	/* got to scan the whole list for fd */
	ncancled = ndone = nprogress = 0;
	for (pplist = &_aio_cb->cb_plist, plist = *pplist; plist; pplist = &plist->next, plist = *pplist)
	{
		for (app = &plist->head, ap = *app; ap; ap = *app) {
			if (ap->aio_fildes == fd)
			{
				flag = atomic_clr_value(&ap->_aio_flag, _AIO_FLAG_QMASK);
				if (flag & _AIO_FLAG_DONE) {
					ndone++;
				} else if (flag & _AIO_FLAG_IN_PROGRESS) {
					nprogress++;
				} else if (flag & _AIO_FLAG_ENQUEUE) {
					if ((*app = ap->_aio_next) == NULL) {
						plist->tail = app;
						/* Don't remove empty entry (see above), leave it
						 * for _aio_block().
						 */
#if 0
						if (plist->head == NULL) {
							for (pplist = &_aio_cb->cb_plist; *pplist; pplist = &((*pplist)->next)) {
								if (*pplist == plist) {
									*pplist = plist->next;
									plist->next = _aio_cb->cb_plist_free;
									_aio_cb->cb_plist_free = plist;
								}
							}
						}
#endif
					}
					ap->_aio_flag = 0;
					ap->_aio_error = ECANCELED;
					ap->_aio_result = (unsigned)-1;
					ncancled++;
				}
			} else {
				app = &ap->_aio_next;
			}
		}
	}
	_mutex_unlock(&_aio_cb->cb_mutex);
	
	if (nprogress) 
	  return AIO_NOTCANCELED;
	
	if (!ncancled) {
		if (ndone) {
			/* all affected operations have completed */ 
			return AIO_ALLDONE;
		} else {
			/* no operations matched the supplied fd, POSIX required to check
			 * if this is a valid fd. PR#51564
			 */
			if (ConnectServerInfo(0, fd, &sinfo) != fd) {
				errno = EBADF;
				return (unsigned)-1;
			} else {
				return AIO_ALLDONE;
			}
		}
	}

	return AIO_CANCELED;
}

__SRCVERSION("aio_cancel.c $Rev: 158434 $");
