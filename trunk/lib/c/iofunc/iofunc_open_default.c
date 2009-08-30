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
#include <unistd.h>
#include <errno.h>
#include <sys/iofunc.h>

int iofunc_open_default(resmgr_context_t *ctp, io_open_t *msg, iofunc_attr_t *attr, void *extra) {
	int				(*attr_lock)(iofunc_attr_t *), (*attr_unlock)(iofunc_attr_t *);
    iofunc_funcs_t	*funcs;
	int				status;

	if (attr->mount == NULL || (funcs = attr->mount->funcs) == NULL || funcs->nfuncs < (offsetof(iofunc_funcs_t, attr_unlock) / sizeof(void *)) || (attr_lock = funcs->attr_lock) == NULL || (attr_unlock = funcs->attr_unlock) == NULL) {
		attr_lock = iofunc_attr_lock, attr_unlock = iofunc_attr_unlock;
	}

	(void)(*attr_lock)(attr);

	if((status = iofunc_open(ctp, msg, attr, 0, 0)) != EOK) {
		(void)(*attr_unlock)(attr);
		return status;
	}

	if((status = iofunc_ocb_attach(ctp, msg, 0, attr, 0)) != EOK) {
		(void)(*attr_unlock)(attr);
		return status;
	}

	(void)(*attr_unlock)(attr);
	return EOK;
}

__SRCVERSION("iofunc_open_default.c $Rev: 153052 $");
