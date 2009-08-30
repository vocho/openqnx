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
#include <string.h>
#include <sys/iofunc.h>

typedef int(*_resmgr_connect_func_t)(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, void *handle, void *extra);

static const resmgr_connect_funcs_t connect_funcs = {
	_RESMGR_CONNECT_NFUNCS,
	iofunc_open_default,
	0,		// io_unlink_t
	0,		// io_rename_t
	0,		// io_mknod_t
	0,		// io_readlink_t
	0,		// io_link_t
	0		// io_pulse_t (unblock)
};

static const resmgr_io_funcs_t io_funcs = {
	_RESMGR_IO_NFUNCS,
	iofunc_read_default,
	iofunc_write_default,
	iofunc_close_ocb_default,
	iofunc_stat_default,
	0,		// io_notify_t
	iofunc_devctl_default,
	iofunc_unblock_default,
	iofunc_pathconf_default,
	iofunc_lseek_default,
	iofunc_chmod_default,
	iofunc_chown_default,
	iofunc_utime_default,
	iofunc_openfd_default,
	iofunc_fdinfo_default,
	iofunc_lock_default,
	0,		// io_space_t
	0,		// io_shutdown_t
	iofunc_mmap_default,
	0,		// io_msg_t
	0,		// reserved
	0,		// io_dup_t
	iofunc_close_dup_default,
	iofunc_lock_ocb_default,
	iofunc_unlock_ocb_default,
	iofunc_sync_default,
	iofunc_power_default
};

void iofunc_func_init(unsigned nconnect, resmgr_connect_funcs_t *connect, unsigned nio, resmgr_io_funcs_t *io) {
	unsigned			*pnum;

	if(nconnect && connect) {
		pnum = &connect->nfuncs;
		*pnum = min(nconnect, _RESMGR_CONNECT_NFUNCS);
		memcpy(&connect->open, &connect_funcs.open, *pnum * sizeof(_resmgr_connect_func_t));
		if(nconnect > _RESMGR_CONNECT_NFUNCS) {
			memset(connect + 1, 0x00, (nconnect - _RESMGR_CONNECT_NFUNCS) * sizeof(_resmgr_connect_func_t));
		}
	}
	if(nio && io) {
		pnum = &io->nfuncs;
		*pnum = min(nio, _RESMGR_IO_NFUNCS);
		memcpy(&io->read, &io_funcs.read, *pnum * sizeof(_resmgr_func_t));
		if(nio > _RESMGR_IO_NFUNCS) {
			memset(io + 1, 0x00, (nio - _RESMGR_IO_NFUNCS) * sizeof(_resmgr_func_t));
		}
	}
}


__SRCVERSION("iofunc_func_init.c $Rev: 153052 $");
