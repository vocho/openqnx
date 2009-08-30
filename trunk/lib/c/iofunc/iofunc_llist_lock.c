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

int _iofunc_llist_lock(iofunc_attr_t *attr) {
	int					status;

	if((status = _sleepon_lock(&_iofunc_sleepon_default)) != EOK) {
		return status;
	}

	while(PTR_ISLOCKED(attr->lock_list)) {
		if((status = _sleepon_wait(&_iofunc_sleepon_default, PTR_WCHAN(attr->lock_list), 0)) != EOK && status != EINTR) {
			_sleepon_unlock(&_iofunc_sleepon_default);
			return status;
		}
	}

	PTR_LOCK(attr->lock_list);

	return _sleepon_unlock(&_iofunc_sleepon_default);
}

int _iofunc_llist_unlock(iofunc_attr_t *attr) {
	int						status;

	if((status = _sleepon_lock(&_iofunc_sleepon_default)) != EOK) {
		return status;
	}

	PTR_UNLOCK(attr->lock_list);

	if((status = _sleepon_signal(&_iofunc_sleepon_default, PTR_WCHAN(attr->lock_list))) != EOK) {
		_sleepon_unlock(&_iofunc_sleepon_default);
		return status;
	}

	return _sleepon_unlock(&_iofunc_sleepon_default);	
}

__SRCVERSION("iofunc_llist_lock.c $Rev: 153052 $");
