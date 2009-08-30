/*
 * $QNXLicenseC:
 * Copyright 2007,2008, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable
 * license fees to QNX Software Systems before you may reproduce,
 * modify or distribute this software, or any work that includes
 * all or part of this software.   Free development licenses are
 * available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email
 * licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review
 * this entire file for other proprietary rights or license notices,
 * as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include "common.h"

#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include "backtrace.h"
#include "memmap.h"

bt_accessor_t BT(acc_self) = {.type=BT_SELF, .flags=0, .pid=0, .tid=0,
							  .stack_limit=0};

#ifndef _BT_LIGHT

int
bt_init_accessor (bt_accessor_t *acc, bt_acc_type_t type, ...)
{
	va_list ap;

	if (acc==0) {
		errno=EINVAL;
		return -1;
	}

	acc->type = type;
	acc->flags = BTF_DEFAULT;
	acc->stack_limit = 0;

	va_start(ap, type);
	switch (type) {
		case BT_SELF:
			acc->pid=0;
			acc->tid=0;         /* don't care */
			break;
		case BT_THREAD:
			acc->pid=getpid();
			acc->tid=va_arg(ap,pthread_t);
			break;
		case BT_PROCESS:
			acc->pid=va_arg(ap,int);
			acc->tid=va_arg(ap,pthread_t);
			break;
		default:
			errno=EINVAL;
			return -1;
			break;
	}
	va_end(ap);

	return 0;
}


int
bt_release_accessor (bt_accessor_t *acc)
{
	if (acc==0) {
		errno=EINVAL;
		return -1;
	}
	return 0;
}

int
bt_set_flags (bt_accessor_t *acc, unsigned flags, int onoff)
{
	if (acc==0) {
		errno=EINVAL;
		return -1;
	}
	/* If any bit other than those currently supported are specified
	 * in flags, then it's an error... */
	if (flags&~(BTF_LIVE_BACKTRACE|BTF_DEFAULT)) {
		errno=EINVAL;
		return -1;
	}
	if (onoff) {
		acc->flags |= flags;
	} else {
		acc->flags &= ~flags;
	}

	return 0;
}

#endif
