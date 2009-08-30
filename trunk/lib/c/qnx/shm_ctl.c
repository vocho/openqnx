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




#include <sys/mman.h>
#include <devctl.h>
#include <ioctl.h>
#include <sys/dcmd_proc.h>
#include <inttypes.h>
#include <errno.h>


static int
do_ctl(int fd, int flags, uint64_t paddr, uint64_t size, unsigned special) {
	int		status;

	struct _dcmd_memmgr_memobj memobj;

	memset(&memobj, 0, sizeof(memobj));

	memobj.offset = paddr;
	memobj.size = size;
	memobj.flags = flags;
	memobj.special = special;

	if((status = _devctl(fd, DCMD_MEMMGR_MEMOBJ, &memobj, sizeof(memobj), _DEVCTL_FLAG_NOTTY)) == -1) {
		errno = ENOSYS;
	}

	return status;
}


int
shm_ctl_special(int fd, int flags, uint64_t paddr, uint64_t size, unsigned special) {
	return do_ctl(fd, flags|SHMCTL_HAS_SPECIAL, paddr, size, special);
}


int
shm_ctl(int fd, int flags, uint64_t paddr, uint64_t size) {
	return do_ctl(fd, flags & ~SHMCTL_HAS_SPECIAL, paddr, size, 0);
}

__SRCVERSION("shm_ctl.c $Rev: 153052 $");
