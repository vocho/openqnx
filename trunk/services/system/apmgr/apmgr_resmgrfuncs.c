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

/*==============================================================================
 * 
 * apmgr_resmgrfuncs
 * 
 * Resource manager miscellaneous connect and I/O functions not warranting their
 * own file
 * 
*/

#include "apmgr.h"


/*******************************************************************************
 * apmgr_chmod
*/
int apmgr_chmod(resmgr_context_t *ctp, io_chmod_t *msg, void *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	return iofunc_chmod_default(ctp, msg, ocb);
}

/*******************************************************************************
 * apmgr_chown
*/
int apmgr_chown(resmgr_context_t *ctp, io_chown_t *msg, void *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	return iofunc_chown_default(ctp, msg, ocb);
}

/*******************************************************************************
 * apmgr_lseek
 * 
 * This function provided primarily to support rewinddir()
*/
 #define IS32BIT(_attr, _ioflag) ((!((_ioflag) & O_LARGEFILE)) || ((_attr)->mount != NULL && (_attr)->mount->flags & IOFUNC_MOUNT_32BIT))
int apmgr_lseek(resmgr_context_t *ctp, io_lseek_t *msg, void *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	return iofunc_lseek_default(ctp, msg, ocb);
}

/*******************************************************************************
 * apmgr_unblock
 * 
*/
int apmgr_unblock(resmgr_context_t *ctp, io_pulse_t *msg, void *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	return iofunc_unblock_default(ctp, msg, ocb);
}

/*******************************************************************************
 * apmgr_openfd
 * 
*/
int apmgr_openfd(resmgr_context_t *ctp, io_openfd_t *msg, void *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	return iofunc_openfd_default(ctp, msg, ocb);
}

/*******************************************************************************
 * apmgr_fdinfo
 * 
*/
int apmgr_fdinfo(resmgr_context_t *ctp, io_fdinfo_t *msg, void *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	return iofunc_fdinfo_default(ctp, msg, ocb);
}

/*******************************************************************************
 * apmgr_utime
 * 
*/
int apmgr_utime(resmgr_context_t *ctp, io_utime_t *msg, void *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	return iofunc_utime_default(ctp, msg, ocb);
}

__SRCVERSION("$IQ: apmgr_resmgrfuncs.c,v 1.23 $");

