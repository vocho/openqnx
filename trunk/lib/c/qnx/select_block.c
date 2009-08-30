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
#include <pthread.h>
#include <sys/iomsg.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int _select_block(const struct timespec *ts, union sigval *value, void *arg) 
{
	sigset_t	set;
	siginfo_t	info;

	arg= arg;
	sigemptyset(&set);
	sigaddset(&set, SIGSELECT);
	if (sigtimedwait(&set, &info, ts) == -1)
	  return -1;
	*value= info.si_value; // I always pass value
	return 0;
}

__SRCVERSION("select_block.c $Rev: 153052 $");
