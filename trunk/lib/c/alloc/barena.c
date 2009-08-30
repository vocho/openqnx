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
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
//Must use <> include for building libmalloc.so
#include <malloc-lib.h>
#include <limits.h>
#include <inttypes.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <assert.h>
#include <sys/mman.h>

static __BandArena *__barena_head=NULL;
static __BandArena *__barena_dhead=NULL;
unsigned int __ba_elem_sz=PSIZ;
extern unsigned _amblksiz;

#define ENQ(__h, __ba) \
	{ \
		if (__h == NULL) { \
			__h = __ba; \
			__ba->a_next = __ba->a_prev = NULL; \
		} \
		else { \
			__ba->a_prev = NULL; \
			__ba->a_next = __h; \
			__h->a_prev = __ba; \
			__h = __ba; \
		} \
	}

#define DEQ(__h, __ba) \
	{ \
		if (__ba == __h) { \
			__h = __ba->a_next; \
			if (__h != NULL) \
				__h->a_prev = NULL; \
		} \
		else { \
			__ba->a_prev->a_next = __ba->a_next; \
			if (__ba->a_next != NULL)  \
				__ba->a_next->a_prev = __ba->a_prev; \
		} \
	}

static __BandArena *__map_barena(unsigned basize, unsigned *rsize)
{
	void *addr;
	__BandArena *ba;
	addr = morecore(basize, 0, rsize);
	if (addr == (void *)-1)
		return(NULL);
	ba = (__BandArena *)addr;
	memset(ba, 0, sizeof(__BandArena));
	_malloc_stats.m_small_freemem += *rsize;
	return(ba);
}

static void __unmap_barena(__BandArena *ba, unsigned basize)
{
	void *addr;
	addr = (__BandArena *)ba;
	donecore(addr, basize);
	_malloc_stats.m_small_freemem -= basize;
	return;
}

__BandArena *__get_barena()
{
	unsigned rsize;
	__BandArena *atemp=NULL;
	__BandArena *btemp=NULL;
	if (__barena_head != NULL) {
		atemp = __barena_head;
	}
	else {
		atemp = __map_barena(_amblksiz, &rsize);
		if (atemp == NULL)
			return(NULL);
		ENQ(__barena_head, atemp);
		atemp->nused = 0;
		atemp->ntotal = rsize/__ba_elem_sz; 
		atemp->arena_size = rsize;
	}
	if (atemp->ahead != NULL) {
		btemp = atemp->ahead;
		atemp->ahead = btemp->b_next;
	}
	else {
		btemp = (__BandArena *)((char *)atemp + (atemp->nused * __ba_elem_sz));
	}
	atemp->nused++;
	if (atemp->nused == atemp->ntotal) {
		DEQ(__barena_head, atemp);
		ENQ(__barena_dhead, atemp);
	}
	btemp->arena = atemp;
	_malloc_stats.m_small_overhead += sizeof(__BandArena);
	_malloc_stats.m_small_freemem -= __ba_elem_sz;
	return(btemp);
}

void __return_barena(__BandArena *ba)
{
	__BandArena *atemp;
	atemp = ba->arena;
	if (atemp->nused == atemp->ntotal) {
		DEQ(__barena_dhead, atemp);
		ENQ(__barena_head, atemp);
	}
	atemp->nused--;
	if (atemp->nused == 0) {
		DEQ(__barena_head, atemp);
		__unmap_barena(atemp, atemp->arena_size);
		_malloc_stats.m_small_overhead -= sizeof(__BandArena);
		_malloc_stats.m_small_freemem += __ba_elem_sz;
		return;
	}
	ba->b_next = atemp->ahead;
	atemp->ahead = ba;
	_malloc_stats.m_small_overhead -= sizeof(__BandArena);
	_malloc_stats.m_small_freemem += __ba_elem_sz;
	return;
}

__SRCVERSION("barena.c $Rev: 159801 $");
