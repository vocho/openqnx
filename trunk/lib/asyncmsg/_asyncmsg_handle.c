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




#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include "asyncmsg_priv.h"

#define _ASYNCMSG_HANDLE_LIST_GROW 4

struct _asyncmsg_handle_list {
	pthread_rwlock_t rwlock;
	void **array;
	int array_total;
};

static struct _asyncmsg_handle_list _channel_list = {
	PTHREAD_RWLOCK_INITIALIZER, 0, 0 
};

static struct _asyncmsg_handle_list _fd_list = {
	PTHREAD_RWLOCK_INITIALIZER, 0, 0
};

static struct _asyncmsg_handle_list _connect_list = {
	PTHREAD_RWLOCK_INITIALIZER, 0, 0
};

void * _asyncmsg_handle(int id, int cmd, void *handle)
{
	struct _asyncmsg_handle_list *list;
	
	if (cmd & _ASYNCMSG_HANDLE_CHANNEL) {
		cmd &= ~_ASYNCMSG_HANDLE_CHANNEL;
		list = &_channel_list;
	} else if (id & _NTO_SIDE_CHANNEL) {
		list = &_connect_list;
		id &= ~_NTO_SIDE_CHANNEL;
	} else {
		list = &_fd_list;
	}
	
	if (cmd == _ASYNCMSG_HANDLE_ADD) {
		if ((errno = pthread_rwlock_wrlock(&list->rwlock)) != EOK) {
			return NULL;
		}
		if (list->array_total <= id) {
			void **new_array;
			size_t new_size;
			int extra;
			

			if ((extra = id - list->array_total + 1) < _ASYNCMSG_HANDLE_LIST_GROW) 
			  extra = _ASYNCMSG_HANDLE_LIST_GROW;
			new_size = (list->array_total + extra) * sizeof(void *);
			if ((new_array = realloc(list->array, new_size)) == NULL) {
				pthread_rwlock_unlock(&list->rwlock);
				return NULL;
			}
			memset(&new_array[list->array_total], 0, extra * sizeof(void *));
			list->array = new_array;
			list->array_total += extra;
		}
		list->array[id] = handle;
		pthread_rwlock_unlock(&list->rwlock);
		return handle;
	}
	
	if (id >= list->array_total) {
		errno = ESRCH;
		return NULL;
	}
	
	if ((errno = pthread_rwlock_rdlock(&list->rwlock)) != EOK) {
		return NULL;
	}
	
	handle = list->array[id];
	if (cmd & _ASYNCMSG_HANDLE_DELETE) {
		list->array[id] = NULL;
	}
	pthread_rwlock_unlock(&list->rwlock);

	if (!handle)
	  errno = ESRCH;
	
	return handle;
}

__SRCVERSION("_asyncmsg_handle.c $Rev: 157149 $");
