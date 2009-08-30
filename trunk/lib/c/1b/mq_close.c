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
#include <mqueue.h>
#include <sys/iomsg.h>
#include <sys/stat.h>

int
mq_close(mqd_t mq) {
	struct stat	st;

	if(fstat(mq, &st) != 0) {
		return -1;
	}
	if(!S_TYPEISMQ(&st)) {
		errno = EBADF;
		return -1;
	}

	return close(mq);
}

__SRCVERSION("mq_close.c $Rev: 153052 $");
