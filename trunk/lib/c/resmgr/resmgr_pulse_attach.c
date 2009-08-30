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





#if (defined(__GNUC__) || defined(__INTEL_COMPILER)) && !defined(__MIPS__)
void resmgr_pulse_attach() __attribute__ ((weak, alias("_resmgr_pulse_attach")));
#endif
#include <stdlib.h>
#include <errno.h>
#define RESMGR_COMPAT
#include <sys/resmgr.h>
#include "resmgr.h"

int resmgr_pulse_attach(int code, void (*func)(resmgr_context_t *ctp, int code, union sigval value, void *handle), void *handle) {
	register struct pulse_func			*p, **pp;

	_mutex_lock(&_resmgr_io_table.mutex);
	if(code == _RESMGR_PULSE_ALLOC) {
		code = _PULSE_CODE_MINAVAIL;
		for(pp = &_resmgr_pulse_list; (p = *pp) && code <= _PULSE_CODE_MAXAVAIL && p->code <= code; pp = &p->next) {
			if(p->code == code) {
				code++;
			}
		}
		if(code > _PULSE_CODE_MAXAVAIL) {
			errno = EAGAIN;
			_mutex_unlock(&_resmgr_io_table.mutex);
			return -1;
		}
	} else if(code >= SCHAR_MIN && code <= SCHAR_MAX) {
		for(pp = &_resmgr_pulse_list; (p = *pp) && code > p->code; pp = &p->next) {
			/* nothing to do */
		}
		if(p && (p->code == code)) {
			errno = EINVAL;
			_mutex_unlock(&_resmgr_io_table.mutex);
			return -1;
		}
	} else {
		errno = EINVAL;
		_mutex_unlock(&_resmgr_io_table.mutex);
		return -1;
	}
	if(!(p = calloc(sizeof *p, 1))) {
		errno = ENOMEM;
		_mutex_unlock(&_resmgr_io_table.mutex);
		return -1;
	}
	p->next = *pp;
	*pp = p;
	p->code = code;
	p->func = func;
	p->handle = handle;
	_mutex_unlock(&_resmgr_io_table.mutex);
	return code;
}

__SRCVERSION("resmgr_pulse_attach.c $Rev: 153052 $");
