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
#include "iofunc.h"

int iofunc_close_ocb(resmgr_context_t *ctp, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
	register iofunc_mount_t			*mountp;
	register iofunc_funcs_t			*funcs;
	void							(*ocb_free)(iofunc_ocb_t *ocb);
	int								(*attr_lock)(iofunc_attr_t *attr);
	int								(*attr_unlock)(iofunc_attr_t *attr);

	if ((mountp = attr->mount) != NULL && (funcs = mountp->funcs) != NULL) {
		if (funcs->nfuncs < (offsetof(iofunc_funcs_t, attr_unlock) / sizeof(void *)) || (attr_lock = funcs->attr_lock) == NULL || (attr_unlock = funcs->attr_unlock) == NULL) {
			attr_lock = iofunc_attr_lock, attr_unlock = iofunc_attr_unlock;
		}
		if (funcs->nfuncs < (offsetof(iofunc_funcs_t, ocb_free) / sizeof(void *)) || (ocb_free = funcs->ocb_free) == NULL) {
			ocb_free = iofunc_ocb_free;
		}
	}
	else {
		attr_lock = iofunc_attr_lock, attr_unlock = iofunc_attr_unlock;
		ocb_free = iofunc_ocb_free;
	}

	(void)(*attr_lock)(attr);

	(void)iofunc_ocb_detach(ctp, ocb);
	(*ocb_free)(ocb);

	(void)(*attr_unlock)(attr);

	return EOK;
}

__SRCVERSION("iofunc_close_ocb.c $Rev: 153052 $");
