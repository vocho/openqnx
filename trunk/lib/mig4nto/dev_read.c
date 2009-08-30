/*
 * $QNXLicenseC:
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
 * $
 */

/*
 * dev_read.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <errno.h>
#include <unistd.h>
#include <mig4nto.h>

/*
 *  dev_read
 *
 *  Note that the proxy and armed parameters are not currently
 *  supported.
 */
int
dev_read(int fd, void *buf, unsigned n, unsigned min, unsigned time,
			unsigned timeout, pid_t proxy, int *armed)
{
	if (proxy || armed) {
		errno = ENOSYS;
		return -1;
	}
	
	return readcond(fd, buf, n, min, time, timeout);
}
