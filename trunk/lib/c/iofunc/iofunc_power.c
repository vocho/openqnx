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
#include <sys/iofunc.h>
#include <resmgr.h>

int iofunc_power(resmgr_context_t *ctp, io_power_t *msg, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
	int				status;
	iofunc_mount_t	*mountp = attr->mount;
	const resmgr_io_funcs_t	*funcs;
	int	(*lock)(resmgr_context_t *, resmgr_iomsgs_t *, void *);
	int	(*unlock)(resmgr_context_t *, resmgr_iomsgs_t *, void *);

	if (mountp == 0 || mountp->power == 0) {
		return ENOSYS;
	}

	funcs = _resmgr_iofuncs(ctp, &ctp->info);

	/*
	 * Unlock the ocb because pmd_power sends messages to Power Manager
	 */
	if ((unlock = _resmgr_io_func(funcs, _IO_RSVD_UNLOCK_OCB))) {
		(void)unlock(ctp, 0, ocb);
	}
	
	status = pmd_power(ctp, msg, mountp->power);

	if ((lock = _resmgr_io_func(funcs, _IO_RSVD_LOCK_OCB))) {
		(void)lock(ctp, 0, ocb);
	}
	return status;
}

__SRCVERSION("iofunc_power.c $Rev: 153052 $");
