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



/*
 * doubly linked first fit allocator with checking.
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
#include <sys/mman.h>
#include <ctype.h>

#define NALLOC()	__ROUND(_amblksiz, PSIZ)
#define UNITSIZE(n)	(__ROUND(((n) + D_OVERHEAD()),_MALLOC_ALIGN))
/* TODO: See if we have a tighter bound */
#define ALIGNUNITSIZE(align,n)	(__ROUND((n),align)+__ROUND(D_OVERHEAD(),align))

/*
 * Various global lists.  One for Arenas, two for Flinks.
 */
#ifndef MALLOC_DEBUG
Arena      __arenas = { &__arenas, &__arenas, 0, { DTSZ(1) } };
#else
Arena      __arenas = { &__arenas, &__arenas, 0, {0, 0}, { DTSZ(1)} };
#endif

int __mallocsizes_inited=0;
unsigned __malloc_mmap_flags=MAP_ANON|MAP_PRIVATE;
unsigned __must_prealloc=0;
unsigned __pre_allocsz=0;
void __core_prealloc_cache(unsigned size);
extern unsigned __core_cache_max_num;
extern unsigned __core_cache_max_sz;
void __core_cache_adjust(void);
Band **__pBands=__static_Bands;
unsigned *__pnband=&__static_nband;
extern Band *__dynamic_Bands[];
extern unsigned __dynamic_nband;

static void delmem(Flink *this)
{
	Arena	*ac;

	ac = (Arena *)this -1; 
	_malloc_stats.m_overhead -= TOTAL_ARENA_OVERHEAD();

	donecore((Arena *)((char *)ac-MA_PADDING), ac->a_size);
	assert(_malloc_stats.m_overhead + _malloc_stats.m_freemem + _malloc_stats.m_allocmem + _malloc_stats.m_small_overhead + _malloc_stats.m_small_allocmem + _malloc_stats.m_small_freemem == _malloc_stats.m_heapsize);

}

#define FLIST_DEQUEUE(f)	{	\
	Flink *fp = (f)->f_prev; \
	Flink *fn = (f)->f_next; \
	fp->f_next = fn; \
	fn->f_prev = fp; \
}

/*
 * Queue f onto list "l"
 */
#define QUEUE_LIFO
#ifdef QUEUE_LIFO
#define FLIST_QUEUE(f, l)	{	\
	Flink *fn = (l)->f_next; \
	(f)->f_prev = (l); \
	(f)->f_next = fn; \
	fn->f_prev = (f); \
	(l)->f_next = (f); \
}
#else
#define FLIST_QUEUE(f, l)	{	\
	Flink *tail = (l)->f_prev; \
	(f)->f_prev = tail; \
	(f)->f_next = (l); \
	tail->f_next = (f); \
	(l)->f_prev = (f); \
	if (tail == (l)) { \
		(l)->f_next = (f); \
	} \
}
#endif

int _malloc_check_fd = 2;
void (*_malloc_abort)(enum mcheck_status) = 0;
int _malloc_check_on = 0;
int _malloc_monotonic_growth = 0;
int _malloc_free_check = 0;
static int checked_on = 0;
int _min_free_list_size = 0;

static void __flist_queue_lifo(Flink *f, Flink *l)
{
  Flink *fn = l->f_next;
  f->f_prev = l;
  f->f_next = fn;
  fn->f_prev = f;
  l->f_next = f;
  return;
}

static void __flist_queue_fifo(Flink *f, Flink *l)
{
  Flink *tail = l->f_prev;
  f->f_prev = tail;
  f->f_next = l;
  tail->f_next = f;
  (l)->f_prev = f;
  if (tail == l) {
    l->f_next = f;
  }
  return;
}

fq_fptr_t __flist_queue_funcptr=__flist_queue_fifo;

#ifndef NDEBUG
# define VERIFY_ARENA(ptr) _check_arena(ptr,1) 
# define VERIFY_LIST() _check_list(1)
#else
# define VERIFY_ARENA(ptr) ((_malloc_check_on) ? _check_arena(ptr,0) : 0)
# define VERIFY_LIST() if (_malloc_check_on) _check_list(0)
#endif

int
_check_arena (void *ptr, int force)
{
	Arena *ap;
	Dhead *dp, *dpend;
	Dtail *dt;
	unsigned   arena=0;
	unsigned   count;
	extern void _check_bands(void);

	if (!force && (++checked_on < _malloc_check_on))
		return 0;

	checked_on = 0;

	for (ap = __arenas.a_next; ap != &__arenas ; arena++, ap = ap->a_next) {

		dp = (Dhead *)(ap + 1);
		dpend = (Dhead *)((char *)ap + ap->a_size - sizeof(Dtail));

		if ((ulong_t)ptr < (ulong_t)dp
			|| (ulong_t)ptr > (ulong_t)dpend) continue;

		if (ap->a_size < sizeof(*ap))
			return -1; //panic("a_size");
		dt = &ap->a_dtail;
		if ((DT_LEN(dt) != 0) || DT_ISFREE(dt))
			return -2; //panic("a_dsize");

		dp = (Dhead *)(ap + 1);
		dpend = (Dhead *)((char *)ap + ap->a_size - sizeof(Dtail));

		count = 0;
		while (dp < dpend) {
			ssize_t dlen;

			dlen = DH_LEN(dp);
			if (dlen <= 0) {
				return -3; //panic("dlen <= 0");
				//return;
			}
			dt = HEAD_TO_DT(dp);
			if (DT_LEN(dt) != DH_LEN(dp))
				return -4; //panic("dt->d_size != dp->d_size");
			if (DH_ISFREE(dp)) {
				Flink *fp = (Flink *)dp;
				if (!DH_ISFREE(fp->f_next))
					return -6;//panic("!DH_ISFREE(fp->f_next)");
				if (!DH_ISFREE(fp->f_prev))
					return -7;//panic("!DH_ISFREE(fp->f_prev)");
				if (fp->f_prev->f_next != fp)
					return -8;//panic("fp->f_prev->f_next != fp");
				if (fp->f_next->f_prev != fp)
					return -9;//panic("fp->f_prev->f_prev != fp");
			}
			dp = (Dhead *)((char *)dp + dlen);
		}
	}
	return 0;
}

static int parse_str(char *str, Band ***bc_conf, unsigned *bc_num)
{
	int num_found = 0;
	char *ind;	
	int i,j,value;
	int is_bad=0;
	unsigned size=0;
	void *addr=NULL;
	static Band *lband;
	unsigned psize;
	unsigned asize;
	if (str == NULL)
		return(-1);
	*bc_conf = NULL;
	ind = str;
	num_found = atoi(ind);
	if (num_found < 0)
		return(-1);
	// string format is 
	// N:s1,n1,p1:s2,n2,p2:s3,n3,p3: ... :sN,nN,pN
	// no spaces are acceptable.. only valid chars are 
	// digits, ':' and ','. Position is important
	// parsing is simple and strict
	// sizes are assumed to be provided in ascending order
	// further validation is done by the allocator
	// if this function doesnt like the string, it ignores 
	// it completely.
	// s = size, n = number , p = prealloc number
	// all must be specified.. a number of zero is acceptable for
	// prealloc
	while (isdigit(*ind)) ind++; // skip forward all the digits
	if (*ind != ':') { 
		return(-1);
	}
	psize = (num_found * sizeof(Band *));
	asize = (num_found * sizeof(Band));
	size = psize + asize;
	size = __ROUND(size, PSIZ);
	addr = getmem(size);
	if (addr == (void *)-1)
		return(-1);
	lband = (Band *)addr;
	*bc_conf = (Band **)((char *)addr + asize);
	_malloc_stats.m_heapsize += size;
	for (i=0; i < num_found; i++) {
		if (is_bad)	
			break;
		ind++;
		(lband)[i].nbpe = atoi(ind);	
		for (j=0; j < i; j++) {
			if ((lband)[i].nbpe < (lband)[j].nbpe) {
				is_bad=1;
				break;
			}
		}
		while (isdigit(*ind)) ind++; // skip forward all the digits
		if (*ind != ',') { // must be a comma now
			is_bad=1;
			break;
		}
		ind++;
		if (!isdigit(*ind)) { // must be followed by a number
			is_bad=1;
			break;
		}
		(lband)[i].nalloc = atoi(ind);	
		if ((lband)[i].nalloc < 0) {
			is_bad=1;
			break;
		}
		while (isdigit(*ind)) ind++; // skip forward all the digits
		if (*ind != ',') { // must be a comma now
			is_bad=1;
			break;
		}
		ind++;
		if (!isdigit(*ind)) { // must be followed by a number
			is_bad=1;
			break;
		}
		value = atoi(ind);	
		if (value < 0) {
			is_bad=1;
			break;
		}
		(lband)[i].slurp = (unsigned)value;	
		while (isdigit(*ind)) ind++; // skip forward all the digits
		if (i < (num_found-1)) { // must be a colon now
			if (*ind != ':') {
				is_bad=1;
				break;
			}
		}
	}
	if (!is_bad) {
		*bc_num = num_found;
		for (i=0; i < num_found; i++)
			(*bc_conf)[i] = &(lband[i]);
	}
	else {
		*bc_num = 0;
		if (addr != NULL)
			putmem(addr, size);
		*bc_conf = NULL;
		_malloc_stats.m_heapsize -= size;
		return(-1);
	}
	return(0);
}

void
_check_list (int force)
{
	Arena *ap;
	Dhead *dp, *dpend;
	Dtail *dt;
	unsigned   arena=0;
	unsigned   count;
	extern void _check_bands(void);

	if (!force && (++checked_on < _malloc_check_on))
		return;

	checked_on = 0;

	for (ap = __arenas.a_next; ap != &__arenas ; arena++, ap = ap->a_next) {

		if (ap->a_size < sizeof(*ap)) {
			panic("a_size");
		}
		dt = &ap->a_dtail;
		if ((DT_LEN(dt) != 0) || DT_ISFREE(dt)) {
			panic("a_dsize");
		}

		dp = (Dhead *)(ap + 1);
		dpend = (Dhead *)((char *)ap + ap->a_size - sizeof(Dtail));

		count = 0;
		while (dp < dpend) {
			ssize_t dlen;

			dlen = DH_LEN(dp);
			if (dlen <= 0) {
				panic("dlen <= 0");
				return;
			}
			dt = HEAD_TO_DT(dp);
			if (DT_LEN(dt) != DH_LEN(dp)) {
				panic("dt->d_size != dp->d_size");
			}
			if (DH_ISFREE(dp)) {
				Flink *fp = (Flink *)dp;
				if (!DH_ISFREE(fp->f_next)) {
					panic("!DH_ISFREE(fp->f_next)");
				}
				if (!DH_ISFREE(fp->f_prev)) {
					panic("!DH_ISFREE(fp->f_prev)");
				}
				if (fp->f_prev->f_next != fp) {
					panic("fp->f_prev->f_next != fp");
				}
				if (fp->f_next->f_prev != fp) {
					panic("fp->f_prev->f_prev != fp");
				}
			}
			dp = (Dhead *)((char *)dp + dlen);
		}
	}

	_check_bands();
}

static void check_free_list (Flink *me)
{
	Flink *fp;
	Flink *fp_list;
	int i;
	Flink *curflistptr;

	curflistptr = __malloc_getflistptr();
	for (i=0; i < __flist_nbins ; i++) {
		fp_list = &(curflistptr[i]);
		for (fp = fp_list->f_next; fp != fp_list; fp = fp->f_next)
		{
			if (fp == me) {
				panic("free -- already on free list");
				break;
			}
		}
	}
}

static void get_environ_vars()
{
  char *p;
  /* if the user does not malloc-ed memory returned from an mmap
     call initialised to zero, they can set this environment variable
     to a non-zero value. Upon unmap memory is zero-ed by default by the 
     kernel anyway. To change this behaviour, one can set the procnto flag
     -m~i */

  p = getenv("MALLOC_MMAP_NOZERO");
  if (p) {
	int sz = atoi(p);
	if (sz > 0) {
		__malloc_mmap_flags = MAP_ANON|MAP_PRIVATE|MAP_NOINIT;
	}
	else {
		__malloc_mmap_flags = MAP_ANON|MAP_PRIVATE;
	}
  }
  p = getenv("MALLOC_ARENA_SIZE");
  if (p) {
    int sz = atoi(p);
    int pagesize = sysconf(_SC_PAGESIZE);

    if ((sz <= 0x40000) && (sz >= pagesize)) {
      _amblksiz = __ROUND(sz, (unsigned)pagesize);
    }
  }
  p = getenv("MALLOC_MEMORY_BANDCONFIG");
	if (p) {
		// bands are configured in a special way
		// we will need to re-assign the band structure
		// and change the default number of bands.
		if (__dynamic_nband) {
			__pBands = __dynamic_Bands;
			__pnband = &__dynamic_nband;
		}
		else {
			__pBands = __static_Bands;
			__pnband = &__static_nband;
		}
	}
	else {
		 // no static config
		 // fall back to the default
		__pBands = __static_Bands;
		__pnband = &__static_nband;
	}
	p = getenv("MALLOC_BAND_CONFIG_STR");
	if (p) {
		int status;
		static Band **__local_bands;
		static unsigned __local_nband;
		status = parse_str(p, &__local_bands, &__local_nband);
		if (status == 0) { // success
			__pBands = __local_bands;
			__pnband = &__local_nband;
		}
	}
  p = getenv("MALLOC_MEMORY_PREALLOCATE");
  if (p) {
    int sz = atoi(p);
		if (sz >= 0) {
			__pre_allocsz = __ROUND(sz, _amblksiz);
			__must_prealloc=1;
		}
		else {
			__pre_allocsz = 0;
			__must_prealloc=0;
		}
  }
  p = getenv("MALLOC_MEMORY_HOLD");
  if (p) {
    int sz = atoi(p);
    if (sz > 0) {
			// explicitly set the hold for both types of holding to -1
      __core_cache_max_num = (unsigned)-1;
      __core_cache_max_sz = (unsigned)-1;
		}
  }
  p = getenv("MALLOC_FREE_LIFO");
  if (p) {
    __flist_queue_funcptr=__flist_queue_lifo;
  }
	p = getenv("MALLOC_ARENA_CACHE_MAXSZ");
	if (p) {
		int newval = atoi(p);
		if (newval > 0) {
			__core_cache_max_sz = newval;
		}
	}
	p = getenv("MALLOC_ARENA_CACHE_MAXBLK");
	if (p) {
		int newval = atoi(p);
		if (newval >= 0) {
			__core_cache_max_num = newval;
		}
	}
  return;
}

void __malloc_sizes_init()
{
	if (__mallocsizes_inited)
		return;
  __mallocsizes_inited=1;
	get_environ_vars(); // initialise environment
	// allocate cache if necessary
	if (__must_prealloc) {
		__core_prealloc_cache(__pre_allocsz);
	}
	// initialise band values
  __init_bands_new();
	_min_free_list_size = UNITSIZE(MAX_BAND_SIZE() + 1);
	__init_flist_bins(MAX_BAND_SIZE()); // initialise free list bins
	return;
}

/*
 * Allocate more core.  "nb" includes caller's Dhead overhead, but not
 * any extra data structures to tie this into an Arena.
 */
static Dhead *
addmem (ssize_t nb, int alignpage)	/* ask system for more memory */
{
	Arena	*ac;
	char	*cp;		// ptr to arena memory
	Dhead	*dp;		// return this pointer
	ssize_t *ph;		// pointer to pseudo-header at end of arena
	ssize_t	nbytes;		// allocated arena size
	ssize_t	size;		// temp
	unsigned rsize = 0;

	/*
	 * Compute the minimum free-list block size.  This is the smallest
	 * block that should be put on the free list
	 * The value is the smallest size that can actually be requested
	 * of the list allocator [assuming malloc is playing fair].
	 * We can put it off the calculation until here because nothing's
	 * ever put on the free list until we've allocated some memory.
	 */
	if (0 == __mallocsizes_inited) {
		__malloc_sizes_init();
	}

	/*
	 * Add size of Arena plus our Arena footer.
	 */
  size = NALLOC();    // temp variable
	nbytes = nb + TOTAL_ARENA_OVERHEAD();
	if (!alignpage)
		nbytes = __ROUND(nbytes, (unsigned)size);
	else
		nbytes = __ROUND(nbytes, PSIZ);
	if (nbytes < 0) {
		errno = ENOMEM;
		return NULL;
	}
	if ((cp = morecore(nbytes, 0, &rsize)) == (void *)-1)
		return NULL;
	nbytes = rsize;

	/*
	 * Set this arena's tailing header.  It appears to be a zero-length,
	 * busy block.
	 */
	ph = (ssize_t *)(cp + nbytes - sizeof(*ph));
	*ph = 1;

	/*
	 * Link into unordered arena list.
	 */
	ac = (Arena *)((char *)cp + MA_PADDING);
#ifdef MALLOC_DEBUG
	ac->a_malloc_chain.head = NULL;
	ac->a_malloc_chain.tail = NULL;
#endif
	ac->a_size = nbytes;
	DT_SET(&ac->a_dtail, 1); // zero-length and busy

	ac->a_next = &__arenas;
	ac->a_prev = __arenas.a_prev;
	__arenas.a_prev = ac;
	ac->a_prev->a_next = ac;

	dp = (Dhead *)(ac + 1);
	dp->d_size = nbytes - TOTAL_ARENA_OVERHEAD();
#ifdef MALLOC_DEBUG
	dp->arena = ac;
#endif

	_malloc_stats.m_overhead += TOTAL_ARENA_OVERHEAD();

	return dp;
}

static void delmem_precache(Flink *this)
{
	Arena	*ac;
	Arena	*ap, *an;

	ac = (Arena *)this -1; 
#ifndef NDEBUG
	assert(DH_LEN(this) == ac->a_size - TOTAL_ARENA_OVERHEAD());
	for (ap = __arenas.a_next; ap != &__arenas; ap = ap->a_next) {
		if (ap == ac) {
			ap = NULL;
			break;
		}
	}
	assert(!ap);
#endif

#ifndef NDEBUG
	if (1 == _malloc_check_on) {
		int i = 0;
		Flink *	cur;
		Flink * end = NULL;
		Flink *fp_list;
		Flink *curflistptr;
		int j;
		curflistptr = __malloc_getflistptr();
		for (j=0; j < __flist_nbins ; j++) {
			fp_list = &(curflistptr[j]);
			for (cur = fp_list->f_next; cur != fp_list; cur = cur->f_next) {
				if (i++ > 10000)
					panic("delmem");
				end = (Flink *)((char *)ac + ac->a_size);
				assert(cur < (Flink *)ac || cur >= end);
			}
		}
	}
#endif

	ap = ac->a_prev;
	an = ac->a_next;

	ap->a_next = an;
	an->a_prev = ap;
	return;
}

static void cache_delmem(Flink *this)
{
	//Arena	*ac;
	//ac = (Arena *)this-1; 
	delmem_precache(this);
	_malloc_stats.m_freemem -= this->f_size;
	delmem(this);
}

/* New way!
 * We don't even pretend to be optimal for non-word alignments 
 */
void *
_list_memalign(size_t alignment, ssize_t n_bytes)
{
	Flink *	cur = NULL;	/* avoid warnings */
	Dhead *	dlink = NULL;
	Dtail * dt;
	ssize_t	nbytes;
	Fit	fit;
	int split=1;

	fit.pos = NULL;
	if ((alignment & (_MALLOC_ALIGN-1)) != 0) {
		errno = EINVAL;
		return NULL;
	}
	if (alignment == 0) alignment = _MALLOC_ALIGN;

	/*
	 * Round up to alignment size plus Dhead header and Dtail footer.
	 */
	nbytes = UNITSIZE(n_bytes);
	if (nbytes < 0) {
		errno = ENOMEM;
		return NULL;
	}

	if ((n_bytes >= __flist_abins[__flist_nbins-1].size))
		split=0;
	/* findFit will remove it from the list */
	if (split)
		fit = _flist_bin_first_fit(alignment, nbytes);
	if ((dlink = (Dhead *)fit.pos) == NULL) {   /* need to get one */
		size_t amt;

		if (alignment < _amblksiz) {
			amt = ALIGNUNITSIZE(alignment,n_bytes);
				if (split)
					dlink = addmem(amt, 0);
				else
					dlink = addmem(amt, 1);
			if (dlink == NULL) {
				return NULL;
			}
		} else {
			/* Do an aligned mmap? Too tricky with Arena & morecore */
			amt = ALIGNUNITSIZE(alignment,n_bytes);
			if (split)
				dlink = addmem(amt, 0);
			else
				dlink = addmem(amt, 1);
			if (dlink == NULL) {
				return NULL;
			}
		}
		_malloc_stats.m_blocks++;
	} else {
		/* It must be aligned */
		Flink *cur2 = fit.entry;
		assert(__TRUNC(dlink+1, alignment) == (ulong_t)(dlink+1));
		assert(fit.over >= 0);
		if (fit.over) {
			long n;
			size_t orig_size;
			n = cur2->f_size - fit.over;
			if (fit.over >= (_min_free_list_size + _MIN_FSIZE())) {
				Dtail *dt2;
				_malloc_stats.m_freemem -= cur2->f_size;
				orig_size = cur2->f_size;
				cur2->f_size = fit.over;
				_malloc_stats.m_freemem += cur2->f_size;
				_malloc_stats.m_blocks++;
				// if the bin has changed, move the free block to the 
				// correct bin
				{
					int newbin;
					__FLIST_FIND_NBIN(cur2->f_size, newbin);
					if (newbin != fit.bin) {
						__flist_dequeue_bin(cur2, orig_size, fit.bin);
						__flist_enqueue_bin(cur2, cur2->f_size, newbin);
					}
				}
#ifdef MALLOC_DEBUG
				((Dhead *)cur2)->arena = dlink->arena;
#endif
				//
				dt2 = HEAD_TO_DT(cur2);
				DT_SET(dt2,cur2->f_size);
				assert(_ADJACENT(cur2) == fit.pos);
				dlink->d_size = n;
			}
		} else {
			__flist_dequeue_bin(cur2, cur2->f_size, fit.bin);
			_malloc_stats.m_freemem -= cur2->f_size;
		}
	}
	/* We always come through here just to make sure we carve out
	   enough in case the fit is too loose */
	if ((dlink != NULL) && (split || (alignment != _MALLOC_ALIGN))) {   /* got one */
		ssize_t over;
		void *vptr;
		ulong_t caddr;

		assert(dlink->d_size >= nbytes);

		/*
		 * If there's room to carve another free bucket, do so.
		 * To ease memory alignment, take ours from the tail.
		 */
		over = dlink->d_size - nbytes;
		if (alignment == _MALLOC_ALIGN
			|| ((vptr = (void *)(dlink+1))
				&& (caddr = __TRUNC(vptr,alignment)) == (ulong_t)vptr)) {
			/* carve out ours from start and put extra on free list */
			if (over >= (_min_free_list_size + _MIN_FSIZE())) {
				dlink->d_size = nbytes;
				cur = _ADJACENT(dlink);
				cur->f_size = over;
				dt = HEAD_TO_DT(cur);
				DT_SET(dt,over);
#ifdef MALLOC_DEBUG
				((Dhead *)cur)->arena = dlink->arena;
#endif
				{
					// find the right bin and drop the extra there
					int newbin;
					__FLIST_FIND_NBIN(cur->f_size, newbin);
					__flist_enqueue_bin(cur, cur->f_size, newbin);
					//
				}
				_malloc_stats.m_freemem += over;
				_malloc_stats.m_blocks++;
			}
		} else if (over >= (_min_free_list_size + _MIN_FSIZE())) { 
			Dhead *dh;
			long d;

			/*
			 * For aligned requests that don't match the
			 * block alignment, carve the block from the
			 * tail.  Happily, these only come from memalign
			 * or small block headers; therefore, there won't
			 * be any attempts to realloc() the block (realloc
			 * makes no claims about what happens to memalign
			 * memory, so it's not safe to use).
			 */

			dlink->d_size -= nbytes;
			dh = _ADJACENT(dlink);
			vptr = dh + 1;
			caddr = __TRUNC(vptr, alignment);
			d = (ulong_t)vptr - caddr;
			dh = (Dhead *)caddr - 1;

			dlink->d_size -= d;
			over = dlink->d_size;
			cur = (Flink *)dlink;
			dlink = dh;
			dlink->d_size = nbytes + d;

			dt = HEAD_TO_DT(cur);
			assert(over >= _MIN_FSIZE());
			DT_SET(dt,over);
			{
				// find the right bin and drop the extra there
				int newbin;
				__FLIST_FIND_NBIN(cur->f_size, newbin);
				__flist_enqueue_bin(cur, cur->f_size, newbin);
				//
			}
			_malloc_stats.m_freemem += over;
			_malloc_stats.m_blocks++;
		} else {
			_malloc_stats.m_blocks++;
		}
	}

	_malloc_stats.m_allocmem += (dlink->d_size - D_OVERHEAD());
	_malloc_stats.m_overhead += D_OVERHEAD();
	assert(_malloc_stats.m_overhead + _malloc_stats.m_freemem + _malloc_stats.m_allocmem + _malloc_stats.m_small_overhead + _malloc_stats.m_small_freemem + _malloc_stats.m_small_allocmem == _malloc_stats.m_heapsize);

	DH_BUSY(dlink);
	SET_DTAIL(dlink);

	return (void *) (dlink+1);
}

void *
_list_alloc(ssize_t n_bytes)
{
	return _list_memalign(_MALLOC_ALIGN, n_bytes);
}


void
_list_release(Dhead *dh)
{
	Flink *this, *neighbour;
	Dtail *dt, *ft;
	int newbin;
	int oldbin;
	size_t orig_tsize;
	size_t orig_nsize;

	if (_malloc_free_check) {
		check_free_list((Flink *)dh);
	}
	if (!DH_ISBUSY(dh)) {
		panic("free malloc object that is not allocated");
	}
	DH_UNBUSY(dh);

#if defined(MALLOC_GUARD) && !defined(MALLOC_DEBUG)
	_malloc_check_guard((void *)(dh + 1), dh, DH_LEN(dh) - D_OVERHEAD());
	dt = HEAD_TO_DT(dh);
	if (!DT_ISBUSY(dt) || (DT_LEN(dt) != dh->d_size)) {
		panic("free -- corrupt object");
	}
#endif

	this = (Flink *)dh;		/* ... as free list pointer */

	_malloc_stats.m_freemem += this->f_size;
	_malloc_stats.m_allocmem -= (this->f_size - D_OVERHEAD());
	_malloc_stats.m_overhead -= D_OVERHEAD();

	/*
	 * If previous block is free, just add this block to it.
	 * Otherwise, queue this in avail-list.
	 */
	dt = (Dtail *)dh - 1;		/* previous block's dtail */
	orig_tsize = this->f_size;
	if (DT_ISFREE(dt)) {
		// previous block free
		neighbour = DT_TO_FLINK(dt);
		// remember orig size of back neighbour
		orig_nsize = neighbour->f_size;
		//
		assert(DH_ISFREE(neighbour));
		_malloc_stats.m_blocks--;
		neighbour->f_size += this->f_size;
		this = neighbour;
		dt = (Dtail *)this - 1;		/* for carve test below */
		// check next block also
		neighbour = _ADJACENT(this);
		if (DH_ISFREE(neighbour)) {
			// forward neighbour free: dequeue
			__flist_dequeue_bin(neighbour, neighbour->f_size, -1);
			//
			_malloc_stats.m_blocks--;
			this->f_size += neighbour->f_size;
			neighbour = _ADJACENT(this);
		}
		/*
	 	* Now that all the dust has settled, set this's footer.
	 	*/
		ft = HEAD_TO_DT(this);
		DT_SET(ft,this->f_size);
	
		if (_malloc_check_on) {
			if (!DT_ISBUSY(dt) || !DH_ISBUSY(neighbour)) {
				panic("free -- corrupt heap");
			}
		}
		// check if we've become an arena
		if ((DT_ISARENA(dt)) && (DH_ISARENA(neighbour))) {
			// dequeue
			__flist_dequeue_bin(this, orig_nsize, -1);
			//
			cache_delmem(this);
			_malloc_stats.m_blocks--;
			return; // done
		}
		else {
			// previous was free, and we didnt become an arena
			__FLIST_FIND_NBIN(orig_nsize, oldbin);
			__FLIST_FIND_NBIN(this->f_size, newbin);
			if (oldbin != newbin) {
				// moving between bins
				__flist_dequeue_bin(this, orig_nsize, oldbin);
				__flist_enqueue_bin(this, this->f_size, newbin);
				//
			}
			//
			return; // done
		}
	} 
	// the only reason we could be here is
	// a) previous was not free

	// check next block also
	neighbour = _ADJACENT(this);
	if (DH_ISFREE(neighbour)) {
		// forward neighbour free: dequeue
		__flist_dequeue_bin(neighbour, neighbour->f_size, -1);
		//
		_malloc_stats.m_blocks--;
		this->f_size += neighbour->f_size;
		neighbour = _ADJACENT(this);
	}

	/*
	 * Now that all the dust has settled, set this's footer.
	 */
	ft = HEAD_TO_DT(this);
	DT_SET(ft,this->f_size);

	if (_malloc_check_on) {
		if (!DT_ISBUSY(dt) || !DH_ISBUSY(neighbour)) {
			panic("free -- corrupt heap");
		}
	}

	if ((DT_ISARENA(dt)) && (DH_ISARENA(neighbour))) {
		cache_delmem(this);
		_malloc_stats.m_blocks--;
		return;
	}
	__FLIST_FIND_NBIN(this->f_size, newbin);
	__flist_enqueue_bin(this, this->f_size, newbin);
	return;
}

extern void malloc_abort(enum mcheck_status);

void *
_list_resize(void *ap, size_t n_bytes)
{
	Dhead *this;
	Flink *next;
	Dtail *dt;
	ssize_t sz;
	ssize_t nbytes;

	if (_malloc_check_on ) {
		enum mcheck_status mcheck_stat;
		if ((mcheck_stat = mprobe(ap)) > MCHECK_OK) {
			malloc_abort(mcheck_stat);
		}
	}

	this = (Dhead *)ap-1;

	if (!DH_ISBUSY(this)) {
		panic("realloc -- corrupt object");
	}

	sz = DH_LEN(this);		// this block size

#if defined(MALLOC_GUARD) && !defined(MALLOC_DEBUG)
	dt = HEAD_TO_DT(this);
	if (!DT_ISBUSY(dt) || (sz != DT_LEN(dt)))
		panic("realloc -- corrupt object");
#endif
	nbytes = UNITSIZE(n_bytes);
	if ((ssize_t)n_bytes < 0 || nbytes < 0)
		return NULL;

	/*
	 * can we grow in place?
	 */
	next = _ADJACENT(this);
	if (DH_ISFREE(next)) {
		/*
		 * If it won't buy us enough space, don't bother.
		 * If we _did_ coalesce, willy-nilly, we'd also have to
		 * SET_DTAIL here instead of waiting until just before
		 * returning our new buffer.
		 */
		if (sz + next->f_size < nbytes)
			return NULL;
		sz += next->f_size;
		this->d_size += next->f_size;	// preserves busy-bit
		_malloc_stats.m_freemem -= next->f_size;
		_malloc_stats.m_allocmem += next->f_size;
		// dequeue
		__flist_dequeue_bin(next, next->f_size, -1);
		//
	} else

	if (sz < nbytes)
		return NULL;

	/*
	 * If there's room to carve another free bucket, do so.
	 */
	sz -= nbytes;
	if (sz >= _min_free_list_size) {
		this->d_size -= sz;		// preserves busy-bit
		next = _ADJACENT(this);
		next->f_size = sz;
		dt = HEAD_TO_DT(next);
		DT_SET(dt,sz);
		{
			// find the right bin and drop the extra there
			int newbin;
			__FLIST_FIND_NBIN(next->f_size, newbin);
			__flist_enqueue_bin(next, next->f_size, newbin);
			//
		}
		_malloc_stats.m_freemem += sz;
		_malloc_stats.m_allocmem -= sz;
	}

	SET_DTAIL(this);

	assert(DH_ISBUSY(this));

	return (char *)(this+1);
}

/*-
 * collect garbage from the list.
 */
void
_list_gc()
{
#if 0
	Flink      *p;
	Flink *fp_list;
	int i;
	Flink *curflistptr;

	curflistptr = __malloc_getflistptr();
	for (i=0; i < __flist_nbins ; i++) {
		fp_list = &(curflistptr[i]);
		for (p=fp_list->f_next ; p != fp_list ; ) {
			if (p->size >= PSIZ) {
				p = carve(p);
			} else {
				p = p->next;
			}
		}
	}
#endif
}

int
malloc_opts(int cmd, void *arg2)
{
	int newval;
	int oldval;
	int oldsz;
	switch (cmd) {
	case MALLOC_VERIFY:
		_check_list(1);
		break;
	case MALLOC_VERIFY_ON:
		_malloc_check_on = (int) arg2;
		break;
	case MALLOC_STATS:
	{
		struct malloc_stats *ms = (struct malloc_stats *)arg2;
		memcpy(ms, &_malloc_stats, sizeof(struct malloc_stats));
		break;
	}
	case MALLOC_FREE_CHECK:
		_malloc_free_check = (int) arg2;
		break;
  case MALLOC_ARENA_SIZE:
  {
    int sz = (int) arg2;
    int rv;
    int pagesize = sysconf(_SC_PAGESIZE);

    if (sz == 0) {
      rv = _amblksiz;
      return(rv);
    }
    if (sz > 0x40000 || sz < pagesize) {
      errno = EINVAL;
      return -1;
    }

    rv = _amblksiz;
    _amblksiz = __ROUND(sz, (unsigned)pagesize);
    return rv;
  }
	case MALLOC_MONOTONIC_GROWTH:
		_malloc_monotonic_growth = (int) arg2;
		break;
	case MALLOC_MEMORY_HOLD:
		newval = (int)arg2;
		if (newval > 0) {
			// explicitly set both types of max-es to -1
			__core_cache_max_num = (unsigned)-1;
			__core_cache_max_sz = (unsigned)-1;
		}
		else {
			__core_cache_max_num = 0;
			__core_cache_max_sz = 0;
		}
		break;
	case MALLOC_ARENA_CACHE_MAXSZ:
		newval = (int) arg2;
		if (newval < 0) {
			errno = EINVAL;
			return -1;
		}
		oldval = __core_cache_max_sz;
		if (newval != oldval) {
			__core_cache_max_sz = newval;
			//__core_cache_adjust();
		}
		return(oldval);
	case MALLOC_ARENA_CACHE_MAXBLK:
		newval = (int) arg2;
		if (newval < 0) {
			errno = EINVAL;
			return -1;
		}
		oldval = __core_cache_max_num;
		if (newval != oldval) {
			__core_cache_max_num = newval;
			//__core_cache_adjust();
		}
		return(oldval);
	case MALLOC_ARENA_CACHE_FREE_NOW:
		newval = (int) arg2;
		if (newval < 0) {
			errno = EINVAL;
			return -1;
		}
		if (newval) {
			oldval = __core_cache_max_num;
			oldsz = __core_cache_max_sz;
			__core_cache_max_num = 0;
			__core_cache_max_sz = 0;
			__core_cache_adjust();
			__core_cache_max_num = oldval;
			__core_cache_max_sz = oldsz;
		}
		else {
			__core_cache_adjust();
		}
		break;
	default:
		errno = EINVAL;
		return -1;
	}
	return 0;
}

__SRCVERSION("dlist.c $Rev: 200568 $");
