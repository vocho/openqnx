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

#ifndef _BACKTRACE_H_INCLUDED
#define _BACKTRACE_H_INCLUDED

#include <limits.h>
#include <pthread.h>

__BEGIN_DECLS

/* Use opaque type in case we eventually want to backtrace remote
 * systems, which in theory could have a different word size */
typedef unsigned bt_addr_t;
#define BT_ADDR_INVALID UINT_MAX

#ifndef _BT_LIGHT

typedef struct {
	int index;
	char *name;
	bt_addr_t vaddr;            /* address in memory */
	bt_addr_t reloc;            /* relocation that gave vaddr. i.e.
								 * elf file vaddr=vaddr-reloc */
	size_t size;                /* size of region in memory (elf
								 * header+PT_LOAD segments) */
} bt_mem_region_t;

typedef struct {
	bt_mem_region_t *region;
	int count;
} bt_memmap_t;

#endif

typedef struct {
	int       type;
	unsigned  flags;
	pid_t     pid;
	pthread_t tid;
	bt_addr_t stack_limit;
} bt_accessor_t;


// bit 0: on: live backtrace, off: HOLD before backtrace
#define BTF_LIVE_BACKTRACE 0x00000001
#define BTF_DEFAULT        0x00000000

typedef enum {
	BT_SELF,
#ifndef _BT_LIGHT    
	BT_THREAD,
	BT_PROCESS,
#endif
} bt_acc_type_t;

#ifndef _BT_LIGHT
int bt_init_accessor(bt_accessor_t *acc, bt_acc_type_t type, ...);
int bt_set_flags(bt_accessor_t *acc, unsigned flags, int onoff);
#endif

int bt_get_backtrace(bt_accessor_t *acc, bt_addr_t *addrs, int len);
int btl_get_backtrace(bt_accessor_t *acc, bt_addr_t *addrs, int len);

#ifndef _BT_LIGHT
int bt_release_accessor(bt_accessor_t *acc);
#endif

#ifndef _BT_LIGHT
int bt_sprnf_addrs(bt_memmap_t *memmap,
				   bt_addr_t *addrs, int addrslen,
				   char *fmt, char *out, size_t outlen, char *seperator);

int bt_load_memmap(bt_accessor_t *acc, bt_memmap_t *memmap);
/* Note:
 * - some or all of reladdrs, offsets, indexes, filenames can be null
 *   if those specific info are not wanted.
 *   If filenames are requested, those are not strdup'ed.  The filenames
 *   will only be valid as long as the memmap is not unloaded.
 */
void bt_translate_addrs(bt_memmap_t *memmap, bt_addr_t *addrs, int arylen,
						bt_addr_t *reladdrs, bt_addr_t *offsets,
						int *index, char **filenames);
int bt_sprn_memmap(bt_memmap_t *memmap, char *out, size_t outlen);
void bt_unload_memmap(bt_memmap_t *memmap);

#endif

extern void (*bt_gather_hook)(bt_addr_t sp);
extern void (*btl_gather_hook)(bt_addr_t sp);
extern bt_accessor_t bt_acc_self;
extern bt_accessor_t btl_acc_self;

__END_DECLS

#endif
