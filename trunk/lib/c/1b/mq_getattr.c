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
#include <devctl.h>
#include <sys/dcmd_misc.h>

extern int	__mq_check(int, mqd_t);

int mq_getattr(mqd_t mq, struct mq_attr *attr) {
	int	rc;

	rc = _devctl(mq, DCMD_MISC_MQGETATTR, attr, sizeof *attr, _DEVCTL_FLAG_NORETVAL);
	return((rc == -1) ? __mq_check(!0, mq) : 0);
}

__SRCVERSION("mq_getattr.c $Rev: 153052 $");
