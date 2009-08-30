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
#include <errno.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <sys/memmsg.h>


void *
_mmap2(void *addr, size_t len, int prot, int flags, int fd, off64_t off, 
		unsigned align, unsigned preload, void **base, size_t *size) {
	mem_map_t						msg;

	msg.i.type = _MEM_MAP;
	msg.i.zero = 0;
	msg.i.addr = (uintptr_t)addr;
	msg.i.len = len;
	msg.i.prot = prot;
	msg.i.flags = flags;
	msg.i.fd = fd;
	msg.i.offset = off;
	msg.i.align = align;
	msg.i.preload = preload;
	msg.i.reserved1 = 0;
	if(MsgSendnc(MEMMGR_COID, &msg.i, sizeof msg.i, &msg.o, sizeof msg.o) == -1) {
		return MAP_FAILED;
	}
	if(base) {
		*base = (void *)(uintptr_t)msg.o.real_addr;
	}
	if(size) {
		*size = msg.o.real_size;
	}
	return (void *)(uintptr_t)msg.o.addr;
}


void *
mmap64(void *addr, size_t len, int prot, int flags, int fd, off64_t off) {
	return _mmap2(addr, len, prot, flags, fd, off, 0, 0, 0, 0);
}


// Make an unsigned version of the 'off_t' type so that we get a zero
// extension down below.
#if __OFF_BITS__ == 32
	typedef _Uint32t uoff_t;
#elif __OFF_BITS__ == 64
	typedef _Uint64t uoff_t;
#else
	#error Do not know what size to make uoff_t
#endif

void *
mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off) {
	return _mmap2(addr, len, prot, flags, fd, (uoff_t)off, 0, 0, 0, 0);
}


// Nobody should be using this function, so we can think about removing
// in a while.
void *
_mmap(void *addr, size_t len, int prot, int flags, int fd, off64_t off, 
		unsigned align, void **base, size_t *size) {
	return _mmap2(addr, len, prot, flags, fd, off, align, 0, base, size);
}

__SRCVERSION("mmap.c $Rev: 153052 $");
