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

int mq_setattr(mqd_t mq, const struct mq_attr *attr, struct mq_attr *oattr) {
	struct mq_attr saveattr;

	if(oattr  &&  mq_getattr(mq, &saveattr) == -1) {
		return -1;
	}

	if(_devctl(mq, DCMD_MISC_MQSETATTR, (void *)attr, sizeof *attr, _DEVCTL_FLAG_NORETVAL) == -1) {
		return(__mq_check(!0, mq));
	}

	if(oattr) {
		*oattr = saveattr;
	}

	return 0;
}

__SRCVERSION("mq_setattr.c $Rev: 153052 $");
