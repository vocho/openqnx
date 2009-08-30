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




#include <sys/types.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

//Must use <> include for building libmalloc.so
#include <malloc-lib.h>

#define bsize(n)  (sizeof(ListNode)*(((n)+sizeof(ListNode)-1)/sizeof(ListNode)))

#ifndef NDEBUG
void
_band_dump(Band *p, unsigned level)
{
	unsigned      i;
	unsigned      count=0;
	unsigned      use_count=0;
	Block        *b;
	for (i=0,b=p->alist; b; b=b->next,i++)
		count += b->navail;
		use_count += p->nalloc - count;
	printf("size %u, %u blocks %u items %u bytes\n",
		p->nbpe, i, count, count*bsize(p->nbpe));
#ifdef STATISTICS
	printf(" %u/%u allocs/frees %u/%u blocks alloced/freed\n",
		p->alloc_counter, p->free_counter, p->blk_alloced, p->blk_freed);
#endif
	if (level > 1) {
		for (b=p->alist; b; b=b->next) {
			printf(" %p..%p %u avail\n",
				b, b->bend, b->navail);
		}
	}
	for (i=0,b=p->dlist; b; b=b->next,i++)
		count += b->navail;
		use_count += p->nalloc;
#ifdef STATISTICS
	printf(" %u/%u allocs/frees %u/%u blocks alloced/freed\n",
		p->alloc_counter, p->free_counter, p->blk_alloced, p->blk_freed);
#endif
	if (level > 1) {
		for (b=p->dlist; b; b=b->next) {
			printf(" %p..%p %u avail\n",
				b, b->bend, b->navail);
		}
	}
	printf("size %u, total %u items in use taking %u bytes\n",
		p->nbpe, use_count, use_count*bsize(p->nbpe));
}

void
_list_dump(int n)
{
	Arena *	ap;
	Dhead *	dp;
	Flink *	fp;
	int	arena_count = 0;
	ssize_t	arena_memory = 0;
	int	acount = 0;
	int	fcount = 0;
	size_t	ab = 0;
	size_t	fb = 0;
	Flink *curflistptr;

	if (n <= 0) return;

	if (n >= 2) {
		Flink *fp_list;
		int i;
		curflistptr = __malloc_getflistptr();
	  printf("Arena(s) free list:\n\n");
		for (i=0; i < __flist_nbins ; i++) {
			fp_list = &(curflistptr[i]);
	    for (fp = fp_list->f_next; fp != fp_list; fp = fp->f_next)
	    {
				printf(" %p..%p\n", fp, (char *)fp + fp->f_size);
	    }
		}
	}
	fflush(stdout);

	for (ap = __arenas.a_next; ap != &__arenas; ap = ap->a_next) {
		ssize_t len;

		arena_count++;
		arena_memory += ap->a_size;

		dp = (Dhead *)(ap+1);
		while (dp->d_size) {
			len = DH_LEN(dp);
			if (DH_ISBUSY(dp)) {
				acount++;
				ab += len;
			} else {
				fcount++;
				fb += len;
			}

			dp = _ADJACENT(dp);
		}
	}

	printf("list - %d Arenas [%u bytes]\n", arena_count, arena_memory);
	printf("list - %d Allocated entries %u bytes\n", acount, ab);
	printf("list - %d Free entries %u bytes\n", fcount, fb);

}

void
_malloc_dump(int level)
{
	int     i;
	for(i=0; i < *__pnband; i++)
		_band_dump(__pBands[i], level);
	fflush(stdout);
	_list_dump(level);
}

#endif /* NDEBUG */

__SRCVERSION("mdump.c $Rev: 153052 $");
