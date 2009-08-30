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
#include <sys/mman.h>

uintptr_t mmap_device_io(size_t len, uint64_t io) {
#if defined(__X86__)
	return io;
#else
	return (uintptr_t)mmap64(0, len, PROT_NOCACHE|PROT_READ|PROT_WRITE, MAP_SHARED|MAP_PHYS, NOFD, io);
#endif
}

__SRCVERSION("mmap_device_io.c $Rev: 153052 $");
