/*
 * $QNXLicenseC:
 * Copyright 2007,2008, QNX Software Systems. All Rights Reserved.
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

#ifndef _BT_LIGHT

#include "common.h"

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <sys/elf.h>
#ifndef _BT_LIGHT
#include <sys/procfs.h>
#include <sys/mman.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "backtrace.h"
#include "mem_reader.h"

/* 
 * - Returns: 
 *   -1 on error
 *   0 if not a relevant region (`region' content undetermined)
 *   1 if it is a relevant region (`region' filed)
 */
static int
get_region_info (mem_reader_t *rdr, bt_mem_region_t * region,
				 bt_addr_t offset, bt_addr_t reloc)
{
	Elf32_Ehdr      ehdr;
	Elf32_Phdr      phdr;
	int             i;
	bt_addr_t       phdr_offset;

	if (rdr->fn_safe(rdr, &ehdr, offset, sizeof(ehdr)) == -1) return -1;

	if (ehdr.e_ident[EI_MAG0] !=  ELFMAG0 ||
		ehdr.e_ident[EI_MAG1] !=  ELFMAG1 ||
		ehdr.e_ident[EI_MAG2] !=  ELFMAG2 ||
		ehdr.e_ident[EI_MAG3] !=  ELFMAG3) {
		return 0;
	}

	region->index = -1;
	region->reloc = reloc;
	region->vaddr = offset;
	region->size = ehdr.e_ehsize;

	phdr_offset = offset + ehdr.e_phoff;
	for (i = 0; i < ehdr.e_phnum; i++, phdr_offset+=ehdr.e_phentsize) {
		if (rdr->fn_safe(rdr, &phdr, phdr_offset, sizeof phdr) == -1)
			return -1;
		// Writable PT_LOAD are data, so those aren't counted
		if (phdr.p_type == PT_LOAD && !(phdr.p_flags&PF_W)) {
			/* Assumed that all PT_LOAD are contiguous at the begining
			 * of the region following the elf header */
			region->size += phdr.p_memsz;
		}
	}
	return 1;
}


int
bt_load_memmap (bt_accessor_t *acc, bt_memmap_t *memmap)
{
	pid_t           pid;
	char            path[64];
	int             fd;
	int             err = EOK;
	procfs_info     info;
	int             query_nmaps, nmaps;
	bt_mem_region_t *reg;
	procfs_mapinfo  *maps=0;
	int             i, ret;
	union {
		procfs_debuginfo    i;
		char                path[1024];
	} debug_info;
	mem_reader_t    rdr;

	if (acc==0 || memmap==0) {
		errno=EINVAL;
		return -1;
	}

	memmap->count=0;
	memmap->region=0;

	if (acc->type == BT_SELF)
		pid = getpid();
	else
		pid = acc->pid;

	sprintf(path, "/proc/%d/as", pid );

	if ((fd = open(path, O_RDONLY)) == -1) {
		err=errno;
		goto load_memmap_err;
	}

	if ((err=devctl(fd, DCMD_PROC_INFO, &info, sizeof(info), 0)) != EOK) {
		goto load_memmap_err;
	}
	if ((err=devctl(fd, DCMD_PROC_MAPINFO, NULL, 0, &query_nmaps)) != EOK) {
		goto load_memmap_err;
	}
	maps = malloc(query_nmaps * sizeof(*maps));
	if (maps == NULL) {
		err = ENOMEM;
		goto load_memmap_err;
	}
	if ((err=devctl(fd, DCMD_PROC_MAPINFO, maps,
					query_nmaps*sizeof(*maps), &nmaps)) != EOK) {
		goto load_memmap_err;
	}

	nmaps = min(query_nmaps, nmaps);
	memmap->region = calloc(nmaps, sizeof(bt_mem_region_t));
	if (memmap->region == 0) {
		err = ENOMEM;
		goto load_memmap_err;
	}

	if (pid == getpid()) {
		_bt_mem_reader_init(&rdr,
							_bt_read_mem_direct_safe,
							_bt_read_mem_direct,
							fd,
							0/*cache*/
							);
	} else {
		_bt_mem_reader_init(&rdr,
							_bt_read_mem_indirect_safe,
							_bt_read_mem_indirect,
							fd,
							alloca(MEM_RDR_CACHE_SZ)/*cache*/
							);
	}

	for (i = 0; i < nmaps; i++) {
		reg = &(memmap->region[memmap->count]);

		/*
		 * shared libs are mapped twice: once for the text segments,
		 * and once for the data segments.  So, the only maps that are
		 * interesting for backtrace are the shared elf (i.e. text segments).
		 * Skip anything else.
		 */
		if (!((maps[i].flags & MAP_ELF) && (maps[i].flags & MAP_SHARED))) {
			continue;
		}

		debug_info.i.vaddr = maps[i].vaddr;
		debug_info.i.path[0]=0;
		err = devctl(fd, DCMD_PROC_MAPDEBUG, &debug_info,
					 sizeof(debug_info), 0);
		if (err != EOK) goto load_memmap_err;

		ret=get_region_info(&rdr, reg,
							maps[i].vaddr, maps[i].vaddr-debug_info.i.vaddr);
		if (ret == -1) {
			err=errno;
			goto load_memmap_err;
		}
		if (ret == 1) {
			reg->index=memmap->count;
			reg->name = strdup(debug_info.i.path);
			if (reg->name == 0) {
				err = ENOMEM;
				goto load_memmap_err;
			}
			memmap->count ++;
		}
	}

	if (memmap->count == 0) {
		free(memmap->region);
	} else {
		// Resize the memory containing the regions, since not all map
		// will be elf
		memmap->region=realloc(memmap->region,
							   sizeof(bt_mem_region_t)*memmap->count);
	}

	close(fd);
	free(maps);
	return 0;

  load_memmap_err:
	close(fd);
	if (maps) free(maps);
	memmap->count=0;
	if (memmap->region) { free(memmap->region); memmap->region = 0; }
	errno=err;				 /* in case close+free change the errno */
	return -1;
}

#endif
