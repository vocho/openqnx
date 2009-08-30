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
#include <sys/resmgr.h>
#include "resmgr.h"

int _resmgr_handle_grow(unsigned min);

#if (defined(__GNUC__) || defined(__INTEL_COMPILER)) && !defined(__MIPS__)
int resmgr_handle_grow(unsigned min) __attribute__ ((alias("_resmgr_handle_grow")));
#else
int resmgr_handle_grow(unsigned min) {
	return _resmgr_handle_grow(min);
}
#endif

int _resmgr_handle_grow(unsigned min) {
	struct _resmgr_handle_entry			*p;

	if(_mutex_lock(&_resmgr_io_table.mutex) != EOK) {
		return -1;
	}
	_resmgr_io_table.min = min;
	while(_resmgr_io_table.total < min) {
		if((p = malloc(sizeof *p))) {
			p->next = _resmgr_io_table.free_list;
			_resmgr_io_table.free_list = p;
			_resmgr_io_table.total++;
			_resmgr_io_table.free++;
		} else {
			break;
		}
	}
	_mutex_unlock(&_resmgr_io_table.mutex);
	return _resmgr_io_table.free;
}


__SRCVERSION("_resmgr_handle_grow.c $Rev: 153052 $");
