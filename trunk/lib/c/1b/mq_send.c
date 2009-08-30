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

extern int	__mq_check(int, mqd_t);

int mq_send(mqd_t mq, const char *buff, size_t nbytes, unsigned msgprio) {
	if (_writex(mq, buff, nbytes, _IO_XTYPE_MQUEUE | (msgprio << 16), NULL, 0) == -1) {
		return(__mq_check(!0, mq));
	}
	return(0);
}

__SRCVERSION("mq_send.c $Rev: 153052 $");
