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
#ifndef malloc_common_h
#define malloc_common_h

#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <malloc-lib.h>

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

extern FlinkBins  __flist_abins[]; // available bins
extern Arena  __arenas;
extern int __flist_nbins;
extern unsigned int __ba_elem_sz; 
extern int __mallocsizes_inited;
extern unsigned __malloc_mmap_flags;

void __flist_enqueue_bin(Flink *item, size_t size, int bin);
void __flist_dequeue_bin(Flink *item, size_t size, int bin);
void __init_flist_bins(int minsize);
Fit _flist_bin_first_fit(size_t alignment, size_t size);
typedef void (*fq_fptr_t)(Flink *, Flink *);
void __return_barena(__BandArena *ba);
__BandArena *__get_barena(void);
void __init_bands_new(void);
void __malloc_sizes_init(void);
Flink *__malloc_getflistptr(void);

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


#endif /* malloc_common_h */

/* __SRCVERSION("malloc-common.h $Rev: 165018 $"); */
