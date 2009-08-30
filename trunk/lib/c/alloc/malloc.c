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
 * malloc family.
 *
 * This malloc library uses different allocation strategies
 * for different sized objects in an attempt to obtain high
 * performance and low waste.
 *
 * The standard malloc library is based upon an sbrk() model;
 * attempts to physically reduce the data size of the program
 * can only be made when the top of the heap is unused.  QNX
 * also thwarts this by refusing to shrink the data segment in
 * brk() to avoid interfering with mmap().
 *
 * The base allocator in this model only has two functions:
 * morecore(n); and donecore(n,s).
 * This implementation uses mmap() in morecore() to acquire memory
 * and munmap() in donecore() to release it.
 *
 * To enhance performances; small objects are placed on a set of lists
 * in band.c.  The band is a linked list of blocks of these small objects.
 * Each of these blocks contains an internal list of objects; the
 * allocation and release of which are constant time.  When the last
 * object of a block is allocated, the block is placed on the band's
 * depleted-object list; blocks with unallocated objects are on the band's
 * alloc-object list.  When there are no allocatable blocks/objects, and a
 * new object is requested, a new block is allocated by a call to malloc.
 *
 * Larger objects are allocated from free-lists.  To minimize fragmentation,
 * the large objects are kept on a classic first fit allocator.  When more
 * memory is needed, an arena of new memory is allocated (via morecore/mmap),
 * and if all the memory in the arena is eventually freed, the arena is
 * returned to the system (via donecore/munmap).
 *
 * Both allocators maintain a Dhead structure immediately preceding the object
 * which contains information about the object.  The small-object Dhead
 * contains an offset -- which is always negative -- to the object's block.
 * The large-object Dhead contains the object size, plus bits tha indicate
 * whether it is free or allocated, etc.
 * Since the small-block Dhead contains a negative value, the large-object
 * allocator cannot fulfill requests for very large (unsigned) sizes --
 * sizes which would overflow when viewed as signed values.  This is not
 * a great burden for a system which does not support paging.
 *
 * If MALLOC_GUARD is defined, the Dhead also contains the caller-requested
 * size -- information which is used when checking the sanity of the heap.
 * If MALLOC_PC is defined, the caller's pc value is also stored in the Dhead.
 * Various debugging programs can display the caller pc and user size info.
 */

#include <sys/types.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <unistd.h>

//Must use <> include for building libmalloc.so
#include <malloc-lib.h>
#include <limits.h>

/*-
 * select a band
 */

#define NEWSELBAND(__size, v, __p) \
	{ \
		__p = NULL; \
		for (v=0; v < *__pnband; v++)  { \
			if (__size <= (__pBands[v])->nbpe) { \
				__p = __pBands[v]; \
				break; \
			} \
		} \
	}

#ifndef MALLOC_DEBUG
#define PTHREAD_CALL(p) p
#else
#define PTHREAD_CALL(p) pthread##p
#endif

extern int _malloc_check_on;
extern int _malloc_monotonic_growth;
int _malloc_fill_area;

#ifndef _LIBMALLOC
ssize_t _msize (void *ptr);
#else
extern ssize_t _msize (void *ptr);
#endif
extern void _check_list (int force);
extern void _check_bands (void);
extern enum mcheck_status mprobe(void *ptr);

# define VERIFY_HEAP() if (_malloc_check_on) _check_list(0)
# define UNLOCKED_VERIFY_HEAP() if (_malloc_check_on) { \
		PTHREAD_CALL(_mutex_lock(&_malloc_mutex)); \
		_check_list(0); \
		PTHREAD_CALL(_mutex_unlock(&_malloc_mutex)); \
}

#ifdef STATISTICS

/*
 * Statistics gathering for dlists
 */

#define DL_STAT_NBINS   13
#define DL_STAT_MAX_SIZE  (1<<(DL_STAT_NBINS-1))

struct _dlist_stat_bin {
  unsigned  size;
  unsigned  nallocs;
  unsigned  nfrees;
  unsigned  reserved;
};

#define DLIST_STAT_INDEX(s, i)	{ unsigned ret = 1<<(DL_STAT_NBINS-1), idx = (DL_STAT_NBINS-1); i = 0;	\
		if((s) > ret) i = idx; else 				\
		while(ret) { 							\
			if((s-1) & ret) { i = idx; break; } 	\
			ret >>= 1; idx--;					\
			}									\
		}

/*
 * Initialization of the statistics structures
 * counters are associated with bins of prev_size+1 to size
 */
struct _dlist_stat_bin    __dlist_stat_bins[DL_STAT_NBINS] =
  {{2, 0, 0, 0},
   {4, 0, 0, 0},
   {8, 0, 0, 0},
   {16, 0, 0, 0},
   {32, 0, 0, 0},
   {64, 0, 0, 0},
   {128, 0, 0, 0},
   {256, 0, 0, 0},
   {512, 0, 0, 0},
   {1024, 0, 0, 0},
   {2048, 0, 0, 0},
   {4096, 0, 0, 0},
   {~0, 0, 0, 0}};    /* Last bin catches all bigger allocs */

unsigned          __ndlist_stat_bins = DL_STAT_NBINS;

#define __update_dlist_stats_nallocs(__size, __num) \
{\
	int __i; \
	DLIST_STAT_INDEX(__size, __i); \
	__dlist_stat_bins[__i].nallocs+=(__num); \
	/* return; */ \
}

#define __update_dlist_stats_nfrees(__size, __num) \
{ \
	int __i; \
	DLIST_STAT_INDEX(__size, __i); \
	__dlist_stat_bins[__i].nfrees+=(__num); \
	/* return; */ \
}

#endif

static int mall_init;

static int mopts;
#define MOPT_SYSV_REALLOC	0x00000001
#define MOPT_SYSV_MALLOC	0x00000002

static void
do_init(void)
{
	char *p;


	mall_init = 1;

	if ((p = getenv("MALLOC_OPTIONS")) == NULL) {
		return;
	}

	for (; *p != '\0'; p++) {
		switch (*p) {
		case 'V':
			mopts |= MOPT_SYSV_MALLOC;
			break;

		case 'v':
			mopts &= ~MOPT_SYSV_MALLOC;
			break;

		case 'R':
			mopts |= MOPT_SYSV_REALLOC;
			break;

		case 'r':
			mopts &= ~MOPT_SYSV_REALLOC;
			break;

		default:
			break;
		}
	}
}



#ifdef MALLOC_GUARD
/*
 * We set guard code here, and not within _list- or band- allocators
 * because realloc can play nasty with user data, copying willy-nilly.
 */
#ifndef MALLOC_DEBUG
void
set_guard (void *ptr, size_t usize)
{
	/* ssize_t	bsize = _msize(ptr); */
	Dhead *dh = (Dhead *)ptr - 1;
	ssize_t bsize;
	int i;

	dh->d_usize = 0;
	if (!_malloc_check_on) return;
	bsize = _msize(ptr);

	dh->d_usize = usize;
	i = bsize - usize;

	if (i) {
		char *gp = (char *)ptr + usize;
		if (i <= 0)
			malloc_abort(MCHECK_HEAD);
		while (i > 0) {
			*gp++ = (char)i--;
		}
	}
}
#endif

enum mcheck_status
_malloc_guard_status (void *ptr, Dhead *dh, ssize_t bsize)
{
#ifndef MALLOC_DEBUG
	int i = bsize - dh->d_usize;
	if (dh->d_usize <= 0) return MCHECK_DISABLED;	/* New mcheck strategy */
	if (i) {
		char *gp = (char *)ptr + dh->d_usize;
		if (i <= 0)
			return (MCHECK_HEAD);
		while (i > 0) {
			if (*gp != (char)i)
				return (MCHECK_TAIL);
			gp++, i--;
		}
	}
#endif
	return MCHECK_OK;
}

void
_malloc_check_guard (void *ptr, Dhead *dh, ssize_t bsize)
{
#ifndef MALLOC_DEBUG
	enum mcheck_status status;
	if ((status = _malloc_guard_status(ptr,dh,bsize)) > MCHECK_OK) {
		malloc_abort(status);
	}
#endif
}
#endif

pthread_mutex_t _malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

struct malloc_stats _malloc_stats;

void __prelocked_free(void *ptr)
{

	Dhead *		dh;
#ifdef STATISTICS
	ssize_t osize;
#endif

	dh = (Dhead *)ptr - 1;		/* point to Dhead header */

	if (_malloc_check_on ) {
		enum mcheck_status mcheck_stat;
		if ((mcheck_stat = mprobe(ptr)) > MCHECK_OK) {
			PTHREAD_CALL(_mutex_unlock(&_malloc_mutex));
			malloc_abort(mcheck_stat);
			/* shouldn't really get here */
			PTHREAD_CALL(_mutex_lock(&_malloc_mutex));
		}
	}

	VERIFY_HEAP();

	if (dh->d_size < 0) {
		Block *b = (Block *)((char *)dh + dh->d_size);
#ifdef STATISTICS
#ifdef _LIBMALLOC
	osize = _msize(ptr);
#else
	osize = b->nbpe;
#endif
#endif
        _band_rlse(b, ptr);
	} else {
#ifdef STATISTICS
#ifdef _LIBMALLOC
	osize = _msize(ptr);
#else
	osize = DH_LEN(dh) - D_OVERHEAD();
#endif
#endif
		_list_release(dh);
	}
#ifdef STATISTICS
	__update_dlist_stats_nfrees(osize, 1);
#endif

	_malloc_stats.m_frees++;

}

void
__free (void *ptr)
{
	if (!ptr)
		return;

	PTHREAD_CALL(_mutex_lock(&_malloc_mutex));

	__prelocked_free(ptr);

	PTHREAD_CALL(_mutex_unlock(&_malloc_mutex));
}

void *
#ifdef MALLOC_PC
__malloc_pc_lock(size_t size, unsigned int *pc, int lockl)
#else
__malloc_lock(size_t size, int lockl)
#endif
{
	unsigned     v;
	Band       *p;
	void        *x;
#ifndef NDEBUG
	unsigned     u;
	static       int first;
#endif
#ifdef STATISTICS
	ssize_t osize=0;
#endif
	/*
	 * If perilously close to 0, this will wrap on SELBAND.
	 * We make a bigger swipe here as to not bother _list_alloc
	 * with unreasonable requests.
	 */
	if ((ssize_t)size < 0) {
		errno = ENOMEM;
		return NULL;
	}

	if ((mopts & MOPT_SYSV_MALLOC) && size == 0) {
		return NULL;
	}

	if (lockl)
		PTHREAD_CALL(_mutex_lock(&_malloc_mutex));

	if (!__mallocsizes_inited)
		__malloc_sizes_init();
#ifndef NDEBUG
	if (!first) {
		first = 1;
		for (u=0; u < 256; u++) {
			NEWSELBAND(u,v,p);
			if (p != NULL) {
				if (p->nbpe < u) {
					p->nbpe = u;
					u=0;
					if (p->nbpe & (_MALLOC_ALIGN-1)) {
						panic("nbpe");
					}
				}
			}
		}
	}
#endif

	VERIFY_HEAP();

	NEWSELBAND(size, v, p);
	if (p != NULL) {
		x = _band_get(p, size);
		if (x) {
			assert (((Dhead *)x - 1)->d_size < 0);
#ifdef STATISTICS
#ifndef _LIBMALLOC
			osize = p->nbpe;
#else
			osize = _msize(x);
#endif
#endif
		}
	} else {
		x = _list_alloc((ssize_t)size);
#ifdef STATISTICS
		if (x) {
#ifndef _LIBMALLOC
			osize = DH_LEN(((Dhead *)x -1)) - D_OVERHEAD();
#else
			osize = _msize(x);
#endif
		}
#endif
	}
#ifdef STATISTICS
	__update_dlist_stats_nallocs(osize, 1);
#endif

	_malloc_stats.m_allocs++;

	if (lockl)
		PTHREAD_CALL(_mutex_unlock(&_malloc_mutex));

#ifdef MALLOC_GUARD
  if (x)
    set_guard(x, size);
#endif
#ifdef MALLOC_PC
  if (x) {
    Dhead *dh = (Dhead *)x - 1;
    dh->d_callerpc = pc;
  }
#endif

  return x;
}

/*
 * Could replace malloc_pc/malloc with memalign_pc
 */
void *
#ifdef MALLOC_PC
__malloc_pc (size_t size, unsigned int *pc)
#else
__malloc (size_t size)
#endif
{
	if (!mall_init) {
		do_init();
	}
#ifdef MALLOC_PC
	return(__malloc_pc_lock(size, pc, 1));
#else
	return(__malloc_lock(size, 1));
#endif
}

int
select_band(size_t alignment, size_t size, int *vp, Band **pp)
{
	size_t	 dividend;
	Band	*p;
	int x;
	for (x=0; x < *__pnband; x++)  {
		if (size <= (__pBands[x])->nbpe) {
			p = __pBands[x]; \
			if ((dividend = (alignment / p->esize)) <= p->nalloc
				&& dividend * p->esize == alignment) { /* an even multiple */
				if (vp != NULL) 
					*vp = x;
				if (pp != NULL) 
					*pp = p;
				return 1;
			}
		}
	}
	return 0;
}

void *
#ifdef MALLOC_PC
__memalign_pc_lock(size_t alignment, size_t size, unsigned int *pc, int lockl)
#else
__memalign_lock(size_t alignment, size_t size, int lockl)
#endif
{
	int         v;
	Band       *p;
	void        *x;
#ifndef NDEBUG
	unsigned     u;
	static       int first;
#endif
#ifdef STATISTICS
	ssize_t osize=0;
#endif
	/*
	 * If perilously close to 0, this will wrap on SELBAND.
	 * We make a bigger swipe here as to not bother _list_alloc
	 * with unreasonable requests.
	 */
	if ((ssize_t)size < 0) {
		errno = ENOMEM;
		return NULL;
	}
	if ((alignment&(_MALLOC_ALIGN-1)) != 0) {
		errno = EINVAL;
		return NULL;
	} else if (alignment == 0 || alignment == _MALLOC_ALIGN) {
#ifdef MALLOC_PC
			return __malloc_pc_lock(size, pc, lockl);
#else
		return __malloc_lock(size, lockl);
#endif
	}

	if (lockl)
		PTHREAD_CALL(_mutex_lock(&_malloc_mutex));

	if (!__mallocsizes_inited)
		__malloc_sizes_init();
#ifndef NDEBUG
	if (!first) {
		first = 1;
		for (u=0; u < 256; u++) {
			NEWSELBAND(u,v,p);
			if (p != NULL) {
				if (p->nbpe < u) {
					p->nbpe = u;
					u=0;
					if (p->nbpe & (_MALLOC_ALIGN-1)) {
						panic("nbpe");
					}
				}
			}
		}
	}
#endif

	VERIFY_HEAP();

	if (select_band(alignment, size, &v, &p)) {
		x = _band_get_aligned(p, alignment, size);
		if (x) {
			assert (((Dhead *)x - 1)->d_size < 0);
#ifdef STATISTICS
#ifndef _LIBMALLOC
			osize = p->nbpe;
#else
			osize = _msize(x);
#endif
#endif
		}
	} else {
		x = _list_memalign(alignment, (ssize_t)size);
#ifdef STATISTICS
		if (x) {
#ifndef _LIBMALLOC
			osize = DH_LEN(((Dhead *)x -1)) - D_OVERHEAD();
#else
			osize = _msize(x);
#endif
		}
#endif
	}
#ifdef STATISTICS
	__update_dlist_stats_nallocs(osize, 1);
#endif
	_malloc_stats.m_allocs++;

	if (lockl)
		PTHREAD_CALL(_mutex_unlock(&_malloc_mutex));

#ifdef MALLOC_GUARD
  if (x)
    set_guard(x, size);
#endif
#ifdef MALLOC_PC
  if (x) {
    Dhead *dh = (Dhead *)x - 1;
    dh->d_callerpc = pc;
  }
#endif

  return x;
}

void *
#ifdef MALLOC_PC
__memalign_pc (size_t alignment, size_t size, unsigned int *pc)
#else
__memalign (size_t alignment, size_t size)
#endif
{
#ifdef MALLOC_PC
	return(__memalign_pc_lock(alignment, size, pc, 1));
#else
	return(__memalign_lock(alignment, size, 1));
#endif
}

#ifdef MALLOC_PC
void *
__memalign (size_t align, size_t size)
{
	unsigned int *pc = __builtin_return_address(0);

	return __memalign_pc(align, size, pc);
}
#endif

/*-
 * TRUNC truncates(down) a number to a multiple of m.
 */
#define __TRUNC(n,m) ((m)*(((ulong_t)(n))/(m)))

int __posix_memalign(void **memptr, size_t alignment, size_t size) {
#ifdef MALLOC_PC
        void *__memalign_pc(size_t __alignment, size_t __size, unsigned int *__pc);
        unsigned int *pc = __builtin_return_address(0);
#endif
        ulong_t bit = 0x1;
        void *p;

        while (bit && (alignment&bit) == 0) bit <<= 1;

        /* if not a power of two or a multiple of word size */
        if ((alignment&~bit) != 0
        	|| __TRUNC(alignment,_MALLOC_ALIGN) != alignment) {
                return EINVAL;
        }
#ifdef MALLOC_PC
        p = __memalign_pc(alignment, size, pc);
#else
        p = __memalign(alignment, size);
#endif
	if (p != NULL) *memptr = p;
	return p != NULL ? EOK : errno;
}

void *
valloc (size_t size)
{
#ifdef MALLOC_PC
	unsigned int *pc = __builtin_return_address(0);
	return __memalign_pc(getpagesize(), size, pc);
#else
	return __memalign(getpagesize(), size);
#endif
}

/*-
 * In Band allocator, the word preceding alloc pointer contains
 * offset to Band's Block.  In List allocator, the preceding word
 * contains its size.
 */
#ifndef _LIBMALLOC
ssize_t _msize (void *ptr)
{
	Dhead *dh = (Dhead *)ptr - 1;

	if (dh->d_size < 0) {
		Block *b = (Block *)((char *)dh + dh->d_size);
		if (b->magic == BLOCK_MAGIC)
			return (b->nbpe);
		panic("size!");
	}

	return DH_LEN(dh) - D_OVERHEAD();
}
#endif

void *
__realloc (void *ptr, size_t size)
{
	Band *was, *will;
	ssize_t osize;
	size_t oband, nband;
	void       *x = 0;

	if (!mall_init) {
		do_init();
	}

	if (ptr == 0)
		return malloc(size);

	if (size == 0) {
		free(ptr);
		if ((mopts & MOPT_SYSV_REALLOC) == 0) {
			return malloc(0);
		} else {
			return NULL;
		}
		
	}
	if ((ssize_t)size < 0) {
		errno = ENOMEM;
		return NULL;
	}

	osize = _msize(ptr);
	was = 0; will = 0;

	/*
	 * Don't need pthread protection to pick bands.
	 */
	NEWSELBAND(osize, oband, was);
	NEWSELBAND(size,  nband, will);


	if (will) {
		if (was == will) {
			/* both sizes mapped to the same band.  */
		 	x = ptr;
			UNLOCKED_VERIFY_HEAP();
		}
	} else if (was == 0) {
		PTHREAD_CALL(_mutex_lock(&_malloc_mutex));
		VERIFY_HEAP();
		x = _list_resize(ptr, size);
#ifdef STATISTICS
		if (x) {
			ssize_t nsize;
			nsize = _msize(x);
			__update_dlist_stats_nfrees(osize, 1);
			__update_dlist_stats_nallocs(nsize, 1);
		}
#endif
		PTHREAD_CALL(_mutex_unlock(&_malloc_mutex));
		if (x == 0 && !_malloc_monotonic_growth && osize > (_amblksiz*4)) { /* it's bound to grow more */
			if ((osize + osize/2) > size) {
				size = osize + osize/2;
			}
		}
	} /* else crossing domains */

	if (x == 0 && (x = malloc(size))) {
		/* memcpy is ok: they can't overlap */
		memcpy(x, ptr, osize < size ? osize : size);
		free(ptr);
	}
#ifdef MALLOC_GUARD
	if (x)
		set_guard(x, size);
#endif
	_malloc_stats.m_reallocs++;
	return x;
}

#ifdef MALLOC_PC
void *
calloc_pc (size_t n, size_t size, unsigned int *caller_pc)
#else
void *
__calloc (size_t n, size_t size)
#endif
{
#ifdef MALLOC_PC
	void *p = malloc_pc(n*size, caller_pc);
#else
	void *p = malloc(n*size);
#endif
	if (p)
		memset(p, 0, n*size);
	return p;
}

__SRCVERSION("malloc.c $Rev: 212948 $");
