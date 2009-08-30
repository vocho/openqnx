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
#include "atexit.h"

void
__cxa_finalize (void *dso) {
	struct atexit_func *cur, *prev;

	for(prev = NULL, cur = _atexit_list; cur; ) {
		if((dso == NULL || cur->cxa.dso_handle == dso) &&
			cur->cxa.func ) {
			void (*func) (void *arg, int status) = cur->cxa.func;
			cur->cxa.func = NULL; /* allow recursive exit() */
			func (cur->cxa.arg, 0);
			if (prev) {
				prev->next = cur->next;
				free(cur);
				cur = prev->next;
			} else {
				_atexit_list = cur->next;
				free(cur);
				cur = _atexit_list;
			}
		} else {
			prev = cur;
			cur = cur->next;
		}
	}
}

__SRCVERSION("__cxa_finalize.c $Rev: 153052 $");
