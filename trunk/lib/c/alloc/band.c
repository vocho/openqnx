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
 * Band ->  Block0 -> mem0->mem1->mem2...
 *           +->Block1->... 
 *
 * compilation flags:
 * -U NDEBUG     -- extensive checking of list consistency, invalid operations.
 * -D STATISTICS -- keep profile of list scans, #allocs, blocks alloced.
 */

#include <stdio.h>
#include <malloc.h>
#include <errno.h>
//Must use <> include for building libmalloc.so
#include <malloc-lib.h>
#include <limits.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>

#define SB_OVERHEAD()	__ROUND((sizeof(ListNode)), _MALLOC_ALIGN)

extern int _malloc_check_on;
void __prelocked_free(void *);
extern void __update_dlist_stats_nallocs(ssize_t, int);
extern void __update_dlist_stats_nfrees(ssize_t, int);
extern ssize_t _msize (void *ptr);
static void __preallocate_all_bands(void);
static Block *band_addblk(Band *p);

static void __band_validate_sizes()
{
	Band *p;
	int i;
	size_t extra;
	size_t totalmem=0;
	size_t largest_mem=0;
	size_t slurp;
	for (i=0; i < *__pnband; i++) {
		p = __pBands[i];
		// each band size must be a multiple of the alignment
		// else make it so
		p->nbpe = __ROUND(p->nbpe, _MALLOC_ALIGN);
		p->esize = p->nbpe + SB_OVERHEAD();
		slurp = sizeof(Block) + sizeof(ListNode);
		p->rem = __ROUND(slurp, p->esize) - slurp;
		extra = p->rem + sizeof(Block) + sizeof(ListNode) + sizeof(__BandArena);
		// find the effective total size of the band
		// this is to be a multiple of __ba_elem_sz
		totalmem = __ROUND(((p->esize * p->nalloc) + extra), __ba_elem_sz);
		if (totalmem > largest_mem) {
			largest_mem = totalmem;
		}
	}
	while (__ba_elem_sz < largest_mem)
		__ba_elem_sz <<= 1;
	return;
}

void __init_bands_new()
{
	size_t mem;
	size_t extra;
	Band *p;
	int i;
	size_t rem;
	__band_validate_sizes();
	for (i=0; i < *__pnband; i++) {
		p = __pBands[i];
		extra = p->rem + sizeof(Block) + sizeof(ListNode) + sizeof(__BandArena);
		mem = __ba_elem_sz - extra;
		// for each band re-calculate the number of each elements
		// this is based on the corrected size
		// and the largest effective band size
		p->nalloc = (mem/p->esize);
		rem = (mem - (p->nalloc*p->esize)); // overhead
		p->mem = sizeof(Block) + (p->esize * p->nalloc) + sizeof(ListNode);
		p->nalloc_stats = p->nalloc * SB_OVERHEAD() + rem;
	}
	__preallocate_all_bands();
	return;
}

static void __preallocate_all_bands()
{
	int i;
	unsigned total;
	Band *p;
	for (i=0; i < *__pnband; i++) {
		p = __pBands[i];
		total = 0;	
		// we overload slurp, by allowing users to specify 
		// a preallocate amount.
		while (total < p->slurp) {
			Block *b;
			b = band_addblk(p);
			if ( b == NULL )
				break;
			total += p->nalloc;
		}
	}
	return;
}

static void
checkListNode(Block *b, ListNode *lnp)
{
	/*
	 * If negative offset -- block is allocated.
	 */
	if (lnp->ln_offset < 0) {
		if ((Block *)((char *)lnp + lnp->ln_offset) != b) {
			panic("lnp offset");
		}
#if defined(MALLOC_GUARD) && !defined(MALLOC_DEBUG)
		if (lnp->ln_head.d_usize > b->nbpe) {
			panic("lnp usize");
		}
#endif
	} else {
		ListNode *l;

		for (l = b->head; l; l = l->ln_next) {
			if (lnp == l)
				break;
		}
		if (!l) {
			panic("listNode not on free list");
		}
	}
}

static void
checkBlock(Band *bp, Block *b)
{
	ListNode   *	lnp;
	int		i;
	char *		ptr;
	size_t		esize = bp->nbpe + SB_OVERHEAD();

	if (b->band != bp) {
		panic("block band pointer");
	}
	if ((b->band != bp) ||
	    (b->magic != BLOCK_MAGIC) ||
	    (bp->nbpe != b->nbpe)) {
		panic("bad band block");
	}

	for (i = 0, lnp = b->head; lnp; i++) {
		if (i > b->navail) {
			panic("too many ListNodes on free list");
		}
		if (((Block *)lnp <= b) || ((char *)lnp >= b->bend)) {
			panic("lnp address");
		}
		lnp = lnp->ln_next;
	}
	if (i < b->navail) {
		panic("too few ListNodes on free list");
	}
	
	ptr = (char *)(b+1);
	for (i = 0; i < bp->nalloc-1; i++, ptr += esize) {
		lnp = (ListNode *)ptr;
		checkListNode(b, lnp);
	}
}

void _check_bands(void)
{
	unsigned int	nbands;
	Band *		bp;
	Block *		blockp;

	for (nbands = 0; nbands < *__pnband; nbands++) {
		bp = __pBands[nbands];
		for (blockp = bp->alist; blockp; blockp = blockp->next) {
			checkBlock(bp, blockp);
		}
		for (blockp = bp->dlist; blockp; blockp = blockp->next) {
			checkBlock(bp, blockp);
		}
	}
}

#ifndef NDEBUG
# define CHECK_BLOCK(p,b)  do { if (_malloc_check_on) { checkBlock(p,b); } } while(0)
#else
#define CHECK_BLOCK(p,b)
#endif

#ifdef MALLOC_PC
# define BAND_PC 0xFEED0000
#endif

/*
 * band_addblk -- get a chunk.
 */
static Block *
band_addblk(Band *p)
{
	Block *	b=NULL, *nb;
	char *	ptr;
	int	i;
	__BandArena *ba;

	/*
	 * We allocate enough memory to hold:
	 *	the Block structure;
	 *	p->nalloc objects, including their ListNode headers;
	 *	a ssize_t guardian word
	 */

	ba = __get_barena();		
	if (ba == NULL)
		return NULL;
	b = (Block *)__BARENA_TO_BLOCK(ba);
	b = (Block *)((char *)b + p->rem);

#ifdef STATISTICS
	p->blk_size = p->mem;
#endif

	b->magic = BLOCK_MAGIC;
	b->navail = p->nalloc;
	b->band = p;
	b->nbpe = p->nbpe;

	ptr = (char *)(b+1);
	b->head = (ListNode *)ptr;
	b->bend = (char *)b + p->mem;
	for (i = 0; i < p->nalloc-1; i++, ptr += p->esize) {
		((ListNode *)ptr)->ln_next = (ListNode *)(ptr+p->esize);
		/* ((ListNode *)ptr)->ln_head.d_usize = 0; */
	}	
	if ((ptr + p->esize) > b->bend) {
		b->bend = ptr+p->esize-1;
	}
	((ListNode *)(ptr))->ln_next = 0;

	/*
	* Now set guardian to a value which looks like an allocated block.
	*/
	ptr += p->esize;
	((ListNode *)ptr)->ln_offset = (char *)b - ptr;
#ifdef MALLOC_GUARD
	if (!_malloc_check_on) {
		((ListNode *)ptr)->ln_head.d_usize = b->nbpe;
	} else {
		((ListNode *)ptr)->ln_head.d_usize = 0;
	}
#endif

	CHECK_BLOCK(p,b);

	b->prev = b->next = NULL;
#ifdef MALLOC_DEBUG
	b->malloc_chain.head = NULL;
	b->malloc_chain.tail = NULL;
#endif
	/*
	 *  To satisfy aligned requests, we may have to add new blocks even
	 *  though the alist is non-empty
	 */
	/* assert(!p->alist); */
	b->next = nb = p->alist;
	p->alist = b;
	if (nb)
		nb->prev = b;

#ifdef STATISTICS
	p->blk_alloced++;
#endif
	_malloc_stats.m_hblocks++;
	_malloc_stats.m_small_blocks += p->nalloc;
	_malloc_stats.m_small_freemem += (p->nalloc * p->nbpe);
	_malloc_stats.m_small_overhead += sizeof(Block) + sizeof(ListNode) + p->rem + p->nalloc_stats;
	assert(_malloc_stats.m_overhead + _malloc_stats.m_freemem + _malloc_stats.m_allocmem + _malloc_stats.m_small_overhead + _malloc_stats.m_small_allocmem + _malloc_stats.m_small_freemem == _malloc_stats.m_heapsize);
	return b;
}

static void *
_block_mem_malloc_align(Band *p, Block *b)
{
	ListNode  *x;
	CHECK_BLOCK(p,b);
	x = b->head;
	b->head = x->ln_next;
	return x;
}

static void *
_block_memalign(Band *p, Block *b, size_t alignment)
{
	ListNode  *x, *prev;
	Block *which;
	if (alignment > _MALLOC_ALIGN) {
		for (which = b; which != NULL; which = which->next) {
			CHECK_BLOCK(p,which);
			for (x = which->head, prev = NULL;
				x != NULL;
				prev = x, x = x->ln_next) {
				void *ptr = ((Dhead *)x) + 1;
				if (__TRUNC(ptr,alignment) == (ulong_t)ptr) {
					/* remove it */
					if (prev) {
						prev->ln_next = x->ln_next;
					} else {
						which->head = x->ln_next;
					}
					return x;
				}
			}
		}
		x = NULL;
	} else {
		CHECK_BLOCK(p,b);
		x = b->head;
		b->head = x->ln_next;
	}
	return x;
}

void *_band_get_aligned(Band *p, size_t alignment, size_t size)
{
	Block     *which;
	ListNode  *x;

	if (!__mallocsizes_inited) {
		__malloc_sizes_init();
	}
	assert(size <= p->nbpe);
	if (alignment != _MALLOC_ALIGN) {
		size_t esize = p->nbpe + SB_OVERHEAD();
		size_t	   dividend = (alignment / esize);
		assert(alignment >= _MALLOC_ALIGN);

		if (dividend > p->nalloc
			|| (alignment != _MALLOC_ALIGN 
      && dividend * esize != alignment)) { /* not an even multiple */
			errno = EINVAL;
			return NULL;
		}
	}

	if ((which = p->alist) != NULL) {
		if (alignment != _MALLOC_ALIGN) {
			x = (ListNode *)_block_memalign(p, which, alignment);
		}
		else {
			x = (ListNode *)_block_mem_malloc_align(p, which);
		}
		if (x != NULL) {
#ifdef STATISTICS
			p->alloc_counter++;
#endif
			x->ln_offset = (char *)which - (char *)x;
			assert(x->ln_offset < 0);
			assert(which->navail > 0);

			/*
		 	* If no more blocks, put this on the depleted-list
		 	*/
			if (--which->navail == 0) {
				Block        *b;

				assert(which->head == NULL);
				assert(which->prev == NULL);

				/*
			 	* Simple delete -- which is first.
			 	*/
				p->alist = b = which->next;
				if (b)
					b->prev = NULL;

				/*
			 	* insert into d(epleted)list
			 	*/
				which->next = b = p->dlist;
				p->dlist = which;
				if (b)
					b->prev = which;
			}
			_malloc_stats.m_small_allocmem += p->nbpe;
			_malloc_stats.m_small_freemem -= p->nbpe;

			return x+1;
		}
	}

	if ((which = band_addblk(p)) == NULL) {
		return 0; /* failed */
	}
	assert(which->navail);
	return _band_get_aligned(p, alignment, size);
}

void *_band_get(Band *p, size_t size)
{
	return _band_get_aligned(p, _MALLOC_ALIGN, size);
}

void
_band_rlse (Block *b, void *x)
{
	ListNode *lnp = (ListNode *)x - 1;
	Band *p = b->band;

	if (b->magic != BLOCK_MAGIC) {
		panic("BLOCK_MAGIC");
		return;
	}

	if ((char *)x + b->nbpe > b->bend) {
		panic("BLOCK ADDRESS");
		return;
	}

	/*
	 * It would be nice to have a check like _list_release's that
	 * this block isn't already on the free list, but a block on
	 * the free list would have a pointer value in the ListNode,
	 * not a negative offset, so _band_rlse wouldn't be involved.
	 * Instead, _list_release will crash because the header and tailer
	 * won't match.
	if (_malloc_free_check) {
		check_block_free_list(b, lnp)
	}
	 */

#if defined(MALLOC_GUARD) && !defined(MALLOC_DEBUG)
	/*
	 * Make sure caller hasn't overridden malloc area...
	 */
	_malloc_check_guard(x, (Dhead *)x - 1, b->nbpe);

	/*
	 * ... and that next ListNode is intact.
	 */
	{
		//size_t esize = b->nbpe + SB_OVERHEAD();
		ListNode *l = (ListNode *)((char *)lnp + p->esize);
		checkListNode(b, l);
	}
#endif
	CHECK_BLOCK(p,b);

#ifdef STATISTICS
		p->free_counter++;
#endif
	_malloc_stats.m_small_allocmem -= b->nbpe;
	_malloc_stats.m_small_freemem += b->nbpe;

	lnp->ln_next = b->head;
	b->head = lnp;
	b->navail++;

	/*
	 * If there is now exactly one available block in Block,
	 * move Block from d(epleted)list to a(lloc)list.
	 */
	if (1 == b->navail) {
		Block *t;

		if ((t = b->prev)) {
			t->next = b->next;
			b->prev = NULL;
		} else
			p->dlist = b->next;

		if (b->next)
			b->next->prev = t;

		b->next = t = p->alist;
		if (t)
			t->prev = b;
		p->alist = b;
	} 
	else

	/*
	 * If all the blocks in Block are now free, free the Block.
	 */
	if (b->navail == p->nalloc) {
		Block *t;
		__BandArena *ba;

		/*
		 * Take this off a(lloc)list.
		 */
		if ((t = b->prev))
			t->next = b->next;
		else
			p->alist = b->next;
		if (b->next)
			b->next->prev = t;

		//b->magic = 0;
		ba = __BLOCK_TO_BARENA(((char *)b-p->rem));
		__return_barena(ba);

		_malloc_stats.m_small_blocks -= p->nalloc;
		_malloc_stats.m_hblocks--;
		_malloc_stats.m_small_freemem -= (p->nalloc * p->nbpe);
		_malloc_stats.m_small_overhead -= sizeof(Block) + sizeof(ListNode) + p->rem + p->nalloc_stats;
#ifdef STATISTICS
		p->blk_freed++;
#endif
	}
}

__SRCVERSION("band.c $Rev: 167195 $");
