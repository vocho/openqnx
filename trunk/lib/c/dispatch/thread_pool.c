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
#include <atomic.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <sys/dispatch.h>
#include "dispatch.h"

void *_thread_pool_context_thread(void *data);
void _thread_pool_attr_normalise(thread_pool_attr_t *tpattr);

thread_pool_t *thread_pool_create(thread_pool_attr_t *attr, unsigned flags) {
	thread_pool_t			*pool;
	struct _pool_properties *props;
	static const pthread_mutex_t _pool_mutex = PTHREAD_MUTEX_INITIALIZER;
	static const pthread_cond_t _pool_cond = PTHREAD_COND_INITIALIZER;

	if((pool = malloc(sizeof *pool)) == NULL) {
		return NULL;
	}
	memset(pool,0,sizeof *pool);
	memcpy(pool,attr,sizeof *attr);
	// @@@ adjust/validate params
	pool->flags = flags & ~(POOL_FLAG_EXITING | POOL_FLAG_CHANGING);
	pool->props = (struct _pool_properties *)calloc(1, 
                       sizeof(struct _pool_properties));
	if (pool->props == NULL) {
		free(pool);
		return(NULL);
	}
	props = (struct _pool_properties *)pool->props;
	props->inline_lock = _pool_mutex;
	props->pool_cond = _pool_cond;
	return(pool);
}


int thread_pool_start(void *tp) {
	thread_pool_t			*pool = tp;
	struct _pool_properties *props;

	// normalise values to be reasonably sane
	_thread_pool_attr_normalise(&pool->pool_attr);

	props = (struct _pool_properties *)pool->props;
	// we do not need to lock here, since we are alone now
	pool->created++;
	props->newthreads++;
  	if (pool->flags & POOL_FLAG_RESERVE)
    		props->reserved_threads++;
	if(pool->flags & POOL_FLAG_USE_SELF) {
		// Use ourself, never returns
		(void)_thread_pool_context_thread(pool);
	} else if(pthread_create(0, pool->pool_attr.attr, 
                  _thread_pool_context_thread, pool) != EOK) {
		// we do not need to lock here, since we are alone now
		pool->created--;
		props->newthreads--;
		return (-1);  
	}

	if(pool->flags & POOL_FLAG_EXIT_SELF) {
		pthread_detach(pthread_self());
		pthread_exit(NULL);
	}
	return EOK;
}

__SRCVERSION("thread_pool.c $Rev: 198218 $");
