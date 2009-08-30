/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <inttypes.h>

#ifndef _MALLOCDEBUG_H_
#define _MALLOCDEBUG_H_

#define MAPSIZ (PSIZ*16)

#define TOT_IN_ARENA() \
	((int)(MAPSIZ-sizeof(__Dbg_Arena))/sizeof(__Dbg_Data))

#define TOT_IN_STARENA() \
	((int)(MAPSIZ-sizeof(__Dbg_St_Arena))/sizeof(__Dbg_St))

struct __dbg_st_arena;
struct __dbg_arena;

typedef struct __Dbg_St {
	unsigned *line;
	struct __Dbg_St *next;
	struct __dbg_st_arena *da;
	void *pad;
} __Dbg_St;

struct __Dbg_Data {
	__Dbg_St *bt;
	__Dbg_St *bttail;
	struct __Dbg_Data *next;
	struct __dbg_arena *da;
	uint64_t ts;
	uint16_t cpu;
	int16_t  tid;
	void *ptr;
	void *pad;
};

typedef struct __dbg_arena {
	uint32_t numfree;
	struct __dbg_arena *next;
	struct __dbg_arena *prev;
	struct __Dbg_Data *cache;
} __Dbg_Arena;

typedef struct __dbg_st_arena {
	uint32_t numfree;
	struct __dbg_st_arena *next;
	struct __dbg_st_arena *prev;
	struct __Dbg_St *cache;
} __Dbg_St_Arena;	

void __malloc_add_dbg_bt(__Dbg_Data *dd, unsigned *line);
void __malloc_del_dbg_info(__Dbg_Data *dd);
__Dbg_Data *__malloc_add_dbg_info(__Dbg_Data *dd, uint16_t cpu, 
            int16_t tid, uint64_t ts, void *ptr);
#endif
