/*
 * $QNXLicenseC: 
 * Copyright 2007, 2008, QNX Software Systems.  
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */





#include "rtc.h"
#include <fcntl.h>
#include <sys/mman.h>

#if defined (__386__)
/* 32 bit versions */
void *
mmap_device_memory(void *__addr, size_t __len, int __prot, int __flags, unsigned __physical) {
	static int	fd = -1;
	void		*addr;
	
	if (fd == -1) {
		if ((fd = shm_open ("Physical", O_RDWR, 0777)) == -1) {
			fprintf(stderr, "Unable to open physical memory (%s)", strerror (errno));
			exit(-1);
		}
	}
	addr = mmap (0, 8196, PROT_READ|PROT_WRITE|PROT_NOCACHE, MAP_SHARED, fd, __physical);
	return addr;
}

#else
/* 16 bit version */

#error 16 bit not supported anymore

#if 0
#include <sys/seginfo.h>
static unsigned sel;

void far *make_clk_pntr(unsigned base) {
	void far *addr;
	
	if((sel = qnx_segment_overlay(base, 8196)) == -1) {
		fprintf(stderr, "Unable get overlay segment(%s)", strerror (errno));
		exit(-1);
	}

	addr = MK_FP(sel, 0);
	return addr;
}

void free_clk_pntr (void *phys) {
	phys = phys;
	qnx_segment_free(sel);
}
#endif
#endif
