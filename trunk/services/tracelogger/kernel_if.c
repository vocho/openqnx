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
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/syspage.h>
#include <sys/trace.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/kercalls.h>
#include <time.h>
#include <unistd.h>

#define DECLARE_GLOBALS
#include "kernel_if.h"
#include "utils.h"

/* RUSH */
#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

#define KBUF_NAME "/ktrace.buffers"

static int create_shared_object(paddr_t paddr, size_t size) {
	int ret, fd;
	uint64_t lpaddr;

	lpaddr = (uint64_t)paddr;

	info("Create shared object paddr 0x%llx size %d\n", lpaddr, size);

	fd = shm_open(KBUF_NAME, O_CREAT | O_EXCL | O_RDWR, S_IRWXU);
	if(fd == -1) {
		debug("Failed creating shared object: %d\n", errno);
		return -1;
	}

	ret = shm_ctl(fd, SHMCTL_PHYS, lpaddr, size);
	if(ret == -1) {
		debug("Failed binding physical address to shared object\n");
	}

	close(fd);

	return ret;
}

static int clean_shared_object() {
	return shm_unlink(KBUF_NAME);
}

static int get_paddr_from_shmem(paddr_t *paddr) {
	off64_t lpaddr;
	void 	*ptr;
	struct stat st;
	int 	fd;

	fd = shm_open(KBUF_NAME, O_RDWR, S_IRWXU);
	if(fd == -1) {
		debug("Failed opening shared object %d\n", errno);
		return -1;
	}

	if(fstat(fd, &st) == -1) {
		debug("Failed stating shared object %d\n", errno);
		close(fd);
		return -1;
	}

	ptr = mmap(0, st.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED) {
		debug("Failed allocating mmap() %d\n", errno);
		close(fd);
		return -1;
	}

	if(mem_offset64(ptr, NOFD, st.st_size, &lpaddr, NULL) == -1) {
		debug("Failed to get memoffet() %d\n", errno);
		close(fd);
		munmap(ptr, st.st_size);
		return -1;
	}
	*paddr = (paddr_t)lpaddr;

	munmap(ptr, st.st_size);
	close(fd);

	return 0;
}

static int get_paddr_from_kernel(int nbufs, paddr_t *paddr) {
	TraceEvent(_NTO_TRACE_DEALLOCBUFFER );
	if (-1 == TraceEvent(_NTO_TRACE_ALLOCBUFFER, nbufs, paddr)) {
		return -1;
	}
	return 0;
}

static int attach_to_paddr(int nbufs, paddr_t paddr) {
	unsigned vaddr, cache_mask, num_colours;

	vaddr = 0;
	num_colours = discover_num_colours();
	cache_mask = (num_colours - 1) * PAGESIZE;

	num_kernel_buffers = nbufs;

#if defined(__ARM__)
	kernel_buffers_vaddr = (tracebuf_t *)paddr;
#else
	/* RUSH - Future proof this stuff */
	if ( num_colours > 0 ) {
		/* MIPS and SH have cache's with colour, so we need to be sensitive to
		 * the colour of the physical address the kernel allocated, and make sure
		 * that our virtual address matches that.
		 */
		/* allocate some vaddr (more than we need (by num_colours * PAGESIZE) */
		vaddr = (unsigned)mmap(
			0, (nbufs * _TRACEBUFSIZE) + (num_colours * PAGESIZE),
			PROT_READ|PROT_WRITE,
			MAP_PRIVATE|MAP_LAZY|MAP_ANON,
			NOFD,
			0
		);
		if(vaddr == (unsigned)MAP_FAILED) {
			perror("allocating virtual address range for mapping");
			return -1;
		}
		/* now unmap it */
		if ( munmap( (void *)vaddr, (nbufs * _TRACEBUFSIZE) + (num_colours * PAGESIZE) ) == -1 ) {
			perror("unmapping virtual address range");
			return -1;
		}
		/* make sure the colour matches the colour of the physical address */
		vaddr += ((paddr & cache_mask) - (vaddr & cache_mask)) & cache_mask;
	}
	kernel_buffers_vaddr = mmap(
		(void *)vaddr, nbufs * _TRACEBUFSIZE,
		PROT_READ|PROT_WRITE,
		vaddr == 0 ?  /* only use fixed mapping if we are worried about colours */
			(MAP_SHARED|MAP_PHYS):(MAP_FIXED|MAP_SHARED|MAP_PHYS),
		NOFD,
		paddr
	);
	if(kernel_buffers_vaddr == MAP_FAILED) {
		return -1;
	}
	if ( num_colours > 0 ) {
		if ( (((unsigned)kernel_buffers_vaddr) & cache_mask) != (((unsigned)paddr) & cache_mask) ) {
			fprintf(stderr, "couldn't allocate kernel buffer memory with correct cache attributes\n");
			errno = ENOMEM;
			return -1;
		}
	}
	debug("kernel_buffers_vaddr = %#lx\n", kernel_buffers_vaddr );
#endif

	return 0;
}

int kernel_attach( int nbufs, int reuse_buffers, paddr_t *kernel_buffers_paddr )
{
	int 	ret = -1;

	if(reuse_buffers) { /* PDB option -R after previous tracelogger with option -P */
		ret = get_paddr_from_shmem(kernel_buffers_paddr);
	}

	//We aren't using shared memory, or it wasn't there, try the kernel
	if(ret == -1) { /* PDB otherwise */
		ret = get_paddr_from_kernel(nbufs, kernel_buffers_paddr);
	}

	if(ret == -1) {
		return -1;
	}

	info("Seeking %d buffers at phys addr 0x%llx (%s)\n",
			nbufs, (uint64_t)*kernel_buffers_paddr, (reuse_buffers) ? "shmem" : "kernel");

	ret = attach_to_paddr(nbufs, *kernel_buffers_paddr);
	if(ret == -1) {
		return -1;
	}

	return 0;
}

int kernel_detach( paddr_t kernel_buffers_paddr, int persist_buffers )
{
	int ret = -1;

	//Always wipe out the shared object ... historical or not.
	clean_shared_object();

	if(persist_buffers) {
		ret = create_shared_object( kernel_buffers_paddr, num_kernel_buffers * _TRACEBUFSIZE );
	}

	if(ret == -1) {
		TraceEvent( _NTO_TRACE_DEALLOCBUFFER );
		debug("De-allocated kernel buffers\n");
	}

	munmap( kernel_buffers_vaddr, num_kernel_buffers * _TRACEBUFSIZE );
	debug("Unmapped local event buffers\n");

	return 0;
}

__SRCVERSION("kernel_if.c $Rev: 157117 $");
