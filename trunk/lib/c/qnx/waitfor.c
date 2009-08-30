/*
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
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
 *
 */

#include <inttypes.h>
#include <libgen.h>
#include <stddef.h>
#include <sys/stat.h>

static int check_stat( __const char *path, void *handle )
{
	struct stat64 sbuf;

	return stat64( path, &sbuf );
}

int waitfor( __const char *path, int delay_ms, int poll_ms )
{
	return _waitfor(path, delay_ms, poll_ms, check_stat, NULL);
}
