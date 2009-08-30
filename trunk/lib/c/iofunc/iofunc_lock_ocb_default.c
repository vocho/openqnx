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
#include <stddef.h>
#include <sys/iofunc.h>

int iofunc_lock_ocb_default(resmgr_context_t *ctp, void *reserved, iofunc_ocb_t *ocb) {
	int				(*attr_lock)(iofunc_attr_t *);
    iofunc_funcs_t	*funcs;

	if (ocb->attr->mount == NULL || (funcs = ocb->attr->mount->funcs) == NULL || funcs->nfuncs < (offsetof(iofunc_funcs_t, attr_unlock) / sizeof(void *)) || (attr_lock = funcs->attr_lock) == NULL || funcs->attr_unlock == NULL) {
		attr_lock = iofunc_attr_lock;
	}

	return (*attr_lock)(ocb->attr);
}

__SRCVERSION("iofunc_lock_ocb_default.c $Rev: 153052 $");
