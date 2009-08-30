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





#include "externs.h"


struct ocb *ocb_calloc(resmgr_context_t *ctp, MQDEV *attr) {
	return MemchunkCalloc(memchunk, 1, sizeof(struct ocb));
}

void ocb_free(struct ocb *ocb) {
	MemchunkFree(memchunk, ocb);
}

void func_init(void) {
	memset(&ocb_funcs, 0, sizeof ocb_funcs);
	ocb_funcs.ocb_calloc = ocb_calloc;
	ocb_funcs.ocb_free = ocb_free;
	ocb_funcs.nfuncs = _IOFUNC_NFUNCS;

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &mq_connect_funcs, _RESMGR_IO_NFUNCS, &mq_io_funcs);
	mq_connect_funcs.open = io_open;
	mq_connect_funcs.unlink = io_unlink;

	mq_io_funcs.read = io_read;
	mq_io_funcs.write = io_write;
	mq_io_funcs.close_dup = io_closedup;
	mq_io_funcs.close_ocb = io_closeocb;
	mq_io_funcs.notify = io_notify;
	mq_io_funcs.devctl = io_devctl;
	mq_io_funcs.unblock = io_unblock;

#ifdef MQUEUE_1_THREAD
	mq_io_funcs.lock_ocb = NULL;
	mq_io_funcs.unlock_ocb = NULL;
#endif

	iofunc_func_init(0, 0, _RESMGR_IO_NFUNCS, &mq_io_dir_funcs);
}

__SRCVERSION("io_func_tables.c $Rev: 153052 $");
