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
 * disk_space.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <sys/statvfs.h>
#include <mig4nto.h>

int disk_space(int fd, long *freeblks, long *totalblks)
{
	struct statvfs	stfs;
	long			scale;

	if (fstatvfs(fd, &stfs) == -1)
		return(-1);
	scale = stfs.f_bsize / 512;
	*freeblks = stfs.f_bfree * scale;
	*totalblks = stfs.f_blocks * scale;
	return(0);
}
