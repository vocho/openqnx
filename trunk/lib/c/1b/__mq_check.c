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




#include <errno.h>
#include <mqueue.h>

int __mq_check(int opened, mqd_t mq) {
	if (opened) {
		/* mqd-level failed: mq_getattr, mq_send, mq_receive, mq_notify */
		/* message was a DCMD or an XTYPE, non-mqueue should ENOSYS it  */
		/* could do fstat(mq)/S_TYPEISMQ() it to be really sure (slow)  */
		if (errno == ENOSYS)
			errno = EBADF;
	}
	else if (mq == (mqd_t)-1) {
		/* connection-level failed: mq_open, mq_unlink      */
		/* message was an FTYPE_MQUEUE, pathmgr should fail */
		/* deduce the absence of a system mqueue server     */
		if (errno == ENOENT)
			errno = (sysconf(_SC_MQ_OPEN_MAX) == -1) ? ENOSYS : ENOENT;
	}
	return(-1);
}

__SRCVERSION("__mq_check.c $Rev: 153052 $");
