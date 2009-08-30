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




#include <pthread.h>
#include "sleepon.h"

/* Sleepon functions which are specific to the iofunc layer
   so it doesn't have to contend with the rest of the system. */

struct _sleepon_handle _iofunc_sleepon_default = {
    PTHREAD_MUTEX_INITIALIZER, NULL, NULL, 0, 1
};

#if 0	/* Just go right to the sleepon functions */
int (iofunc_sleepon_lock)(void) {
		return _sleepon_lock(&_iofunc_sleepon_default);
}

int (iofunc_sleepon_unlock)(void) {
		return _sleepon_unlock(&_iofunc_sleepon_default);
}

int (iofunc_sleepon_wait)(void *addr) {
        return _sleepon_wait(&_iofunc_sleepon_default, addr, 0);
}

int (iofunc_sleepon_signal)(void *addr) {
        return _sleepon_signal(&_iofunc_sleepon_default, addr);
}

int (iofunc_sleepon_broadcast)(void *addr) {
        return _sleepon_broadcast(&_iofunc_sleepon_default, addr);
}
#endif

__SRCVERSION("iofunc_sleepon.c $Rev: 153052 $");
