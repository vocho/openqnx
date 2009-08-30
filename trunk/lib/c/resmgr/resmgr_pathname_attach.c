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
void resmgr_pathname_attach() __attribute__ ((weak, alias("_resmgr_pathname_attach")));
#endif
#include <unistd.h>
#include <errno.h>
#define RESMGR_COMPAT
#include <sys/resmgr.h>
#include <sys/pathmgr.h>
#include "resmgr.h"

int resmgr_pathname_attach(const char *path, int chid, enum _file_type file_type, unsigned flags,
		const resmgr_connect_funcs_t *connect_funcs, const resmgr_io_funcs_t *io_funcs, void *handle) {
	struct link							*linkp;

	if(!(linkp = _resmgr_link_alloc())) {
		errno = ENOMEM;
		return -1;
	}
	linkp->connect_funcs = connect_funcs;
	linkp->io_funcs = io_funcs;
	linkp->handle = handle;
	if(chid == -1) {
		linkp->link_id = -1;
	} else {
		if((linkp->link_id = pathmgr_link(path, 0, 0, chid, linkp->id, file_type, flags & _RESMGR_FLAG_MASK)) == -1) {
			_resmgr_link_free(linkp->id, _RESMGR_DETACH_ALL);
			return -1;
		}
	}
	linkp->flags &= ~_RESMGR_LINK_HALFOPEN;
	return linkp->id;
}

__SRCVERSION("resmgr_pathname_attach.c $Rev: 153052 $");
