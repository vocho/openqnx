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




#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/iofunc.h>
#include <sys/dcmd_all.h>

int iofunc_devctl_default(resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb) {
	return iofunc_devctl(ctp, msg, ocb, ocb->attr);
}

__SRCVERSION("iofunc_devctl_default.c $Rev: 153052 $");
