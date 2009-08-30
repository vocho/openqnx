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
 * internal definitions for allocation library.
 */
#ifndef malloc_lib_h
#define malloc_lib_h

#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>


/*-
 * UNITS partitions n modulo m.
 * ROUND rounds(up) a number to a multiple of m.
 * TRUNC truncates a number mod m.
 */
#define __UNITS(n,m) (((n)+((m)-1))/(m))
#define __ROUND(n,m) ((m)*__UNITS(((ulong_t)(n)),(m)))
#define __TRUNC(n,m) ((m)*(((ulong_t)(n))/(m)))
#define PSIZ    4096

/*
 * Define to gather statistics (used by the tools)
 */
#define STATISTICS	1


typedef struct Arena    Arena;
typedef struct Dhead    Dhead;
typedef struct Dtail    Dtail;
typedef struct Flink    Flink;

/*
 * Data structure used to link all data blocks within and Arena.
 *
 * The high bits of d_size indicate the size of the buffer, including
 * the size of Dhead and its bookend structure Dtail.  If the buffer
 * is available, the low bit will be free, and the buffer will be
 * queued on a free list (see Flink below).
 */
struct Dhead {
	ssize_t		d_size;		/* size of this block, inc Dhead */
#ifdef MALLOC_PC
	unsigned int * 	d_callerpc;	/* caller's id */
#endif
#ifdef MALLOC_GUARD
	ssize_t 	d_usize;	/* user-requested size */
#endif
};

/*
 * Data structure appended to allocated buffer.
 *
 * The Dtail structure is the bookend for the Dhead structure.
 * First element must match Flink.
 *
 * d_size low bit is set when buffer is allocated.
 */
struct Dtail {
	ssize_t 	d_tail;		/* encoded size & busy-bit */
};

/*
 * Data structure to link free blocks.  First element must match Dhead.
 */
struct Flink {
	ssize_t 	f_size;		/* size of this block, inc Flink */
	Flink *		f_next;		/* next free block */
	Flink *		f_prev;		/* prev free block */
};

/*
 * Data structure used to link discontiguous memory arenas.
 * Arenas are are always page aligned and multiples of pages in size.
 */
struct Arena {
	Arena *		a_next;		/* next arena */
	Arena *		a_prev;		/* prev free block */
	int		a_size;		/* size of arena in bytes */
	struct Dtail	a_dtail;	/* mimic dtail */
};

/*
 * Dhead & Dtail low bit is set to indicate busy.
 * The DH_ macros are cast so they can be called for Flink pointers, too.
 */
#define DH_BUSY(dh)	(((Dhead *)(dh))->d_size |= 1)
#define DH_UNBUSY(dh)	(((Dhead *)(dh))->d_size &= ~1)
#define DH_ISBUSY(dh)	(((Dhead *)(dh))->d_size & 1)
#define DH_ISFREE(dh)	!DH_ISBUSY(dh)
#define DH_LEN(dh)	(((Dhead *)(dh))->d_size & ~1)
#define DH_ISARENA(dh)	(((Dhead *)(dh))->d_size == 1)

#ifdef MALLOC_GUARD
#define DH_ULEN(dh)	(((Dhead *)(d))->d_usize)
#else
#define DH_ULEN(dh)	(0)
#endif

#define D_OVERHEAD()	(sizeof(Dhead) + sizeof(Dtail))

/*
 * We xor Dhead d_size to create Dtail bits.  We do this instead of
 * just copying Dhead->d_size so that we don't get fooled by a process
 * errantly copying the same pattern over an entire Dlist block.
 * We still or-in the low bit _before_ setting d_tail, so a free block
 * has a dtail with the low bit _set_.
 */
#define DTSZ(sz)	((sz) ^ 0xffffffff)
#define DT_SET(dt,sz)	((dt)->d_tail = DTSZ(sz))
#define DT_LEN(dt)	(((dt)->d_tail ^ 0xffffffff) & ~1)
#define DT_ISFREE(dt)	((dt)->d_tail & 1)
#define DT_ISBUSY(dt)	!DT_ISFREE(dt)
#define DT_ISARENA(dt)	(DT_LEN(dt) == 0)

#define HEAD_TO_DT(dh)	(Dtail *)((char *)(dh) + DH_LEN(dh)) - 1
#define DT_TO_HEAD(dt)	(Dtail *)((char *)(dt) - DT_LEN(dt)) + 1
#define DT_TO_DHEAD(dt)	(Dhead *)(DT_TO_HEAD(dt))
#define DT_TO_FLINK(dt)	(Flink *)(DT_TO_HEAD(dt))

#define _ADJACENT(x)	((void *)((char *)x + DH_LEN(x)))

/* extra padding to ensure alignment is on _MALLOC_ALIGN */
#define MA_PADDING (__ROUND(sizeof(Dhead), _MALLOC_ALIGN)-sizeof(Dhead))
#define SET_DTAIL(dh) DT_SET(HEAD_TO_DT(dh), (dh)->d_size | 1)
#define ARENA_OVERHEAD()   (sizeof(Arena) + sizeof(Dtail))
#define TOTAL_ARENA_OVERHEAD() (ARENA_OVERHEAD() + MA_PADDING)
#define _MIN_FSIZE()	(sizeof(Flink) + sizeof(Dtail))

extern void   _list_release(Dhead *);
extern void  *_list_alloc(ssize_t);
extern void  *_list_memalign(size_t __align, ssize_t __size);
extern void  *_list_resize(void *, unsigned);
extern void   _list_gc(void);

/*
 * Small-block allocator.
 */
typedef union ListNode	ListNode;
typedef struct Band	Band;
typedef struct Block	Block;

union ListNode {
	ListNode *	ln_next;	/* when free, next entry */
	ssize_t		ln_offset;	/* when allocate, offset */
	Dhead		ln_head;	/* when allocated, for GUARD, PC */
};

struct Band {
	// the order of the first three elements is important
	// they must be in this order, and must be the first 3
	short	nbpe;	/* element size */
	short	nalloc;	/* elements per block */
	size_t slurp;
	size_t esize;
	size_t mem;
	size_t rem;
	unsigned nalloc_stats;
/*- private: */
	Block *	alist;	/* Blocks that have data to allocate */
	Block *	dlist;	/* completely allocated (depleted) Blocks */
#ifdef STATISTICS
	unsigned    blk_alloced;    /* #blocks allocated */
	unsigned    blk_freed;      /* #blocks freed */
	unsigned    alloc_counter;  /* allocs */
	unsigned    free_counter;  	/* frees */
	unsigned    blk_size;  		/* size of allocated blocks */
#endif
};

#ifdef __GNUC__
# define BLOCK_MAGIC (unsigned)0x594f484e
#else
# define BLOCK_MAGIC (unsigned)'YOHN'
#endif

struct Block {
	unsigned    magic;   /* guard */
	char *      bend;    /* pointer to end of list */
	Block      *next;    /* next block in Band     */
	Block      *prev;    /* previous block in Band */
	Band       *band;    /* this block's band */
	short       nbpe;    /* cached Band nbpe */
	short       navail;  /* number of free units in block */
	ListNode   *head;    /* freelist in block        */
};

#define CORESIZE(n)  __ROUND((n), PSIZ)
extern char  * morecore(unsigned nbytes, int fixed, unsigned *rsize);
extern int    donecore(void *p, unsigned nbytes);


extern void * _band_get(Band *p, size_t);
extern void * _band_get_aligned(Band *p, size_t __align, size_t __size);
extern void    _band_rlse(Block *p, void *up);

extern Band          *__static_Bands[];
extern unsigned       __static_nband;
extern Band           **__pBands;
extern unsigned       *__pnband;

extern int __mallocsizes_inited;

#define MAX_BAND_SIZE()	(__pBands[(*__pnband)-1]->nbpe)


/*-
 * watcom carry overs: amblksiz is the allocation request size.
 *                     (defined in <stdlib.h>)
 *                     amhiwater is the threshold to release at.
 */

extern unsigned       _amhiwater;     /* minsize to release to system */

extern struct malloc_stats _malloc_stats;

/*
 * miscellany
 */

#ifndef NDEBUG
extern void   _malloc_error(const char *file, unsigned int lineno, const char *msg);
#define panic(x)  _malloc_error(__FILE__, __LINE__, (x))
#else
#define panic(x)
#endif
extern void (*_malloc_abort)(enum mcheck_status);
extern enum mcheck_status _malloc_guard_status (void *, Dhead *, ssize_t);
extern void malloc_abort (enum mcheck_status);
#ifdef MALLOC_GUARD
extern void _malloc_check_guard (void *, Dhead *, ssize_t);
#endif

extern int malloc_opts(int cmd, void *arg2);

#include <malloc-common.h>

#endif /* malloc_lib_h */

/* __SRCVERSION("malloc-lib.h $Rev: 159801 $"); */
