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
#include <unistd.h>
#include <errno.h>
#include <sys/resmgr.h>
#include "resmgr.h"

int _resmgr_link_free(int id, unsigned flags) {
	register struct link				*p, **pp;

	_mutex_lock(&_resmgr_io_table.mutex);
	for(pp = &_resmgr_link_list; (p = *pp); pp = &p->next) {
		if(p->id == id) {
			if(p->link_id != -1) {
				close(p->link_id);
				p->link_id = -1;
			}
			if(!(flags & _RESMGR_DETACH_PATHNAME)) {
				*pp = p->next;
				free(p);
			}
			_mutex_unlock(&_resmgr_io_table.mutex);
			return 0;
		}
	}
	errno = EINVAL;
	_mutex_unlock(&_resmgr_io_table.mutex);
	return -1;
}

__SRCVERSION("_resmgr_link_free.c $Rev: 153052 $");
