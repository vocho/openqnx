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
  * Mark the heap using standard Garbage Collection techniques
  * Tracing done using stack marking with a mark stack and mark stack
  * overflow handling.
  *
  * see Garbage Collection by Richard Jones, ISBN 0 471 94148 4
  * chapter 4 for details.
  */
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "malloc-lib.h"
#include "mallocint.h"
#include <stdio.h>


int __malloc_outfd=-1;
extern int __pagesize;
/*
 * function: find_child
 *
 * scan a block of memory looking for pointers to heap buffers
 *
 */


int _malloc_hold_threads()
{
  ThreadCtl(_NTO_TCTL_THREADS_HOLD, 0);
  return(0);
}

int _malloc_release_threads()
{
  return(ThreadCtl(_NTO_TCTL_THREADS_CONT, 0));
}


static ulong_t
find_child(ulong_t start, ulong_t end, Dhead **child, arena_range_t *rp)
{
    ulong_t start_addr;
    ulong_t end_addr;
    ulong_t *addrp;

    /*
     * Scan through the block, looking for occurences of ptr
     * Only check whole dwords that are dword-aligned
     */

    start_addr = ((start + 3) & ~3);
    end_addr = ((end - (3 - (end & 3))) & ~3);
    for (addrp = (ulong_t *)start_addr; addrp <= (ulong_t *)end_addr; addrp++)
    {
        Dhead *dh;
        if (*addrp >= _malloc_start && *addrp <= _malloc_end) {

            /* interior pointer *are* allowed! */

            if ((dh = find_malloc_ptr((char *)(*addrp), rp)) != NULL) {
	        if( (dh->d_debug.flag & M_INUSE) != 0 )
	        {
	            char *data = (char *)(dh + 1);
	            Block *bp = (Block *)data;

	            /*
	             * If this segment is a Block, skip it
	             */
	            if (bp->magic != BLOCK_MAGIC) {
                        if (child != NULL) *child = dh;
                        /* return the address containing the child */
                        return (ulong_t)addrp;
              }
	        }
            }
        }
    }

    return 0;
}


/*
 *  Mark Stack routines
 */
mark_stack_t _mark_stack;

/*
 * Create
 */
static int
_mark_stack_create()
{
    char *mp;
	int newsize = __pagesize*32;
#ifdef MAP_ANON
    mp = mmap(0, newsize, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
#else
    int fd = open("/dev/zero", O_RDWR);
    mp = mmap(0, newsize, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
#endif
    if (mp != (char *)-1) {
        _mark_stack.entries = (mse_t *)mp;
        _mark_stack.size = newsize;
        _mark_stack.bot = _mark_stack.top = NULL;
        _mark_stack.n = (newsize / sizeof *_mark_stack.entries) - 1;
        _mark_stack.overflow = 0;
    }
    return _mark_stack.entries == NULL ? -1 : 0;
}

/*
 * Destroy
 */
static int
_mark_stack_destroy()
{
    if (_mark_stack.entries != NULL) {
        munmap(_mark_stack.entries, _mark_stack.size);
        _mark_stack.entries = NULL;
        _mark_stack.size = 0;
        _mark_stack.n = 0;
        _mark_stack.bot = _mark_stack.top = 0;
        _mark_stack.overflow = 0;
    }
    return 0;
}

/*
 * Grow
 */
static int
_mark_stack_grow()
{
    int newsize = 2 * _mark_stack.size;
    char *mp;
#ifdef MAP_ANON
    mp = mmap(0, newsize, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
#else
    int fd = open("/dev/zero", O_RDWR);
    mp = mmap(0, newsize, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);
#endif
    if (mp != (char *)-1) {
        munmap(_mark_stack.entries, _mark_stack.size);
        _mark_stack.entries = (mse_t *)mp;
        _mark_stack.size = newsize;
        _mark_stack.bot = _mark_stack.top = NULL;
        _mark_stack.n = (newsize / sizeof *_mark_stack.entries) - 1;
        _mark_stack.overflow = 0;
        return 0;
    }
    return -1;
}

/*
 * Empty test
 */
static int
_mark_stack_empty()
{
    return (_mark_stack.bot == _mark_stack.top);
}

/*
 * Push an element to the mark stack
 */
static int
_mark_stack_push(ulong_t start, ulong_t end)
{
    if ((_mark_stack.top - _mark_stack.bot) == -1 ||
        (_mark_stack.top - _mark_stack.bot) == _mark_stack.n) {
        _mark_stack.overflow = 1;
        return -1;
    } else {
        _mark_stack.entries[_mark_stack.top].mse_start = start;
        _mark_stack.entries[_mark_stack.top].mse_end = end;
        if (++_mark_stack.top > _mark_stack.n) {
            _mark_stack.top = 0;
        }
        return 0;
    }
}

static mse_t *
_mark_stack_pop()
{
    if (_mark_stack.bot != _mark_stack.top) {
        mse_t *bot = &_mark_stack.entries[_mark_stack.bot];
        if (++_mark_stack.bot > _mark_stack.n) _mark_stack.bot = 0;
        return bot;
    } else {
        return NULL;
    }
}

/*
 * Function:	find_marked_nodes()
 *
 * Purpose:	Traverse malloc chain, finding marked items that have
 *              unmarked children.
 *              Used for continuing after mark stack overflow
 *
 * Arguments:	todo
 *		0	- don't warn if problems detected in malloc chain
 *		1	- call malloc_warn if a problem is found in the chain
 *
 * Returns:	0	- malloc chain intact & no overflows
 *		other	- problems detected in malloc chain
 *
 * Narrative:
 *
 * Notes:	If todo is non-zero the malloc_warn function, when called
 *		may not return (i.e. it may exit)
 *
 */

static int
find_marked_nodes_helper(const char *func, const char *file, int line,
	int todo, arena_range_t *rp)
{
    Flink	*oldptr;
    Flink	*ptr;
    DebugInfo_t	*mptr;
    chain_t	*chain = rp->r_type == RANGE_ARENA
    			? &rp->un.r_arena->a_malloc_chain
    			: &rp->un.r_block->malloc_chain;

    oldptr = (Flink *)_malloc_start;
    for(ptr = chain->head; ; oldptr = ptr, ptr = ptr->f_next)
    {
	/*
	 * Since the malloc chain is a forward only chain, any
	 * pointer that we get should always be positioned in
	 * memory following the previous pointer.  If this is not
	 * so, we must have a corrupted chain.
	 */
	if( ptr )
	{
#if 0
	    if((ulong_t)ptr < (ulong_t)oldptr)
	    {
		malloc_errno = M_CODE_CHAIN_BROKE;
		if( todo )
		{
		    malloc_fatal(func,file,line,oldptr);
		}
		break;
	    }
#endif
	}
	else
	{
	    if( chain->tail && (oldptr != chain->tail) )
	    {
		/*
		 * This should never happen.  If it does, then
		 * we got a real problem.
		 */
		malloc_errno = M_CODE_NO_END;
		if( todo )
		{
		    malloc_fatal(func,file,line,oldptr);
		}
	    }
	    break;
	}

	/*
	 * verify that ptr is within the malloc region...
	 * since we started within the malloc chain this should never
	 * happen.
	 */
	if( ((ulong_t)ptr < (ulong_t)rp->r_start) ||
	    ((ulong_t)ptr > (ulong_t)rp->r_end) )
	{
	    malloc_errno = M_CODE_BAD_PTR;
	    if( todo )
	    {
		malloc_fatal(func,file,line,oldptr);
	    }
	    break;
	}
	mptr = &((Dhead *)ptr)->d_debug;

	/*
	 * verify magic flag is set
	 */

	if (!MAGIC_MATCH(mptr))
	{
	    malloc_errno = M_CODE_BAD_MAGIC;
	    if( todo )
	    {
		malloc_warning(func,file,line, NULL);
	    }
	    continue;
	}

	/*
	 * verify segments are correctly linked together
	 */
	if( (ptr->f_prev && (ptr->f_prev->f_next != ptr) ) ||
	    (ptr->f_next && (ptr->f_next->f_prev != ptr) ) )
	{
	    malloc_errno = M_CODE_BAD_CONNECT;
	    if( todo )
	    {
		malloc_warning(func,file,line,ptr);
	    }
	    continue;
	}

	/*
	 * If this segment is allocated
	 */
	if( (mptr->flag & M_INUSE) != 0 )
	{
	    char *data = (char *)((Dhead *)ptr + 1);
	    ulong_t start = (ulong_t)data;
	    ulong_t end = (ulong_t)(data + DH_ULEN(ptr));
	    Block *bp = (Block *)data;

	    /*
	     * If this segment is a Block, skip it
	     */
	    if (bp->magic == BLOCK_MAGIC) continue;

            if ( (mptr->flag & M_REFERENCED) != 0)  { /* it's marked */
                ulong_t child = start;
                Dhead *dh;
	        arena_range_t range;
	        /*
	         * scan through the block on dword-aligned
	         * boundaries looking for unmarked children
	         */
                do {
                    if ((child = find_child((ulong_t)child, (ulong_t)end, &dh, &range)) != NULL) {
                        child += 4;
                        if ((dh->d_debug.flag & M_REFERENCED) == 0) {

                            /* if we found an unmarked child, add the parent to
                             * the mark stack.
                             */
                            _mark_stack_push(start, end);
                            break;
                        }
                    }
                } while (child != 0);
	    }
	}
    } /* for(... */
    return 0;
}

int
find_marked_nodes(const char *func, const char *file, int line, int todo)
{
    /*
     * Don't init malloc or check the chain because this has
     * to be a valid pointer
     */
    Arena		*ap;
    Block 		* bp;
    Dtail		* dt;
    Dhead		* dp, *dpend;
    int			  nb;
    int			  cont = 1;
    arena_range_t	  range;

    /*
     * Go through malloc chains for each Arena looking for marked nodes
     */
	for (ap = __arenas.a_next; cont && ap != &__arenas; ap = ap->a_next) {
	if (ap->a_size < sizeof(*ap)) panic("a_size");

	dt = &ap->a_dtail;
	dp = (Dhead *)(ap + 1);
	dpend = (Dhead *)((char *)ap + ap->a_size - sizeof(Dtail));

        range.r_start = (char *)(dp + 1);
        range.r_end = (char *)dpend;
	range.r_type = RANGE_ARENA;
	range.un.r_arena = ap;

	find_marked_nodes_helper(func, file, line, todo, &range);
    }

    /*
     * Go through malloc chains for every Block in every Band
     */
    for (nb = 0; cont && nb < *__pnband; nb++)
    {
	Band  *band;

	band = __pBands[nb];

	/*
	 * For each Block on the allocated list
	 */
        for (bp = band->alist; cont && bp; bp = bp->next)
        {
	    int esize = bp->nbpe + SB_OVERHEAD();
	    range.r_start = (char *)bp + sizeof *bp + sizeof(Dhead);
	    range.r_end = range.r_start + esize * band->nalloc;
	    range.r_type = RANGE_BLOCK;
	    range.un.r_block = bp;

	    find_marked_nodes_helper(func, file, line, todo, &range);
        }

	/*
	 * For each Block on the depleted list
	 */
        for (bp = band->dlist; cont && bp; bp = bp->next)
        {
	    int esize = bp->nbpe + SB_OVERHEAD();
	    range.r_start = (char *)bp + sizeof *bp + sizeof(Dhead);
	    range.r_end = range.r_start + esize * band->nalloc;
	    range.r_type = RANGE_BLOCK;
	    range.un.r_block = bp;

	    find_marked_nodes_helper(func, file, line, todo, &range);
        }
    }

    return 0;
}

/*
 * Function:	malloc_mark()
 *
 * Purpose:	Traverse malloc chain, marking items that are still referenced
 *
 * Arguments:	todo
 *		0	- don't warn if problems detected in malloc chain
 *		1	- call malloc_warn if a problem is found in the chain
 *
 * Returns:	0	- malloc chain intact & no overflows
 *		other	- problems detected in malloc chain
 *
 * Narrative:
 *
 * Notes:	If todo is non-zero the malloc_warn function, when called
 *		may not return (i.e. it may exit)
 *
 */
extern int DBmalloc_mark(const char *file, int line, int todo);

int
malloc_mark(int todo)
{
    return( DBmalloc_mark((char *)NULL, 0, todo) );
}

int
DBmalloc_mark(const char *file, int line, int todo)
{
		char *temp=NULL;
		int status;
    ulong_t top = (ulong_t)&temp;
		ENTER();
		MALLOC_INIT();
		_malloc_hold_threads();
    status = DBFmalloc_mark("malloc_mark",file,line,top,todo);
		_malloc_release_threads();
		LEAVE();
		return(status);
}

#define MAX_ROOTS 512

static int
_mark(const char *func, const char *file, int line, int todo)
{
    int rtn = 0;
    mse_t *ep; mse_t node;
    do {
        if (_mark_stack_empty()) {
            _mark_stack_grow();  /* make some room to avoid more overflows */
            _mark_stack.overflow = 0;
            /* can overflow */
            if (find_marked_nodes(func, file, line, todo) > 0) {
                /* errors traversing chains
                     -- check todo to decide whether to continue? */
                rtn++;
            }
        }
        if (_mark_stack_empty()) return 0; /* paranoia */
        if ((ep = _mark_stack_pop()) != NULL) {
            Dhead *dh;
            ulong_t start, end;
            arena_range_t range;

            node = *ep;
            start = node.mse_start;
            end = node.mse_end;

            while (start < node.mse_end) {
                if ((start = find_child(start, end, &dh, &range)) != NULL) {
                    start += 4;
                    if ((dh->d_debug.flag & M_REFERENCED) == 0) {
                        dh->d_debug.flag |= M_REFERENCED;
                        dh->d_crc = LinkCRC((Flink *)dh);
                        _mark_stack_push((ulong_t)(dh+1), (ulong_t)(dh + 1) + DH_ULEN(dh));
                    }
                } else {
                    break;
                }
            }
        }
    } while (!_mark_stack_empty() || _mark_stack.overflow);
    return rtn;
}

/*
 * Must be called with the mutex locked!
 */
int
DBFmalloc_mark_all_helper(const char *func,const char *file,int line,
	int todo, int referenced, arena_range_t *rp, int *cont)
{
    void		* oldptr;
    Flink		* ptr;
    DebugInfo_t		* mptr;
    chain_t		* chain = rp->r_type == RANGE_ARENA
    				? &rp->un.r_arena->a_malloc_chain
    				: &rp->un.r_block->malloc_chain;
    int rtn = 0;

    oldptr = NULL;

    for(ptr = chain->head; ptr; oldptr = ptr, ptr = ptr->f_next)
    {
	/*
	 * Since the malloc chain is a forward only chain, any
	 * pointer that we get should always be positioned in
	 * memory following the previous pointer.  If this is not
	 * so, we must have a corrupted chain.
	 */
	if( ptr )
	{
#if 0
	    if((ulong_t)ptr < (ulong_t)oldptr )
	    {
		malloc_errno = M_CODE_CHAIN_BROKE;
		if( todo )
		{
		    malloc_fatal(func,file,line,oldptr);
		}
		rtn++;
		*cont = 0;
		break;
	    }
#endif
	}
	else
	{
	    if( chain->tail && (oldptr != chain->tail) )
	    {
		/*
		 * This should never happen.  If it does, then
		 * we got a real problem.
		 */
		malloc_errno = M_CODE_NO_END;
		if( todo )
		{
		    malloc_fatal(func,file,line,oldptr);
		}
		rtn++;
	    }
	    *cont = 0;
	    break;
	}

	/*
	 * verify that ptr is within the malloc region...
	 * since we started within the malloc chain this should never
	 * happen.
	 */
	if( ((ulong_t)ptr < (ulong_t)rp->r_start) ||
		((ulong_t)ptr > (ulong_t)rp->r_end) )
	{
	    malloc_errno = M_CODE_BAD_PTR;
	    if( todo )
	    {
		malloc_fatal(func,file,line,oldptr);
	    }
	    rtn++;
	    *cont = 0;
	    break;
	}

	mptr = &((Dhead *)ptr)->d_debug;
	/*
	 * verify magic flag is set
	 */

        if( ((Dhead *)ptr)->d_crc != LinkCRC(ptr))
        {
	    malloc_errno = M_CODE_BAD_CRC;
	    malloc_warning(func,file,line,(void *)ptr);
	    rtn++;
	    continue;
        }

	if (!MAGIC_MATCH(mptr))
	{
	    malloc_errno = M_CODE_BAD_MAGIC;
	    if( todo )
	    {
		malloc_warning(func,file,line, NULL);
	    }
	    rtn++;
	    continue;
	}


	/*
	 * verify segments are correctly linked together
	 */
	if( (ptr->f_prev && ptr->f_prev->f_next != ptr) ||
		(ptr->f_next && ptr->f_next->f_prev != ptr) )
	{
	    malloc_errno = M_CODE_BAD_CONNECT;
	    if( todo )
	    {
		malloc_warning(func,file,line,ptr);
	    }
	    rtn++;
	    continue;
	}

	/*
	 * If this segment is allocated
	 */
	if( (mptr->flag & M_INUSE) != 0 )
	{
	    /*
	     * verify no overflow of data area
	     */

	    // FIX:
	    if (_malloc_fill_area && _malloc_fill_area < mptr->hist_id)
	    {
	        if (todo)
	        {
	            if (malloc_check_fill(func,file,line,(Dhead *)ptr) != 0)
	            {
			rtn++;
			*cont = 0;
			break;
	            }
	        }
	    }
	}

        ((Dhead *)ptr)->d_debug.flag &= ~M_REFERENCED;
        ((Dhead *)ptr)->d_crc = LinkCRC(ptr);
    } /* for(... */
    return rtn;
}

/*
 * Must be called with the mutex locked!
 */
int
DBFmalloc_mark_all(const char *func,const char *file,int line,
int todo, int referenced)
{
    Arena		* ap;
    Block 		* bp;
    Dtail		* dt;
    Dhead		* dp, *dpend;
    int			  nb;
    int			  rtn = 0;
    int			  cont = 1;
    arena_range_t	  range;

    MALLOC_INIT();

    /*
     * Go through malloc chains for each Arena
     */
	for (ap = __arenas.a_next; cont && ap != &__arenas; ap = ap->a_next) {
	if (ap->a_size < sizeof(*ap)) panic("a_size");

	dt = &ap->a_dtail;
	dp = (Dhead *)(ap + 1);
	dpend = (Dhead *)((char *)ap + ap->a_size - sizeof(Dtail));

        range.r_start = (char *)dp;
        range.r_end = (char *)dpend;
	range.r_type = RANGE_ARENA;
	range.un.r_arena = ap;

	rtn += DBFmalloc_mark_all_helper(func,file,line,todo, referenced, &range, &cont);
    }

    /*
     * Go through malloc chains for every Block in every Band
     */
    for (nb = 0; cont && nb < *__pnband; nb++)
    {
	Band  *band;

	band = __pBands[nb];

	/*
	 * For each Block on the allocated list
	 */
        for (bp = band->alist; cont && bp; bp = bp->next)
        {
	    int esize = bp->nbpe + SB_OVERHEAD();
	    range.r_start = (char *)(bp + 1);
	    range.r_end = range.r_start + esize * band->nalloc;
	    range.r_type = RANGE_BLOCK;
	    range.un.r_block = bp;

	    rtn += DBFmalloc_mark_all_helper(func,file,line,todo,referenced,&range,&cont);
        }

	/*
	 * For each Block on the depleted list
	 */
        for (bp = band->dlist; cont && bp; bp = bp->next)
        {
	    int esize = bp->nbpe + SB_OVERHEAD();
	    range.r_start = (char *)(bp + 1);
	    range.r_end = range.r_start + esize * band->nalloc;
	    range.r_type = RANGE_BLOCK;
	    range.un.r_block = bp;

	    rtn += DBFmalloc_mark_all_helper(func,file,line,todo,referenced,&range,&cont);
        }
    }

    return(rtn);

} /* malloc_mark_all(... */

// we assume we are already locked when inside here
// so we DO not call ENTER / LEAVE
int
DBFmalloc_mark(const char *func, const char *file, int line,
	ulong_t top, int todo)
{
    int			  rtn = 0;
    mse_t 		 *roots;
    int			  nroots;
    Block 		* bp;
    int			  nb;

    int i = 0;
    unsigned int start, end;
		int count=0;
	int cache = 0;

    extern int		  _malloc_scan_start();
    extern int		  _malloc_scan_finish();

    MALLOC_INIT();

    /*
     * Mark all the nodes as unreferenced at the start.
     * Make sure the malloc chain is valid or we will be in for trouble
     */
    if ((rtn = DBFmalloc_mark_all("DBFmalloc_mark", file, line, todo, 0)) != 0) {
	return rtn;
    }

    if ((roots = alloca(MAX_ROOTS * sizeof *roots)) == NULL) {
        return 0;
    }
    mc_cache_clear();
    cache = mc_get_cache_size();
    mc_set_cache(0);
    nroots = _malloc_scan_start(roots, MAX_ROOTS, top);

    _mark_stack_create();

    /*
     * We go through each root of the root set.
     * We never mark more than one page of a root at a time to avoid
     * having its unmarked children overflowing the mark stack.
     */

    start = roots[i].mse_start;
    end = roots[i].mse_end;
    if (end > start + __pagesize) end = start + __pagesize;

    while (i < nroots) {
        _mark_stack_push(start, end);

		count++;
        /* _mark will find every heap node reachable from the root
         * that was pushed onto the mark stack.
         */
        rtn += _mark(func, file, line, todo);
        start += __pagesize;
        if (start >= roots[i].mse_end) {
            i++;
            start = roots[i].mse_start;
            end = roots[i].mse_end;
            if (end > start + __pagesize) end = start + __pagesize;
        } else {
            end += __pagesize;
            if (end > roots[i].mse_end) end = roots[i].mse_end;
        }
    }
    _mark_stack_destroy();

//    /*
//     * Go through every Block in every Band and mark it
//     *  - this is because it's an internal structure and shouldn't be
//     *    reported as a leak
//     */
//    for (nb = 0; nb < *__pnband; nb++)
//    {
//	Band  *band;
//
//	band = __pBands[nb];
//
//	/*
//	 * For each Block on the allocated list
//	 */
//        for (bp = band->alist; bp; bp = bp->next)
//        {
//	    Dhead *dh = (Dhead *)bp - 1;
//            dh->d_debug.flag |= M_REFERENCED;
//        }
//
//	/*
//	 * For each Block on the depleted list
//	 */
//        for (bp = band->dlist; bp; bp = bp->next)
//        {
//	    Dhead *dh = (Dhead *)bp - 1;
//            dh->d_debug.flag |= M_REFERENCED;
//        }
//    }

    _malloc_scan_finish();
    mc_set_cache(cache);
    return rtn;
} /* malloc_mark(... */

int
DBmalloc_dump_unreferenced(int fd, const char *file, int line, int todo)
{
    static char *title = "Dump of Unreferenced Heap Elements\n";
	int rtn = 0;
	char *temp=NULL;
    ulong_t top = (ulong_t)&temp;
    ENTER();
    MALLOC_INIT();
	_malloc_hold_threads();
	__malloc_outfd=fd;
	rtn = DBFmalloc_mark("malloc_check_unreferenced", file, line, top, todo);
	write(fd, title, strlen(title));
	malloc_list_items(fd, 3, 0L, 0L); /* unreferenced */
	__malloc_outfd=-1;
	_malloc_release_threads();
 	LEAVE();
	return rtn;
}
