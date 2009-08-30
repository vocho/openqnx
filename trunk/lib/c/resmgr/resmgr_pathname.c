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
#include <errno.h>
#include <sys/stat.h>
#include <sys/iomgr.h>
#include <sys/resmgr.h>
#include "resmgr.h"

int resmgr_pathname(int id, unsigned flags, char *path, int maxbuf) {
	struct link				*p;
	int						len;

	if(!(p = _resmgr_link_query(id, 1)) || (len = iofdinfo(p->link_id, flags, 0, path, maxbuf)) == -1) {
		errno = EINVAL;
		len = -1;
	}
	if (p) {
		_resmgr_link_return(p, 1);
	}
	return len;
}

__SRCVERSION("resmgr_pathname.c $Rev: 153052 $");
