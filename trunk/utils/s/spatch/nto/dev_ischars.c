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




#include <sys/types.h>
#include <unistd.h>
#include <devctl.h>
#include <sys/dcmd_chr.h>

int dev_ischars(int fd)
{
	unsigned nchars;

	/* for a character device, will return chars waiting to be read;
       a way to ensure that data is available without using O_NONBLK */
	if (-1!=devctl(fd, DCMD_CHR_ISCHARS, NULL, 0, &nchars)) return nchars;
	return 0; /* pretend that nothing is available on error */
}
