/*
 * $QNXtpLicenseC:
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





#ifndef malloc_lib_h
#define malloc_lib_h

#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <limits.h>

/*
 * Define to gather statistics (used by the tools)
 */
#define STATISTICS 1

typedef struct Flink    Flink;
typedef struct Arena    Arena;
typedef struct Block    Block;


/*-
 * Malloc debugging structures and macros
 * Overrides definitions in normal use for malloc library
 */

    /*
     * (c) Copyright 1990, 1991 Conor P. Cahill (uunet!virtech!cpcahil).  
     * You may copy, distribute, and use this software as long as this
     * copyright statement is not removed.
     */

/*
 * since we redefine much of the stuff that is #defined in string.h and 
 * memory.h, we should do what we can to make sure that they don't get 
 * included after us.  This is typically accomplished by a special symbol
 * (similar to _malloc_g_h defined above) that is #defined when the
 * file is included.  Since we don't want the file to be included we will
 * #define the symbol ourselves.  These will typically have to change from
 * one system to another.  I have put in several standard mechanisms used to
 * support this mechanism, so hopefully you won't have to modify this file.
 */
#if 0 /* Don't continue to do this */
#ifndef _STRING_H_INCLUDED
#define _STRING_H_INCLUDED
#endif 
#ifndef _MEMORY_H_INCLUDED
#define _MEMORY_H_INCLUDED
#endif
#endif

/*
 * Malloc warning/fatal error handler defines...
 */
#define M_HANDLE_DUMP	0x80  /* 128 */
#define M_HANDLE_TRACEBACK	0x40  
#define M_HANDLE_IGNORE	0
#define M_HANDLE_ABORT	1
#define M_HANDLE_EXIT	2
#define M_HANDLE_CORE	3
#define M_HANDLE_STOP	4

/*
 * Mallopt commands and defaults
 *
 * the first four settings are ignored by the debugging mallopt, but are
 * here to maintain compatibility with the system malloc.h.
 */
#define M_KEEP		4		/* ignored by mallopt		*/
#define MALLOC_WARN	100		/* set malloc warning handling	*/
#define MALLOC_FATAL	101		/* set malloc fatal handling	*/
#define MALLOC_ERRFILE	102		/* specify malloc error file	*/
#define MALLOC_CKCHAIN	103		/* turn on chain checking	*/
#define MALLOC_FILLAREA	104		/* turn off area filling	*/
#define MALLOC_CKBOUNDS	104		/* turn on/off area filling */
#define MALLOC_LOWFRAG	105		/* use best fit allocation mech	*/
#define MALLOC_CKACCESS	106		/* verify string and memory access	*/
#define MALLOC_VERBOSE	107		/* set verbosity level		*/
#define MALLOC_EVENTFILE 108	/* specify programatic err/output file */
#define MALLOC_CKACCESS_LEVEL	109		/* verify string access more strictly*/
#define MALLOC_TRACEFILE	110		/* turn on and specify malloc trace file */
#define MALLOC_TRACEMIN	111		/* if trace is enable, minimum size to track */
#define MALLOC_TRACEMAX	112		/* if trace is enable, maximum size to track */
#define MALLOC_USE_DLADDR	113	/* Use dladdr(3), to get more info on the address */
#define MALLOC_CKALLOC		114	/* turn on/off checks for the argument of for realloc(3) and free(3) */
#define MALLOC_TRACEBTDEPTH		115 /* if trace is enable the backtrace depth */
#define MALLOC_EVENTBTDEPTH		116 /* if trace is enable the backtrace depth */
#define MALLOC_HANDLE_SIGNALS	117	/* turn on abnormal termination signals handling */
/*
 * Malloc warning/fatal error codes
 */

#define M_CODE_CHAIN_BROKE	1	/* malloc chain is broken	*/
#define M_CODE_NO_END		2	/* chain end != endptr		*/
#define M_CODE_BAD_PTR		3	/* pointer not in malloc area	*/
#define M_CODE_BAD_MAGIC	4	/* bad magic number in header	*/
#define M_CODE_BAD_CONNECT	5	/* chain poingers corrupt	*/
#define M_CODE_OVERRUN		6	/* data overrun in malloc seg	*/
#define M_CODE_REUSE		7	/* reuse of freed area		*/
#define M_CODE_NOT_INUSE	8	/* pointer is not in use	*/
#define M_CODE_NOMORE_MEM	9	/* no more memory available	*/
#define M_CODE_OUTOF_BOUNDS	10	/* gone beyound bounds 		*/
#define M_CODE_FREELIST_BAD	11	/* inuse segment on freelist	*/
#define M_CODE_BAD_CRC		12	/* bad CRC in header		*/
#define M_CODE_PTR_INSIDE	13	/* bad CRC in header		*/
#define M_CODE_NPD			14  /* null pointer dereference */
#define M_CODE_SIGNAL		15  /* signal caught */

#define CALLERPCDEPTH 124 

typedef struct debugInfo DebugInfo_t;
typedef struct __Dbg_Data __Dbg_Data;

struct debugInfo {
    ulong_t	 id;		/* identifier for malloc request */
    ulong_t	 flag;
    ulong_t	 hist_id;
    uintptr_t	 magic2;
    __Dbg_Data *dbg;
    const char	*file;
    unsigned int *callerpc_line; 	/* caller's id - pc or line # */
};

/*
 * The debugging library keeps a malloc chain and holds more info 
 * The malloc chain provides a backup/second line of defense for
 * checking in case the user has corrupted part of the allocator's view.
 */

/*
 * Data structure used to link all data blocks within and Arena.
 *
 * The high bits of d_size indicate the size of the buffer, including
 * the size of Dhead and its bookend structure Dtail.  If the buffer
 * is available, the low bit will be free, and the buffer will be
 * queued on a free list (see Flink below).
 */


typedef struct Dhead Dhead;
#define DHEAD_T
#ifdef MALLOC_DEBUG
#define d_callerpc	d_debug.callerpc_line
struct Dhead {
	_CSTD ssize_t		 d_size; 	/* size of this block, inc Dhead */
	Dhead		*d_next;	/* next free/inuse block */
	Dhead		*d_prev;	/* prev free/inuse block */
	DebugInfo_t	 d_debug;
	unsigned	 d_usize;	/* user-requested size */
	unsigned long	 d_crc;		/* CRC of header */
   	Arena *arena;
	void *pad;
};
#else
struct Dhead {
	_CSTD ssize_t		 d_size; 	/* size of this block, inc Dhead */
#ifdef MALLOC_PC
	unsigned int *	 d_callerpc;	/* caller's id */
#endif
#ifdef MALLOC_GUARD
	_CSTD ssize_t		 d_usize;	/* user-requested size */
#endif
};
#endif /* MALLOC_DEBUG */

/*
 * NB. these macros to access links require malloc-lib.h
 */

#define LinkDebug(link)	(DH_ISBUSY(link) \
			? &(((Dhead *)link)->d_debug) \
			: &(((ListNode *)link)->ln_head.d_debug))
#define LinkCRC(link)	DB_compute_crc_32(offsetof(Dhead,d_crc),(char *)link)	

extern long compute_crc_32(int __n, char *__buf);

typedef struct link {
    struct link	*next;
} link_t;

typedef struct chain {
    Flink	*head;
    Flink	*tail;
} chain_t;

#define	RANGE_BLOCK	0
#define	RANGE_ARENA	1
typedef struct arena_range {
    int		 r_type;
    union {
    	Block	*r_block;
    	Arena	*r_arena;
    } un;
    const char	*r_start;
    const char	*r_end;
    const Dhead *r_ptr;    /* cached pointer from a search */
} arena_range_t;

#ifdef _LIBMALLOC
#include "mallocint.h"
#include "malloc-debug.h"
#endif
#ifdef MALLOC_DEBUG
#include <malloc_g/prototypes.h>
#endif


/*-
 * Normal malloc library structures and macros
 * These *must* be synchronized with any changes to libc allocator
 */

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
 * Data structure appended to allocated buffer.
 *
 * The Dtail structure is the bookend for the Dhead structure.
 * First element must match Flink.
 *
 * d_size low bit is set when buffer is allocated.
 */
#ifndef DTAIL_T
#define DTAIL_T
typedef struct Dtail    Dtail;
struct Dtail {
	_CSTD ssize_t 	d_tail;		/* encoded size & busy-bit */
};
#endif

/*
 * Data structure to link free blocks.  First element must match Dhead.
 */
#ifndef FLINK_T
#define FLINK_T
struct Flink {
	_CSTD ssize_t 	f_size;		/* size of this block, inc Flink */
	Flink *		f_next;		/* next free block */
	Flink *		f_prev;		/* prev free block */
};
#endif

/*
 * Data structure used to link discontiguous memory arenas.
 * Arenas are are always page aligned and multiples of pages in size.
 */
struct Arena {
	Arena *		a_next;		/* next arena */
	Arena *		a_prev;		/* prev free block */
	int		a_size;		    /* size of arena in bytes */
#ifdef MALLOC_DEBUG
	chain_t		a_malloc_chain;	/* the malloc chain for debugging */
#endif
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
#define DH_ULEN(dh)	(((Dhead *)(dh))->d_usize)
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
#define _MIN_FSIZE()  (sizeof(Flink) + sizeof(Dtail))

#ifdef __cplusplus
extern "C" {
#endif

extern void   _list_release(Dhead *);
extern void  *_list_alloc(_CSTD ssize_t);
extern void  *_list_memalign(_CSTD size_t __align, _CSTD ssize_t __size);
extern void  *_list_resize(void *, unsigned);
extern void   _list_gc(void);

extern Arena  __arenas;
extern Flink  __flist_avail;

/*
 * Small-block allocator.
 */
typedef union ListNode	ListNode;
typedef struct Band	Band;

union ListNode {
	ListNode *	ln_next;	/* when free, next entry */
	_CSTD ssize_t		ln_offset;	/* when allocate, offset */
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
	unsigned	free_counter;	/* frees */
	unsigned	blk_size;		/* size of allocated blocks */
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
#ifdef MALLOC_DEBUG
	chain_t		malloc_chain;	/* the malloc chain for debugging */
#endif
};


#define CORESIZE(n)  __ROUND((n), PSIZ)
extern char  * morecore(unsigned nbytes, int fixed, unsigned *rsize);
extern int    donecore(void *p, unsigned nbytes);


extern void * _band_get(Band *p, _CSTD size_t);
extern void * _band_get_aligned(Band *p, _CSTD size_t __align, _CSTD size_t __size);
extern void    _band_rlse(Block *p, void *up);

extern Band          *__static_Bands[];
extern unsigned       __static_nband;
extern Band           **__pBands;
extern unsigned       *__pnband;

extern int __mallocsizes_inited;
extern unsigned __malloc_mmap_flags;

#define MAX_BAND_SIZE()	(__pBands[(*__pnband)-1]->nbpe)



/*-
 * watcom carry overs: amblksiz is the allocation request size.
 *                     (defined in <stdlib.h>)
 *                     amhiwater is the threshold to release at.
 */

extern unsigned       _amhiwater;     /* minsize to release to system */

extern struct malloc_stats _malloc_stats;

/*
 Used for communication with an external server for debug events.
*/
#ifndef _DEVCTL_H_INCLUDED
#include <devctl.h>
#endif

#define _DCMD_DBGMEM _DCMD_MISC
enum {
	DBGMEM_REGISTER,
	DBGMEM_ADDSYM 
};

struct dbg_sym_map {
	int	 vaddr;
	char name[60];
};

#define DCMD_DBGMEM_REGISTER      __DIOT(_DCMD_DBGMEM, DBGMEM_REGISTER, struct sigevent)
#define DCMD_DBGMEM_ADDSYM      __DIOT(_DCMD_DBGMEM, DBGMEM_ADDSYM, struct dbg_sym_map)


/*-
 * miscellany
 */
extern void _malloc_error(const char * const __file, unsigned int __lineno, const char * const __msg);
#ifndef NDEBUG
#define panic(x)  do { \
	malloc_errno = M_CODE_OVERRUN; \
	malloc_fatal(__FUNCTION__, NULL, 1, NULL); \
	_malloc_error(__FILE__, __LINE__, (x)); } while(0)
#else
#define panic(x)
#endif
extern void (*_malloc_abort)(enum mcheck_status);
#ifdef MALLOC_GUARD
extern void _malloc_check_guard (void *, Dhead *, _CSTD ssize_t);
#endif
extern enum mcheck_status _malloc_guard_status (void *, Dhead *, _CSTD ssize_t);
extern void malloc_abort (enum mcheck_status);
extern int malloc_opts(int cmd, void *arg2);
extern void malloc_fatal(const char *funcname, const char *file, int line, const void *link);
extern void malloc_warning(const char *funcname, const char *file, int line, const void *link);

typedef struct Fit {
  Flink *list;
  Flink *entry;
  Dhead *pos;
	int bin;
	long over;
} Fit;

typedef struct __flistbins {
	size_t size;
} FlinkBins;

struct __band_arena;

typedef struct __band_arena {
  struct __band_arena *a_next;
  struct __band_arena *a_prev;
  struct __band_arena *arena;
  struct __band_arena *b_next;
  struct __band_arena *ahead;
  int nused;
	int ntotal;
	unsigned arena_size;
} __BandArena;

#define __BARENA_TO_BLOCK(ba) ((__BandArena *)ba+1)
#define __BLOCK_TO_BARENA(blk) ((__BandArena *)blk-1)

extern FlinkBins  __flist_abins[]; /* available bins */
extern Arena  __arenas;
extern int __flist_nbins;
extern unsigned int __ba_elem_sz; 
extern unsigned __ba_must_cache;
extern int __mallocsizes_inited;

void __flist_enqueue_bin(Flink *item, size_t size, int bin);
void __flist_dequeue_bin(Flink *item, size_t size, int bin);
void __init_flist_bins(int minsize);
Fit _flist_bin_first_fit(size_t alignment, size_t size);
typedef void (*fq_fptr_t)(Flink *, Flink *);
void __return_barena(__BandArena *ba);
__BandArena *__get_barena();
void __init_bands_new();
void __malloc_sizes_init();
Flink *__malloc_getflistptr();

#define __FLIST_FIND_NBIN(__size, __bin) \
{ \
  int __i; \
  for (__i=0; __i < __flist_nbins; __i++) { \
    if (__flist_abins[__i].size >= __size) \
      break; \
  } \
  if (__i >= __flist_nbins) \
    __i = __flist_nbins-1; \
  __bin = __i; \
}

#define getmem(nbytes) \
	mmap(0, (nbytes), PROT_READ|PROT_WRITE, __malloc_mmap_flags, -1, 0)

#define putmem(cp,nbytes) \
	munmap((cp),(nbytes)) 

#ifdef __cplusplus
}
#endif


#endif /* malloclib_h */
