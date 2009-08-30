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




#include <malloc.h>
#include <sys/resmgr.h>
#include "resmgr.h"

static pthread_once_t	_resmgr_key_once =  PTHREAD_ONCE_INIT;	

static void key_once_func(void) {
	(void)pthread_key_create(&_resmgr_thread_key, NULL);
}

struct link *_resmgr_link_alloc(void) {
	register struct link				*p, **pp;
	register int						id;

	/*
	 We do the key initialization here for a couple of reasons.
	 First it is a low bandwidth call, so we don't waste our
	 time checking that it has been called before.
	 Second it is called by the name attach functions of both
	 the dispatch library and the historical resmgr code so
	 we only do this if the client is attaching a name.
	*/
	pthread_once(&_resmgr_key_once, key_once_func);

	_mutex_lock(&_resmgr_io_table.mutex);
	for(id = 0, pp = &_resmgr_link_list; (p = *pp) && p->id == id; pp = &p->next, id++) {
		/* nothing to do */
	}
	if(!(p = calloc(sizeof *p, 1))) {
		_mutex_unlock(&_resmgr_io_table.mutex);
		return 0;
	}
	p->id = id;
	p->link_id = -1;
	p->flags = _RESMGR_LINK_HALFOPEN;
	p->next = *pp;
	*pp = p;
	_mutex_unlock(&_resmgr_io_table.mutex);
	return p;
}

__SRCVERSION("_resmgr_link_alloc.c $Rev: 153052 $");
