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




/*-
 memory handling, for both buffer pools and varlength

description:
	Frame provides a convienient allocator for fixed-size requests.
	It builds a single header block containing block sizes and a free list.
	Since the requests are fixed size, there is limited search required to
	allocate and deallocate blocks, thus higher performance.   In applications
	where many "little" blocks  are allocated, this approach saves significant
	amount of memory.

public:
	FRAME	*fr_create(int entries, int size)
	description:
		creates the buffer pool for allocation.
	returns:
		0 -- if no memory, or has been previously created.
		1 -- if create is OK.

	void *fr_alloc(FRAME *hdr)
	description:
		removes a buffer from the "free-list", and returns pointer to it.
	returns:
		0 -- no more space available.
		non-0 -- pointer to allocated buffer.

	int	fr_free(FRAME *hdr,void *p)
	description:
		adds buffer p to free list.  Note: does not verify that p is valid,
		other than non-null.
	returns:
		0 -- buffer pool not initialized, or p == 0.
		1 -- p added to buffer pool.

	int	fr_delete(FRAME *hdr)
	description:
		resets the buffer routines.
	returns :
		0 -- buffer not previously allocated.
		1 -- OK.

NOTES:
	1.	Only handles FIXED-BLOCK allocation (performance ).
	2.	Requires EXPLICIT initialization, (ie. used at interrupt time,
		so cannot use allocate memory.)

*/




#include "number.h"
#include <stdlib.h>


void 
mem_stats()
{
	return;
}


void           *
new_memory(int size, int nelem)
{
	void           *p = calloc(nelem, size);
	if (p == 0)
		error(0, "new_memory: no memory\n");
	return p;
}


void           *
resize_memory(void *ptr,int nlen)
{
	void           *p;
	if ((p = realloc(ptr, nlen)) == 0)
		error(0, "resize_memory: no memory\n");
	return p;

}

int dispose_memory(void *ptr)
{
	free(ptr);
	return 1;
}



static
void init_list(FRAME *fr)
{
	int             i;
	char           *ptr;

	for (i = 0, ptr = fr->frbuf; ++i < fr->nentry;) {
		*(int *) ptr = i * fr->nsize;
		ptr += fr->nsize;
	}
	*(int *) ptr = FR_NULL;
	fr->freelist = 0;	/* beginning of buffer */
}

FRAME          *
fr_create(int entries, int size)
{
	FRAME          *tab;

	if (size < sizeof(char **))
		size = sizeof(char **);	/* ensure size for free list */

	if ((tab = ALLOCMEM(FRAME, 1)) == 0) {
		return 0;
	}
	tab->nentry = entries;
	tab->nsize = size;
	if ((tab->frbuf = MALLOCMEM(entries * size)) == 0) {
		FREEMEM(tab);
		return 0;
	}
	init_list(tab);
	tab->next = 0;		/* continuation block */
	return tab;
}


int fr_delete(FRAME *fr)
{
	FRAME          *p, *c;
	for (p = fr; p != 0; p = c) {
		if (p->frbuf != 0)
			FREEMEM(p->frbuf);
		c = p->next;
		FREEMEM(p);
	}
	return 1;
}

void           *
fr_alloc(FRAME *fr)
{
	FRAME          *p, *c;
	void           *uptr;

	if (fr == 0) {
		return 0;
	}
	for (c = fr, p = 0; c != 0; p = c, c = c->next) {
		if (c->freelist != FR_NULL) {
			/* take this member out of the free list */
			uptr = (char *) c->frbuf + c->freelist;
			c->freelist = *(int *) uptr;
			return uptr;
		}
	}
	/* no memory found, add to the chain */
	if (p == 0)
		p = fr;
	if ((p->next = fr_create(fr->nentry, fr->nsize)) == 0) {
		no_mem("fr_alloc");
		return 0;
	}
	p = p->next;
	uptr = (char *) p->frbuf + p->freelist;
	p->freelist = *(int *) uptr;
	return uptr;
}

int fr_free(FRAME *fr, void *ptr)
{
	FRAME          *p, *c;
	for (c = fr, p = 0; c != 0; p = c, c = c->next) {
		if ((ptr >= (void*)c->frbuf) && (ptr < (void*)(((char*) c->frbuf) + c->nentry * c->nsize)))
			break;
	}
	if (c == 0)
		return error(1, "fr_free, invalid pointer");
	*(int *) ptr = c->freelist;
	c->freelist = (char *) ptr - (char *) c->frbuf;
	return 1;
}
