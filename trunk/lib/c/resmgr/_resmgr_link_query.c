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




#include <atomic.h>
#include <sys/resmgr.h>
#include "resmgr.h"


struct link *_resmgr_link_query(int id, int countadj) {
	register struct link				*p;

	_mutex_lock(&_resmgr_io_table.mutex);
	for(p = _resmgr_link_list; p; p = p->next) {
		if(p->id == id && !(p->flags & _RESMGR_LINK_DETACHWAIT)) {
			atomic_add(&p->count, countadj);
			_mutex_unlock(&_resmgr_io_table.mutex);
			return p;
		} else if(p->id >= id) {
			break;
		}
	}
	_mutex_unlock(&_resmgr_io_table.mutex);
	return 0;
}

void _resmgr_link_return(struct link *link, int countadj) {
	atomic_sub(&link->count, countadj);

	/* Any time that this flag is raised, then once the count
	 drops below one we start signalling.  There can only be
	 one person waiting at a time in any case, but they might
	 have come in from a link handler.
	*/
	if((link->flags & _RESMGR_LINK_DETACHWAIT) && (link->count <= 1)) {
		pthread_sleepon_lock();
		(void)pthread_sleepon_broadcast(link);
		pthread_sleepon_unlock();
	}
}


__SRCVERSION("_resmgr_link_query.c $Rev: 162224 $");
