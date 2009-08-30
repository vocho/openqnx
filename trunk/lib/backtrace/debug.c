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

#ifndef _BT_LIGHT

#include "common.h"

#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include "debug.h"

extern int vsnprintf(char *str, size_t size, const char *fmt, va_list);
void
_bt_trace (char *fmt, ...)
{
	char m[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(m, sizeof(m), fmt, ap);
	va_end(ap);
	write(1, m, strlen(m));
}

#endif
