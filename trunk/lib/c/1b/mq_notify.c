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

int mq_notify(mqd_t mq, const struct sigevent *event) {
	int rc;
	struct sigevent	tmp;

	if(event && SIGEV_GET_TYPE(event) == SIGEV_SIGNAL) {
		tmp = *event;
		tmp.sigev_notify = SIGEV_SIGNAL_CODE;
		tmp.sigev_code = SI_MESGQ;
		event = &tmp;
	}

	if (((rc = ionotify(mq, _NOTIFY_ACTION_TRANARM,
		_NOTIFY_COND_INPUT, event)) == -1) && (errno == EAGAIN)) {
		rc = 0;
	}

	return((rc == -1) ? __mq_check(!0, mq) : 0);
}

__SRCVERSION("mq_notify.c $Rev: 168079 $");
