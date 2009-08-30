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
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <share.h>
#include <mqueue.h>
#include <sys/iofunc.h>

extern int __mq_check(int, mqd_t);

mqd_t _vmq_open(const char *name, int oflag, va_list ap) {
	mode_t			mode = 0;
	struct mq_attr	*attr = 0;

	if(oflag & O_CREAT) {
		mode = va_arg(ap, mode_t);
		attr = va_arg(ap, struct mq_attr *);
	}

	oflag |= O_CLOEXEC;
	return _connect_object(name, "/dev/mqueue", mode, oflag, _FTYPE_MQUEUE, _IO_CONNECT_EXTRA_MQUEUE, sizeof *attr, attr);
}

mqd_t mq_open(const char *name, int oflag, ...) {
	va_list			ap;
	mqd_t			ret;

	va_start(ap, oflag);
	ret = _vmq_open(name, oflag, ap);
	va_end(ap);
	return((ret == (mqd_t)-1) ? __mq_check(0, ret) : ret);
}

__SRCVERSION("mq_open.c $Rev: 153052 $");
