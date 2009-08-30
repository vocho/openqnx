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

#include <stdlib.h>
#include <stdio.h>
#include "backtrace.h"

int
bt_sprn_memmap (bt_memmap_t *memmap, char *out, size_t outlen)
{
	bt_mem_region_t *reg;
	int i;
	int written=0;

	written+=snprintf(out, max(0,outlen-written),
					  "====================\n"
					  "***** MEMMAP  *****\n");
	for (i=0; i < memmap->count; i++) {
		reg=&(memmap->region[i]);
		written+=snprintf(out+written, max(0,outlen-written),
						  "--------------------\n%d %s\n"
						  "vaddr=%p reloc=%p size=%08llx",
						  reg->index, reg->name,
						  (void*)reg->vaddr, (void*)reg->reloc,
						  (long long unsigned)reg->size);
		if (reg->reloc)
			written+=snprintf(out+written, max(0,outlen-written),
							  " elf vaddr=%p\n",
							  (void*)(reg->vaddr-reg->reloc));
		else
			written+=snprintf(out+written, max(0,outlen-written),"\n");
	}
	written+=snprintf(out+written, max(0,outlen-written),
					  "====================\n\n");
	if (written>=outlen)
		return -1;
	return 0;
}


void
bt_translate_addrs (bt_memmap_t *memmap, bt_addr_t *addrs, int arylen,
					bt_addr_t *reladdrs, bt_addr_t *offsets,
					int *indexes, char **filenames)
{
	bt_addr_t addr, reladdr, offset;
	bt_mem_region_t *reg=0;
	int i,j;
	int index;
	char *name=0;

	for (i=0; i < arylen; i++) {
		addr=addrs[i];
		reladdr=addr;
		offset=0;
		index=-1;
		for (j=0; j < memmap->count; j++) {
			reg=&(memmap->region[j]);
			if (addr >= reg->vaddr) {
				if ((addr - (reg->vaddr)) < reg->size) {
					offset = reg->reloc;
					reladdr -= offset;
					index=reg->index;
					name=reg->name;
					break;
				}
			}
		}
		if (reladdrs)
			reladdrs[i]=reladdr;
		if (offsets)
			offsets[i]=offset;
		if (indexes)
			indexes[i]=index;
		if (filenames)
			filenames[i]=name;
	}
}

void
bt_unload_memmap (bt_memmap_t *memmap)
{
	int i;
	if (memmap->count) {
		for (i=0; i < memmap->count; i++) {
			free(memmap->region[i].name);
		}
		free(memmap->region);
	}
}

#endif
