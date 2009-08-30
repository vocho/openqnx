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




#if (defined(__GNUC__) || defined(__INTEL_COMPILER)) && !defined(__MIPS__)
void resmgr_start() __attribute__ ((weak, alias("_resmgr_start")));
#endif
#define RESMGR_COMPAT
#include <stdlib.h>
#include <pthread.h>
#include <sys/resmgr.h>
#include "resmgr.h"

#ifndef PATH_MAX
#define PATH_MAX	1024 	/* Should use fpathconf(path, _PC_PATH_MAX) */
#endif
#define MSG_MAX_SIZE	(sizeof(struct _io_connect_link_reply) + sizeof(struct _io_connect_entry) * SYMLOOP_MAX + PATH_MAX + 1)

int resmgr_start(resmgr_ctrl_t *ctrl) {
	ctrl->msg_max_size = max(ctrl->msg_max_size, max(MSG_MAX_SIZE, sizeof(resmgr_iomsgs_t)));
	ctrl->nparts_max = max(ctrl->nparts_max, 1);
	ctrl->maximum = max(ctrl->maximum, 1);
	ctrl->hi_water = max(ctrl->hi_water, 1);
	ctrl->created = 1;
	ctrl->waiting = 1;
	pthread_mutex_init(&ctrl->mutex, 0);
	(void)_resmgr_thread(ctrl);
	return -1;
}

__SRCVERSION("resmgr_start.c $Rev: 153052 $");
