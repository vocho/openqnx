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




#undef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS	64

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/resmgr.h>
#include "resmgr.h"

int resmgr_devino(int id, dev_t *pdevno, ino64_t *pino) {
	struct link				*p;
	struct stat				buff;

	if(!(p = _resmgr_link_query(id, 1)) || fstat(p->link_id, &buff) == -1) {
		if (p) {
			_resmgr_link_return(p, 1);
		}
		errno = EINVAL;
		return -1;
	}
	_resmgr_link_return(p, 1);
	*pdevno = buff.st_dev;
	*pino = buff.st_ino;
	return 0;
}

__SRCVERSION("resmgr_devino.c $Rev: 153052 $");
