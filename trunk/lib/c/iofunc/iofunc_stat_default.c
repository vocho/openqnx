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
#include <unistd.h>
#include <sys/iofunc.h>

int iofunc_stat_default(resmgr_context_t *ctp, io_stat_t *msg, iofunc_ocb_t *ocb) {
	int		status;

	(void)iofunc_time_update(ocb->attr);
	if ((status = iofunc_stat(ctp, ocb->attr, &msg->o)) != EOK)
		return(status);
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

__SRCVERSION("iofunc_stat_default.c $Rev: 153052 $");
