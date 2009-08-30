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

#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <malloc-lib.h>
#include <malloc-debug.h>

__Dbg_Arena *__mdbg_darena_ahead=NULL;
__Dbg_Arena *__mdbg_darena_atail=NULL;
__Dbg_Arena *__mdbg_darena_uhead=NULL;
__Dbg_Arena *__mdbg_darena_utail=NULL;
int num_darena=0;
__Dbg_St_Arena *__mdbg_starena_ahead=NULL;
__Dbg_St_Arena *__mdbg_starena_atail=NULL;
__Dbg_St_Arena *__mdbg_starena_uhead=NULL;
__Dbg_St_Arena *__mdbg_starena_utail=NULL;
int num_starena=0;
static int tot_in_a = TOT_IN_ARENA();
static int tot_in_sta = TOT_IN_STARENA();

#define CACHE 5

static void cache_ddt(__Dbg_Data **cache, __Dbg_Data *ddt)
{
	if (*cache == NULL) {
		*cache = ddt;
		ddt->next = NULL;
	}
	else {
		ddt->next = *cache;	
		*cache = ddt;
	}
	return;
}

static void cache_dst(__Dbg_St **cache, __Dbg_St *dst)
{
	if (*cache == NULL) {
		*cache = dst;
		dst->next = NULL;
	}
	else {
		dst->next = *cache;	
		*cache = dst;
	}
	return;
}

static __Dbg_Data *uncache_ddt(__Dbg_Data **cache)
{
	__Dbg_Data *ddt=NULL;
	if (*cache != NULL) {
		ddt = *cache;
		*cache = (*cache)->next;
	}
	return(ddt);
}

static __Dbg_St *uncache_dst(__Dbg_St **cache)
{
	__Dbg_St *dst=NULL;
	if (*cache != NULL) {
		dst = *cache;
		*cache = (*cache)->next;
	}
	return(dst);
}

static __Dbg_Arena *add_arena()
{
	__Dbg_Arena *da;
	void *addr;
	addr = mmap(0, MAPSIZ, PROT_READ|PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);	
	if (addr == MAP_FAILED)
		return(NULL);
	da = (__Dbg_Arena *)addr;
	da->numfree = tot_in_a;
	da->cache = NULL;
	da->prev = NULL;
	da->next = NULL;
	if (__mdbg_darena_ahead == NULL) {
		__mdbg_darena_ahead = __mdbg_darena_atail = da;
	}
	else {
		da->next = __mdbg_darena_ahead;
		__mdbg_darena_ahead->prev = da;
		__mdbg_darena_ahead = da;
	}
	num_darena++;
	return(da);
}

static void del_arena(__Dbg_Arena *da)
{
	if (__mdbg_darena_ahead == da) {
		__mdbg_darena_ahead = da->next;
		if (__mdbg_darena_ahead == NULL)
			__mdbg_darena_atail = NULL; 
		else {
			__mdbg_darena_ahead->prev = NULL;
		}
	}
	else {
		if (da == __mdbg_darena_atail) {
			__mdbg_darena_atail = da->prev;
			__mdbg_darena_atail->next = NULL;
		}
		else {
			da->next->prev = da->prev;
			da->prev->next = da->next;
		}
	}
	num_darena--;
	if (num_darena > CACHE)
		munmap(da, MAPSIZ);
	return;
}

static __Dbg_St_Arena *add_st_arena()
{
	__Dbg_St_Arena *da;
	void *addr;
	addr = mmap(0, MAPSIZ, PROT_READ|PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);	
	if (addr == MAP_FAILED)
		return(NULL);
	da = (__Dbg_St_Arena *)addr;
	da->numfree = tot_in_sta;
	da->cache = NULL;
	da->prev = NULL;
	da->next = NULL;
	if (__mdbg_starena_ahead == NULL) {
		__mdbg_starena_ahead = __mdbg_starena_atail = da;
	}
	else {
		da->next = __mdbg_starena_ahead;
		__mdbg_starena_ahead->prev = da;
		__mdbg_starena_ahead = da;
	}
	num_starena++;
	return(da);
}

static void del_st_arena(__Dbg_St_Arena *da)
{
	if (__mdbg_starena_ahead == da) {
		__mdbg_starena_ahead = da->next;
		if (__mdbg_starena_ahead == NULL)
			__mdbg_starena_atail = NULL; 
		else {
			__mdbg_starena_ahead->prev = NULL;
		}
	}
	else {
		if (da == __mdbg_starena_atail) {
			__mdbg_starena_atail = da->prev;
			__mdbg_starena_atail->next = NULL;
		}
		else {
			da->next->prev = da->prev;
			da->prev->next = da->next;
		}
	}
	num_starena--;
	if (num_starena > CACHE)
		munmap(da, MAPSIZ);
	return;
}

static __Dbg_Data *find_dbg_data_slot()
{
	__Dbg_Arena *da;
	__Dbg_Data *ddt;
	int num=tot_in_a;
	da = __mdbg_darena_ahead;
	if (da == NULL) {
		da = add_arena();
	}
	ddt = uncache_ddt(&(da->cache));
	if (!ddt) {
		ddt = (__Dbg_Data *)((char *)da + sizeof(__Dbg_Arena));
		ddt = (__Dbg_Data *)&ddt[num-(da->numfree)];
	}
	(da->numfree)--;
	if (da->numfree == 0) {
		__mdbg_darena_ahead = da->next;
		if (__mdbg_darena_ahead == NULL)
			__mdbg_darena_atail = NULL;
		else 
			__mdbg_darena_ahead->prev = NULL;
		if (__mdbg_darena_uhead == NULL) {
			__mdbg_darena_uhead = da;
			__mdbg_darena_utail = da;
			da->next = NULL;
			da->prev = NULL;
		}
		else {
			__mdbg_darena_uhead->prev = da;
			da->next = __mdbg_darena_uhead;
			__mdbg_darena_uhead = da;
			da->prev = NULL;
		}
	}
	ddt->da = da;
	return(ddt);	
}

static __Dbg_St *find_dbg_st_slot()
{
  __Dbg_St_Arena *da;
  __Dbg_St *dst;
	int num=tot_in_sta;
  da = __mdbg_starena_ahead;
  if (da == NULL) {
    da = add_st_arena();
	}
	dst = uncache_dst(&(da->cache));
	if (!dst) {
		dst = (__Dbg_St *)((char *)da + sizeof(__Dbg_St_Arena));
		dst = (__Dbg_St *)&dst[num-(da->numfree)];
	}
	(da->numfree)--;
	if (da->numfree == 0) {
		__mdbg_starena_ahead = da->next;
		if (__mdbg_starena_ahead == NULL)
			__mdbg_starena_atail = NULL;
		else
			__mdbg_starena_ahead->prev = NULL;
		if (__mdbg_starena_uhead == NULL) {
			__mdbg_starena_uhead = da;
			__mdbg_starena_utail = da;
			da->next = NULL;
			da->prev = NULL;
		}
		else {
			__mdbg_starena_uhead->prev = da;
			da->next = __mdbg_starena_uhead;
			__mdbg_starena_uhead = da;
			da->prev = NULL;
		}
	}
	dst->da = da;
  return(dst);
}

void __malloc_add_dbg_bt(__Dbg_Data *dd, unsigned *line) 
{
  __Dbg_St *dst;
	if (line == 0)
		return;
	dst = find_dbg_st_slot();
	if (dst == NULL)
		return;
	dst->line = line;
	dst->next = NULL;
	if (dd->bt == NULL) {
		dd->bt = dst;
		dd->bttail = dst; 
	}
	else {
		dd->bttail->next = dst;
		dd->bttail = dst;
	}
	return;
}

static void del_dbg_st_chain(__Dbg_St *dst)
{
	__Dbg_St *ds;
	__Dbg_St *dstemp;
	__Dbg_St_Arena *da;
	int num=tot_in_sta;
	if (dst == NULL)
		return;
	ds = dst;
	while (ds != NULL) {
		dstemp = ds->next;
		ds->line = 0;
		da = ds->da;
		(da->numfree)++;
		if (da->numfree == 1) {
			if (da == __mdbg_starena_uhead) {
				__mdbg_starena_uhead = da->next;
				if (__mdbg_starena_uhead == NULL)
					__mdbg_starena_utail = NULL;
				else
					__mdbg_starena_uhead->prev = NULL;
			}
			else {
				if (__mdbg_starena_utail == da) {
					__mdbg_starena_utail = da->prev;
					__mdbg_starena_utail->next = NULL;
				}
				else {
					da->next->prev = da->prev;
					da->prev->next = da->next;
				}
			}
			da->prev = NULL;
			if (__mdbg_starena_ahead == NULL) {
				__mdbg_starena_ahead = __mdbg_starena_atail = da;
				da->next = NULL;
			}
			else {
				da->next = __mdbg_starena_ahead;
				__mdbg_starena_ahead->prev = da;
				__mdbg_starena_ahead = da;
			}
		}
		if (da->numfree >= num)	
			del_st_arena(da);
		else
			cache_dst(&(da->cache), ds);
		ds = dstemp;
	}
	return;
}

__Dbg_Data *__malloc_add_dbg_info(__Dbg_Data *odd, uint16_t cpu,  
                    int16_t tid, uint64_t ts, void *ptr)
{
	__Dbg_Data *dd;
	if (odd == NULL)
		dd = find_dbg_data_slot();
	else {
		dd = odd;
		if (dd->bt) {
			del_dbg_st_chain(dd->bt);
		}
		dd->bt = NULL;
		dd->bttail = NULL;
	}
	if (dd != NULL) {
		dd->cpu = cpu;
		dd->tid = tid;
		dd->ts = ts;
		dd->ptr = ptr;
		dd->bt = NULL;	
		dd->bttail = NULL;
	}
	return(dd);
}

void __malloc_del_dbg_info(__Dbg_Data *dd)
{
	__Dbg_Arena *da;
	int num=tot_in_a;
	if (dd == NULL)
		return;
	da = dd->da;
	del_dbg_st_chain(dd->bt);
	dd->bt = NULL;
	dd->bttail = NULL;
	dd->ptr = 0;	
	(da->numfree)++;
	if (da->numfree == 1) {
		if (da == __mdbg_darena_uhead) {
			__mdbg_darena_uhead = da->next;
			if (__mdbg_darena_uhead == NULL)
				__mdbg_darena_utail = NULL;
			else
				__mdbg_darena_uhead->prev = NULL;
		}
		else {
			if (__mdbg_darena_utail == da) {
				__mdbg_darena_utail = da->prev;
				__mdbg_darena_utail->next = NULL;
			}
			else {
				da->next->prev = da->prev;
				da->prev->next = da->next;
			}
		}
		da->prev = NULL;
		if (__mdbg_darena_ahead == NULL) {
			__mdbg_darena_ahead = __mdbg_darena_atail = da;
			da->next = NULL;
		}
		else {
			da->next = __mdbg_darena_ahead;
			__mdbg_darena_ahead->prev = da;
			__mdbg_darena_ahead = da;
		}
	}
	if (da->numfree >= num) {
		del_arena(da);
	}
	else 
		cache_ddt(&(da->cache), dd);
	return;
}

void print_status()
{
	__Dbg_Arena *dt;
	__Dbg_St_Arena *dst;
	printf("num arenas = %d\n", num_darena);
	printf("num starenas = %d\n", num_starena);
	dt = __mdbg_darena_ahead;
	while (dt) {
		printf("tot = %d\n", tot_in_a);
		printf("arena 0x%08x, numfree = %d\n", (unsigned)dt, dt->numfree);
		dt = dt->next;
	}
	dt = __mdbg_darena_uhead;
	while (dt) {
		printf("tot = %d\n", tot_in_a);
		printf("arena 0x%08x, numfree = %d\n", (unsigned)dt, dt->numfree);
		dt = dt->next;
	}
	dst = __mdbg_starena_ahead;
	while (dst) {
		printf("tot = %d\n", tot_in_sta);
		printf("arena 0x%08x, numfree = %d\n", (unsigned)dst, dst->numfree);
		dst = dst->next;
	}
	dst = __mdbg_starena_uhead;
	while (dst) {
		printf("tot = %d\n", tot_in_sta);
		printf("arena 0x%08x, numfree = %d\n", (unsigned)dst, dst->numfree);
		dst = dst->next;
	}
}
