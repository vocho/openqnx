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




#undef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS	32
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/iomsg.h>

int posix_fallocate64(int fd, off64_t offset, off64_t len) {
	io_space_t					msg;

	msg.i.type = _IO_SPACE;
	msg.i.combine_len = sizeof msg.i;
	msg.i.subtype = F_ALLOCSP64;
	msg.i.whence = SEEK_SET;
	msg.i.start = offset;
	msg.i.len = len;

	if(MsgSend(fd, &msg.i, sizeof msg.i, 0, 0) == -1) {
		return -1;
	}

	return 0;
}

int posix_fallocate(int fd, off_t offset, off_t len) {
	return posix_fallocate64(fd, offset, len);
}

/***********************************************************
 THIS FUNCTION IS ONLY FOR COMPATIBILITY WITH THE ORIGINAL
 POSIX_FALLOCATE FUNCTION. POSIX CHANGED THE PROTOTYPE IN AN
 INCOMPATIBLE WAY REQUIRING A DIFFERENT LINKAGE FOR THE FUNCTION.
 IT SHOULD BE REMOVED WHEN THE LIBC VERSION MOVES PAST 2.
 ***********************************************************/
#if (_LIBC_SO_VERSION == 2 || _LIBC_SO_VERSION == 3)
#ifndef posix_fallocate64
#error "fcntl.h" requires posix_fallocate64 to be defined for compatibility
#endif
static __inline int _fallocate64(int fd, off64_t offset, off64_t len) {
	return posix_fallocate64(fd, offset, len);
}
#undef posix_fallocate64
/* Original posix prototype had a size_t for length, later it was changed to an off_t */
int posix_fallocate64(int fd, off64_t offset, size_t len) {
	return _fallocate64(fd, offset, len);
}

#elif defined(posix_fallocate64)
#error "fcntl.h" must be updated to remove the #define for posix_fallocate64
#endif

__SRCVERSION("posix_fallocate.c $Rev: 171092 $");
