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
#include <sys/memmsg.h>

/* This was 1003.1j D5 and should be removed */

typedef _uint64         access_addr_t;
struct access_info {
    access_addr_t       access_addr;
    size_t              access_size;
    _uint32             access_flags;
};

int mem_access_clear(int master_fd, struct access_info *info) {
	errno = ENOSYS;
	return -1;
}

__SRCVERSION("mem_access_clear.c $Rev: 153052 $");
