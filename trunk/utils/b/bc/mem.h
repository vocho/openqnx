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




#ifndef _mem_h_included
#define	_mem_h_included


#ifdef	DEBUGGING


void	*new_memory(int, int);
void	*resize_memory(void *, int);
int	dispose_memory(void *);
void	mem_stats(void);

#define	ALLOCMEM(_siz,_units)	new_memory(sizeof(_siz),(_units))
#define	MALLOCMEM(_size)		new_memory(1,(_size))
#define	FREEMEM(_ptr)	dispose_memory((_ptr))
#define	MEMCHSIZE(_ptr,_nsize)	resize_memory((_ptr),(_nsize))

#else

#include <malloc.h>

#define	ALLOCMEM(_siz,_units)	calloc((_units),sizeof(_siz))
#define	MALLOCMEM(_size)		calloc(1,(_size))
#define	FREEMEM(_ptr)	free((_ptr))
#define	MEMCHSIZE(_ptr,_nsize)	realloc((_ptr),(_nsize))

#endif

#define	FR_NULL	(-1)
typedef	struct frame_hdr	FRAME;

struct frame_hdr {
	int	nentry,nsize;	/* number of entries & size */
	int	freelist;	/* free list within frame */
	void	*frbuf;		/* the frame table itself */
	FRAME   *next;		/* when buffer overflows, allocate a new pool */
};

	

FRAME	*fr_create(int entries, int size);
void *fr_alloc(FRAME *hdr);
int	fr_free(FRAME *hdr,void *p);
int	fr_delete(FRAME *hdr);


#endif
