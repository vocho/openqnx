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




#include <inttypes.h>
#include <errno.h>
#include <sys/memmsg.h>


static int 
_mem_offset64(const void *addr, size_t len, off64_t *off, size_t *contig_len, int *fd, int subtype) {
	mem_offset_t					msg;

	msg.i.type = _MEM_OFFSET;
	msg.i.subtype = subtype;
	msg.i.addr = (uintptr_t)addr;
	msg.i.reserved = -1;
	msg.i.len = len;
	if(MsgSendnc(MEMMGR_COID, &msg.i, sizeof msg.i, &msg.o, sizeof msg.o) == -1) {
		return -1;
	}
	if(off) {
		*off = msg.o.offset;
	}
	if(contig_len) {
		*contig_len = msg.o.size;
	}
	if(fd) {
		*fd = msg.o.fd;
	}
	return 0;
}


int 
posix_mem_offset64(const void *addr, size_t len, off64_t *off, size_t *contig_len, int *fd) {
	return _mem_offset64(addr, len, off, contig_len, fd, _MEM_OFFSET_FD);
}


int 
posix_mem_offset(const void *addr, size_t len, off_t *off, size_t *contig_len, int *fd) {
	int					ret;
	off64_t				offset;

	ret = posix_mem_offset64(addr, len, &offset, contig_len, fd);
	if(off) {
		if(offset > INT_MAX) {
			errno = EOVERFLOW;
			return -1;
		}
		*off = offset;
	}
	return ret;
}


/* This was 1003.1j D5 and posix_mem_offset should be used instead */

int 
(mem_offset64)(const void *addr, int fd, size_t len, off64_t *off, size_t *contig_len) {
	if(fd != NOFD) {
		/* 2.00 only supported NOFD */
		errno = ENODEV;
		return -1;
	}
	return _mem_offset64(addr, len, off, contig_len, 0, _MEM_OFFSET_PHYS);
}


int 
(mem_offset)(const void *addr, int fd, size_t len, off_t *off, size_t *contig_len) {
	int					ret;
	off64_t				offset;

	if(fd != NOFD) {
		/* 2.00 only supported NOFD */
		errno = ENODEV;
		return -1;
	}
	ret = _mem_offset64(addr, len, &offset, contig_len, 0, _MEM_OFFSET_PHYS);
	if(off) {
		if(offset > INT_MAX) {
			errno = EOVERFLOW;
			return -1;
		}
		*off = offset;
	}
	return ret;
}

__SRCVERSION("mem_offset.c $Rev: 153052 $");
