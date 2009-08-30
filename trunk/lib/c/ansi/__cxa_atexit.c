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
#include <malloc.h>
#include "atexit.h"

int __cxa_atexit (void (*func) (void *), void *arg, void *dso) {
	struct atexit_func			*f;

	if((f = malloc(sizeof *f))) {
		f->cxa.func       = (void (*) (void *, int)) func;
		f->cxa.arg        = arg;
		f->cxa.dso_handle = dso;
		f->func = NULL;
		f->next = _atexit_list;
		_atexit_list = f;
		return 0;
	}
	return -1;
}

__SRCVERSION("__cxa_atexit.c $Rev: 153052 $");
