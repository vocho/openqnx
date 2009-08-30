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
#include <sys/iofunc.h>
#include "iofunc.h"

int iofunc_attr_trylock(iofunc_attr_t *attr) {
	pthread_t			tid = pthread_self();
	int					status;

	if((status = _sleepon_lock(&_iofunc_sleepon_default)) != EOK) {
		return status;
	}
	if(attr->lock_count && tid != attr->lock_tid) {
		_sleepon_unlock(&_iofunc_sleepon_default);
		return EBUSY;
	}
	attr->lock_tid = tid;
	attr->lock_count++;
	return _sleepon_unlock(&_iofunc_sleepon_default);
}

__SRCVERSION("iofunc_attr_trylock.c $Rev: 153052 $");
