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





/*
 * (c) Copyright 1990, 1991 Conor P. Cahill (uunet!virtech!cpcahil).  
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
#include <stdio.h>
#include <fcntl.h>
#include "malloc-lib.h"
#include "mallocint.h"


static int DBmalloc_chain_check (const char *file, int line, int todo);


/*
 * Function:	malloc_chain_check()
 *
 * Purpose:	to verify malloc chain is intact
 *
 * Arguments:	todo	- 0 - just check and return status
 *			  1 - call malloc_warn if error detected
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

static int DBmalloc_chain_check(const char *file,int line,int todo);

int
malloc_chain_check(int todo)
{
	int ret;
	ENTER();
	ret = DBmalloc_chain_check( (char *)NULL, 0, todo);
	LEAVE();
	return ret;
}

/*
 * Must be called with the mutex locked!
 */
static int
DBmalloc_chain_check(const char *file,int line,int todo)
{
	return( DBFmalloc_chain_check("malloc_chain_check",file,line,todo) );
}

/*
 * Must be called with the mutex locked!
 */
static int
DBFmalloc_chain_check_helper(const char *func,const char *file,int line,
	int todo, arena_range_t *rp, int *cont)
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
    } /* for(... */
    return rtn;
}

/*
 * Must be called with the mutex locked!
 */
int
DBFmalloc_chain_check(const char *func,const char *file,int line,int todo)
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

	rtn += DBFmalloc_chain_check_helper(func,file,line,todo, &range, &cont);
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

	    rtn += DBFmalloc_chain_check_helper(func,file,line,todo,&range,&cont);
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

	    rtn += DBFmalloc_chain_check_helper(func,file,line,todo,&range,&cont);
        }
    }

    return(rtn);

} /* malloc_chain_check(... */
