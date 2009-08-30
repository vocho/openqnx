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


/*
 *  No-ops to link against for situations when procnto does not
 *  support, or need to support, a real implementation of a routine.
 */
#include <errno.h>
#include <sys/iofunc.h>

#ifndef VARIANT_gcov
/*
 *  procnto does not use stdio, yet pthread_exit() wants to release
 *  all stdio mutexes as part of thread cleanup; since we never use
 *  stdio, it will not be locked, and these may be empty stubs here.
 */
void _Unlocksysmtx(void)
{
}
void _Unlockfilemtx(void)
{
}
#endif

/*
 *  procnto has partial implementations of filesystems, but none are
 *  either as simplistic or full-featured as set up by the default
 *  iofunc_func_init(); so we may provide some empty stubs here (for
 *  details on mmap handling, look at "memmgr/fs_check[]" processing;
 *  if POSIX file locking not supported, no need for the default
 *  close/unblock handlers which just maintain the lock_list).
 */
int iofunc_open_default(resmgr_context_t *ctp, io_open_t *msg, iofunc_attr_t *attr, void *extra)
{ 
	return ENOSYS;
}
int iofunc_unblock_default(resmgr_context_t *ctp, io_pulse_t *msg, iofunc_ocb_t *ocb)
{ 
	return _RESMGR_DEFAULT;
}
int iofunc_lock_default(resmgr_context_t *ctp, io_lock_t *msg, iofunc_ocb_t *ocb)
{
	return ENOSYS;
}
int iofunc_mmap_default(resmgr_context_t *ctp, io_mmap_t *msg, iofunc_ocb_t *ocb)
{
	return ENOSYS;
}
int iofunc_close_dup_default(resmgr_context_t *ctp, io_close_t *msg, iofunc_ocb_t *ocb)
{
	return EOK;
}
int iofunc_sync_default(resmgr_context_t *ctp, io_sync_t *msg, iofunc_ocb_t *ocb)
{
	return ENOSYS;
}
int iofunc_power_default(resmgr_context_t *ctp, io_power_t *msg, iofunc_ocb_t *ocb)
{
	return ENOSYS;
}

__SRCVERSION("link_noops.c $Rev: 161179 $");
