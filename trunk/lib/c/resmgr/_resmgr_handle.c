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
#include <errno.h>
#include <string.h>
#include <sys/resmgr.h>
#include <sys/netmgr.h>
#include "resmgr.h"

#define RECYCLE_ENTRY(p) do { \
	if(_resmgr_io_table.total > _resmgr_io_table.min && _resmgr_io_table.free >= _RESMGR_OBJ_LOWAT) { \
		_resmgr_io_table.total--; \
		free(p); \
	} else { \
		(p)->next = _resmgr_io_table.free_list; \
		_resmgr_io_table.free_list = (p); \
		_resmgr_io_table.free++; \
	} \
} while (0)

#define RECYCLE_BUCKET(list) do { \
	if((list)->nlists > _RESMGR_CLIENT_FD_MIN || (_resmgr_io_table.total_buckets > _resmgr_io_table.min_buckets && \
	    _resmgr_io_table.nfree_buckets >= _RESMGR_OBJ_LOWAT)) { \
		_resmgr_io_table.total_buckets--; \
		free((list)->lists); \
	} \
	else { \
		((union _resmgr_entry_bucket *)((list)->lists))->next = _resmgr_io_table.free_buckets; \
		_resmgr_io_table.free_buckets = (union _resmgr_entry_bucket *)((list)->lists); \
		_resmgr_io_table.nfree_buckets++; \
	} \
	(list)->lists = NULL; \
	(list)->nlists = 0; \
	(list)->nlists_max = _resmgr_io_table.nlists_max; \
} while (0)

void *_resmgr_handle(struct _msg_info *rep, void *handle, enum _resmgr_handle_type type) {
	struct _resmgr_handle_entry			*p;
	struct _resmgr_handle_list			*list;
	int									scoid, tmp;
	unsigned							lidx;
	unsigned							lock;
	struct _resmgr_handle_entry			**newlists;

	lock = type & _RESMGR_HANDLE_LOCK;
	type &= ~_RESMGR_HANDLE_LOCK;

	if(type == _RESMGR_HANDLE_SET && handle == (void *)-1) {
		errno = EINVAL;
		return (void *)-1;
	}

	_mutex_lock(&_resmgr_io_table.mutex);

	if(rep->scoid < 0) {
		for(list = _resmgr_io_table.vector, scoid = 0; scoid < _resmgr_io_table.nentries; list++, scoid++) {
			if(list->list && rep->pid == list->pid && ND_NODE_CMP(rep->nd, list->nd) == 0) {
				break;
			}
		}
		if(scoid >= _resmgr_io_table.nentries) {
			_mutex_unlock(&_resmgr_io_table.mutex);
			errno = ESRCH;
			return (void *)-1;
		}
	} else {
		scoid = rep->scoid & ~_NTO_SIDE_CHANNEL;
		if(scoid >= _resmgr_io_table.nentries) {
			if(type != _RESMGR_HANDLE_SET) {
				_mutex_unlock(&_resmgr_io_table.mutex);
				errno = ESRCH;
				return (void *)-1;
			}
			if((list = realloc(_resmgr_io_table.vector, (scoid + 1) * sizeof *list))) {
				_resmgr_io_table.vector = list;
				memset(&list[_resmgr_io_table.nentries], 0x00, ((scoid - _resmgr_io_table.nentries) + 1) * sizeof *list);
				/*
				 * Set the current hash size here, when entry is reused (RECYCLE_BUCKET())
				 * and when explicitly overriden (remgr_handle_tune()).  This means
				 * new hash sizes come into effect when new scoids (clients) connect
				 * not immediately for existing ones; but, saves having to resort existing
				 * scoids or checking current hash size on every access.
				 */
				for(tmp = _resmgr_io_table.nentries; tmp < scoid + 1; tmp++) {
					list[tmp].nlists_max = _resmgr_io_table.nlists_max;
				}
				_resmgr_io_table.nentries = scoid + 1;
			} else {
				_mutex_unlock(&_resmgr_io_table.mutex);
				errno = ENOMEM;
				return (void *)-1;
			}
		}
		list = &_resmgr_io_table.vector[scoid];
		if(type == _RESMGR_HANDLE_DISCONNECT) {
			while((p = list->list)) {
				int								status;

				do {
					if(!(p->coid & lock)) {
						rep->coid = p->coid;
						rep->pid = list->pid;
						rep->nd = list->nd;
						rep->tid = 0;
						p->coid |= lock;
						handle = p->handle;
						_mutex_unlock(&_resmgr_io_table.mutex);
						return handle;
					}
				} while((p = p->next));

				list->waiting++;
				status = pthread_cond_wait(&_resmgr_io_table.cond, &_resmgr_io_table.mutex);
				/* Reset in case realloced while mutex unlocked */
				list = &_resmgr_io_table.vector[scoid];
				if(status != EOK && status != EINTR) {
					list->waiting--;
					_mutex_unlock(&_resmgr_io_table.mutex);
					errno = status;
					return (void *)-1;
				}
				list->waiting--;
			}
		} else if((p = list->list)) {
			if(rep->pid != list->pid || ND_NODE_CMP(rep->nd, list->nd) != 0) {
				if(type != _RESMGR_HANDLE_SET) {
					_mutex_unlock(&_resmgr_io_table.mutex);
					errno = ESRCH;
					return (void *)-1;
				}
				while((p = list->list)) {
					list->list = p->next;
					RECYCLE_ENTRY(p);
				}
				RECYCLE_BUCKET(list);
				if(list->waiting) {
					pthread_cond_broadcast(&_resmgr_io_table.cond);
				}
			}
		}
	}

	for(;;) {
		int								status;

		lidx = rep->coid % list->nlists_max;

		if(lidx >= list->nlists) {
			p = NULL;
			break;
		}

		for(p = list->lists[lidx]; p; p = p->next) {
			if((tmp = p->coid & ~_RESMGR_HANDLE_LOCK) == rep->coid) {
				break;
			}
			if(tmp % list->nlists_max != lidx) {
				p = NULL;
				break;
			}
		}
		if(!p || !(p->coid & lock)) {
			break;
		}
		list->waiting++;
		status = pthread_cond_wait(&_resmgr_io_table.cond, &_resmgr_io_table.mutex);
		/* Reset in case realloced while mutex unlocked */
		list = &_resmgr_io_table.vector[scoid];
		if(status != EOK && status != EINTR) {
			list->waiting--;
			_mutex_unlock(&_resmgr_io_table.mutex);
			errno = status;
			return (void *)-1;
		}
		list->waiting--;
	}

	if(p) {
		if(rep->pid != list->pid || ND_NODE_CMP(rep->nd, list->nd) != 0) {
			_mutex_unlock(&_resmgr_io_table.mutex);
			errno = EINVAL;
			return (void *)-1;
		}
		if(type == _RESMGR_HANDLE_SET) {
			_mutex_unlock(&_resmgr_io_table.mutex);
			errno = EBUSY;
			return (void *)-1;
		}
	} else {
		if(type != _RESMGR_HANDLE_SET) {
			_mutex_unlock(&_resmgr_io_table.mutex);
			errno = ESRCH;
			return (void *)-1;
		}
		if(!list->list) {
			list->nd = rep->nd;
			list->pid = rep->pid;
		}

		if(lidx >= list->nlists) {
			tmp = _RESMGR_CLIENT_FD_MIN;
			while(lidx >= tmp)
				tmp <<= 1;
			tmp = min(tmp, list->nlists_max);
			if(list->nlists == 0 && tmp == _RESMGR_CLIENT_FD_MIN && _resmgr_io_table.free_buckets) {
				newlists = &_resmgr_io_table.free_buckets->lists;
				_resmgr_io_table.free_buckets = _resmgr_io_table.free_buckets->next;
				_resmgr_io_table.nfree_buckets--;
			}
			else {
				if((newlists = realloc(list->lists, tmp * sizeof(*newlists))) == NULL) {
					_mutex_unlock(&_resmgr_io_table.mutex);
					errno = ENOMEM;
					return (void *)-1;
				}
				_resmgr_io_table.total_buckets += (list->nlists == 0 ? 1 : 0);
			}
			memset(&newlists[list->nlists], 0x00, (tmp - list->nlists) * sizeof(*newlists));
			list->lists = newlists;
			list->nlists = tmp;
		}

		if((p = _resmgr_io_table.free_list)) {
			_resmgr_io_table.free_list = p->next;
			_resmgr_io_table.free--;
		} else if((p = malloc(sizeof *p))) {
			_resmgr_io_table.total++;
		} else {
			_mutex_unlock(&_resmgr_io_table.mutex);
			errno = ENOMEM;
			return (void *)-1;
		}


		if(list->lists[lidx] == NULL) {
			if((p->next = list->list))
				p->next->prev = &p->next;
			/*
			 * p->prev = &list->list is expected; however,
			 * list may be realloc'd above so we can't save
			 * a pointer to it.  Use p->prev == NULL as an
			 * indication it's at the head.  This means an
			 * extra if just below and one on removal.
			 */
			p->prev = NULL;
			list->list = p;
		}
		else {
			if((p->prev = list->lists[lidx]->prev))
				*p->prev = p;
			else
				list->list = p;
			list->lists[lidx]->prev = &p->next;

			p->next = list->lists[lidx];
		}
		list->lists[lidx] = p;

		p->coid = rep->coid;
		p->handle = handle;
	}

	if(type == _RESMGR_HANDLE_REMOVE) {
		if(handle != NULL && p->handle != handle) {
			_mutex_unlock(&_resmgr_io_table.mutex);
			errno = ESRCH;
			return (void *)-1;
		}

		handle = p->handle;

		if(list->lists[lidx] == p && (list->lists[lidx] = p->next) &&
		    ((p->next->coid & ~_RESMGR_HANDLE_LOCK) % list->nlists_max) != lidx) {
			list->lists[lidx] = NULL;
		}

		if(p->next)
			p->next->prev = p->prev;
		if(p->prev)
			*p->prev = p->next;
		else if((list->list = p->next) == NULL)
			RECYCLE_BUCKET(list);

		RECYCLE_ENTRY(p);
		if(list->waiting) {
			pthread_cond_broadcast(&_resmgr_io_table.cond);
		}
		_mutex_unlock(&_resmgr_io_table.mutex);
		return handle;
	}

	p->coid |= lock;

	if(type == _RESMGR_HANDLE_UNLOCK) {
		if((p->coid & _RESMGR_HANDLE_LOCK) && list->waiting) {
			pthread_cond_broadcast(&_resmgr_io_table.cond);
		}
		p->coid &= ~_RESMGR_HANDLE_LOCK;
	}

	handle = p->handle;
	_mutex_unlock(&_resmgr_io_table.mutex);
	return handle;
}

__SRCVERSION("_resmgr_handle.c $Rev: 153052 $");
