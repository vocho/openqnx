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
#include <sys/mman.h>
#include <sys/iofunc.h>

int shm_open(const char *name, int oflag, mode_t mode) {
	oflag |= O_CLOEXEC;
	return _connect_object(name, "/dev/shmem", mode, oflag, _FTYPE_SHMEM, 0, 0, 0);
}

__SRCVERSION("shm_open.c $Rev: 153052 $");
