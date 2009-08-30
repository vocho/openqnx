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
#include "malloc-lib.h"
#include "mallocint.h"
#include "debug.h"
#include <dlfcn.h>
#include "malloc_cache.h"

int malloc_check_inited = 0;
uint64_t __malloc_clocktime(void);

#define MALLOC_CHECK_INIT() if (malloc_check_inited == 0) malloc_check_init()
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

void
malloc_check_init()
{
    extern ulong_t _end;
    ulong_t *sym = &_end;
    malloc_check_inited = 1;
    _malloc_start = (ulong_t)sym;
}

#if 0
static int
find_block_in_band(Block *block, Band *band)
{
    Block *b;
    for (b = band->alist; b; b = b->next) {
        if (b == block) return 1;
    }
    for (b = band->dlist; b; b = b->next) {
        if (b == block) return 1;
    }
    return 0;
}
#endif

/*
 * Function: find_malloc_chain(const char *cptr)
 *
 * Purpose: to return the malloc chain_t where the pointer should be
 *           No check is done on the pointer.
 * 
 * Arguments:	cptr	- pointer to verify
 *		range	- pointer to a structure, filled with location info
 *
 * Returns:	ptr	- a pointer to the start of the buffer header
 *		NULL	- otherwise
 */
Dhead *
find_malloc_range(const void *cptr, arena_range_t *range) {
	Dhead *dh;
	unsigned     v;
	Band       *band;
	int size;

	dh = (Dhead *)cptr-1;
	if (range) {
		size = _msize(cptr); // fix for PR53913 - cannot use dh->d_usize because it can be much lower than block size because

		NEWSELBAND(size,v,band);
		if (band != NULL) {
			Block *bp = (Block *)((char *)dh + dh->d_size);
			int esize = bp->nbpe + SB_OVERHEAD();

			range->r_start = (char *)(bp + 1);
			range->r_end = range->r_start + esize * band->nalloc; 
			range->r_type = RANGE_BLOCK;
			range->un.r_block = bp;
			range->r_ptr = dh;
		} else {
			Arena *ap;
			Dhead * dp=NULL, *dpend=NULL;

			ap = dh->arena;
			if (ap == NULL) {
				/* Go through each Arena */
				for (ap = __arenas.a_next; ap != &__arenas; ap = ap->a_next) {
					dp = (Dhead *)(ap + 1);
					dpend = (Dhead *)((char *)ap + ap->a_size - sizeof(Dtail));
					if ((char *)dh >= (char *)dp && (char *)dh < (char *)dpend) {
						break;
					}
				}
			} else {
				dp = (Dhead *)(ap + 1);
				dpend = (Dhead *)((char *)ap + ap->a_size - sizeof(Dtail));
			}

			range->r_start = (char *)dp;
			range->r_end = (char *)dpend;
			range->r_type = RANGE_ARENA;
			range->un.r_arena = ap;
			range->r_ptr = dh;
		}
	}
	return dh;
}



extern int _malloc_initialized;

/*
 * Function:	find_malloc_ptr(const char *cptr, arena_range_t *range)
 *
 * Purpose:	to verify address is within malloc area,
 *                 whether it's a Block or an Arena
 *
 * Arguments:	cptr	- pointer to verify
 *		range	- pointer to a structure, filled with location info
 *
 * Returns:	ptr	- a pointer to the start of the buffer header
 *		NULL	- otherwise
 *
 * Narrative:
 *   IF pointer is >= malloc area start AND <= malloc area end
 *      return TRUE
 *   ELSE
 *      return FALSE
 *
 * Mod History:	
 *   90/01/24	cpcahil		Initial revision.
 */

void *
find_malloc_ptr(const void *ptr, arena_range_t *range) {
    MALLOC_CHECK_INIT();
    if (ptr==0 || !_malloc_initialized) return NULL;
		{
				// activate cache optimization
				Dhead	* dh  = mc_cache_get(ptr, range);
				if (dh!=NULL) {
					if (dh==CACHED_NULL) {
						return NULL;
					}
					// pointer found in recent cache
					return dh;
				} 
			
			
			{
				Block	*bp;
				Dtail	*dt;
				Arena	*rtn = NULL;
				Arena	*ap;
				Dhead	*dp, *dpend;
				int 	 nb;

				/*
				 * Go through every Block in every Band
				 * NB.  Only necessary if the pointer is corrupted, but
				 *      should take precedence over Arena for safety's sake
				 */
				for (nb = 0; rtn == NULL && nb < *__pnband; nb++) {
					Band  *	band = __pBands[nb];
					ulong_t lptr = (ulong_t)ptr;
					Block * lists[]={band->alist,band->dlist};
					int ilist;
					for (ilist = 0; ilist < 2 && rtn==NULL; ++ilist) {
						/* For each Block on the allocated list and then on depleted list */
						for (bp = lists[ilist]; bp; bp = bp->next)
						{
							int esize, bsize;
							ulong_t bp1 = (ulong_t)(bp+1);
							if (lptr < bp1) continue;
							esize = bp->nbpe + SB_OVERHEAD();
							bsize = esize * band->nalloc;
							if (lptr < bp1 + bsize)
							{
								/*
								 * Calculate the small buffer position
								 */
								int diff = lptr - bp1;
								int i = diff / esize;
								Dhead *dh = (Dhead *)((char *)bp1 + i*esize);

								if (range) {
									range->r_start = (char *)bp1;
									range->r_end = range->r_start + bsize; 
									range->r_type = RANGE_BLOCK;
									range->un.r_block = bp;
									range->r_ptr = dh;
								}

								rtn = (void *)dh;
								break;
							}
						}
					}
				}
	
				

				for (ap = __arenas.a_next; rtn == NULL && ap != &__arenas; ap = ap->a_next) {
					Dhead	* dh = (Dhead *)ptr - 1;
					if (ap->a_size < sizeof(*ap)) panic("a_size");
					dt = &ap->a_dtail;
					dp = (Dhead *)(ap + 1);
					dpend = (Dhead *)((char *)ap + ap->a_size - sizeof(Dtail));

					/* this check isn't completely accurate, since we could be
						 in a Block inside the given Arena */
					if ((char *)dh >= (char *)dp && (char *)dh < (char *)dpend) {

						DebugInfo_t *dbgp;
						Dhead	*dha = dh;
						dha = (Dhead *)__TRUNC(dh, _MALLOC_ALIGN);
						dbgp = &dha->d_debug;

						/* See PR 43552
						 *   It happens that the magic pattern was found by error
						 *   This _temporary__ until we find a better fix
						 *   and this will allow the MME tester folks to continue there tests.
						 */
						if (MAGIC_MATCH(dbgp))
						{
							if (range) {
								range->r_start = (char *)dp;
								range->r_end = (char *)dpend;
								range->r_type = RANGE_ARENA;
								range->un.r_arena = ap;
							}
							rtn = (void *)dha;
						}
						else
						{
							Flink	*flink;
							char	*data;
							/*
							 * It may be an interior pointer, search the 
							 * Arena for the big buffer, and double-check for
							 * a Block in the Arena holding a matching small buffer
							 *
							 * NB. It could also be a small buffer for which the
							 *     mutator has corrupted the header.

							 * NBB. Blocks won't appear in the allocation chains
							 *     because the underlying band implementation
							 *     doesn't call up to us.
							 */

							flink = ap->a_malloc_chain.head;

							/*
							 * Look on the malloc chain for the Arena, there
							 * could be an allocated big buffer, or a Block
							 */
							for (flink = ap->a_malloc_chain.head;
									flink != NULL; 
									flink = flink->f_next)
							{
								data = (char *)((Dhead *)flink + 1);
								if ( ((ulong_t)flink < (ulong_t)ptr)
										&& ((ulong_t)(data+_msize(data)) > (ulong_t)ptr) ) {
									rtn = (void *)flink;
									break;
								}
							}
						}
						break;
					}
				}
				if (rtn == NULL)
				{
					Flink	*flink;
					char	*data;
					int i;
					/* Not allocated, search the free list */
					for (i=0; i < __flist_nbins ; i++) {
						Flink *fp_list;
						Flink *curflistptr;
						curflistptr = __malloc_getflistptr();
						fp_list = &(curflistptr[i]);
						for (flink = fp_list->f_next;
								flink != fp_list; 
								flink = flink->f_next)
						{
							data = (char *)((Dhead *)flink + 1);
							if ( ((ulong_t)flink < (ulong_t)ptr)
									&& ((ulong_t)(data+_msize(data)) > (ulong_t)ptr) ) {
								rtn = (void *)flink;
								break;
							}
						}
					}
				}
				

				if (range)
					range->r_ptr = (void *)rtn;
	
				if (rtn!=NULL) {
				  mc_cache_update(ptr, (Dhead*) rtn);	
				  return rtn;
				}
			}
		}
		mc_cache_update(ptr, (Dhead *) NULL);
		return NULL;
}

char *
_mptr(const char *ptr)
{
    arena_range_t range;
    Dhead *dh = (Dhead *)find_malloc_ptr(ptr, &range);
    return dh ? (void *)(dh + 1) : NULL;
}

/*
 * Function:	find_malloc_area(const char *cptr, arena_range_t *range)
 *
 * Purpose:	to verify address is within malloc area,
 *                 whether it's a Block or an Arena
 *
 * Arguments:	ptr	- pointer to verify
 *
 * Returns:	TRUE	- if it's a valid pointer
 *		FALSE	- otherwise
 *
 * Narrative:
 *   IF pointer is >= malloc area start AND <= malloc area end
 *      return TRUE
 *   ELSE
 *      return FALSE
 *
 * Mod History:	
 *   90/01/24	cpcahil		Initial revision.
 */
int
find_malloc_area(const void *ptr, arena_range_t *range)
{
    if (find_malloc_ptr(ptr, range) != NULL) return 1;
    return 0;
}

/*
 * Function:	malloc_check_ptr()
 *
 * Arguments:	func	- name of function calling this routine
 *		file	- name of file for caller
 *		line	- line number in the caller
 *		ptr	- pointer to area to check
 *
 * Purpose:	to verify that if str is within the malloc arena and in use.
 *
 * Returns:	Any error
 *
 * Narrative:
 *   IF pointer is within malloc arena
 *      do all header checks
 *   return 
 *
 * Mod History:	
 *   01/02/26	furr		Initial revision.
 */
int
malloc_check_ptr(const char *func, const char *file, int line, const char *cptr)
{
    register DebugInfo_t	* ptr;
    Flink			* flink;
    arena_range_t		  range; 
    int				  status = 0;

    /*
     * initialize the malloc sub-system.
     */
    ENTER();
    MALLOC_INIT();

    if( cptr == NULL )
    {
	      status = M_CODE_BAD_PTR;
        goto err;
    }


    /*
     * verify that cptr is within the malloc region...
     * and that it is aligned properly
     */
    if((cptr != (void *)__TRUNC((ulong_t)cptr, _MALLOC_ALIGN)) ||
       (!find_malloc_area(cptr, &range)))
    {
		status = malloc_errno = M_CODE_BAD_PTR;
		malloc_warning(func,file,line,(void *)NULL);
		goto err;
    }

    flink = (Flink *)((Dhead *)cptr - 1);
    ptr = &((Dhead *)flink)->d_debug;

    if( range.r_ptr && (Dhead *)flink != range.r_ptr )
    {
		status = malloc_errno = M_CODE_PTR_INSIDE;
		malloc_warning(func,file,line,(void *)NULL);
		goto err;
    }

    /*
     * check the magic number 
     */	
    if (!MAGIC_MATCH(ptr))
    {
		status = malloc_errno = M_CODE_BAD_MAGIC;
		malloc_warning(func,file,line,(void *)NULL);
		goto err;
    }

    /*
     * if this segment is not flagged as being in use
     */
    if( ! (ptr->flag & M_INUSE) )
    {
		status = malloc_errno = M_CODE_NOT_INUSE;
		malloc_warning(func,file,line,flink);
		goto err;
    }

    if( ((Dhead *)flink)->d_crc != LinkCRC(flink))
    {
		status = malloc_errno = M_CODE_BAD_CRC;
		malloc_warning(func,file,line,(void *)flink);
		goto err;
    }

    /*
     * check to see that the pointers are still connected
     */
    {
	Flink *prev = flink->f_prev;
	Flink *next = flink->f_next;
 	if( (prev && (prev->f_next != flink) ) ||
       		(next && (next->f_prev != flink) ) )
	{
	    status = malloc_errno = M_CODE_BAD_CONNECT;
	    malloc_warning(func,file,line,flink);
	    goto err;
	}
    }

    /*
     * Here's a new one!  Even fill checking won't help if you
     * run into the next block's header, so we can check the
     * next contiguous block to see if it's all right.
     */
    /* malloc_check_neighbors(func,file,line,&range,(Dhead *)flink); */

    /*
     * if we are filling the area with fill bytes, check to see if the
     * program has gone beyond the end of the assigned area
     */
    if( _malloc_fill_area && _malloc_fill_area < ptr->hist_id )
    {
	if ( malloc_check_fill(func,file,line,(Dhead *)flink) < 0) {
	    status = M_CODE_OVERRUN;
	}
    }
    if (malloc_check_guard(func, file, line, (Dhead *)flink) < 0) {
	    status = M_CODE_OVERRUN;
    }

err:
    LEAVE();
    return status;
}


/*
 * Function:	malloc_check_str()
 *
 * Arguments:	func	- name of function calling this routine
 *		str	- pointer to area to check
 *
 * Purpose:	to verify that if str is within the malloc arena, the data 
 *		it points to does not extend beyond the applicable region.
 *
 * Returns:	Nothing of any use (function is void).
 *
 * Narrative:
 *   IF pointer is within malloc arena
 *      determin length of string
 *      call malloc_verify() to verify data is withing applicable region
 *   return 
 *
 * Mod History:	
 *   90/01/24	cpcahil		Initial revision.
 *   90/01/29	cpcahil		Added code to ignore recursive calls.
 */
void
malloc_check_str(const char *func, const char *file, int line, const char *str)
{
    static int		 layers;
    int			 layer;
    register const char	*s;
    arena_range_t	 range;

    MALLOC_INIT();

    /*
     * if we are already in the malloc library somewhere, don't check
     * things again.
     */
    if( in_malloc() || !malloc_verify_access )
    {
	return;
    }

    MALLOC_CHECK_INIT();

    if( ((layer = layers++) == 0) &&  find_malloc_area((void *)str, &range) )
    {
	for( s=str; *s; s++)
	{
	}
		
	malloc_verify(func,file,line,&range,str,s-str+1);
    }

    layers--;
}

/*
 * Function:	malloc_check_strn()
 *
 * Arguments:	func	- name of function calling this routine
 *		str	- pointer to area to check
 * 		len     - max length of string
 *
 * Purpose:	to verify that if str is within the malloc arena, the data 
 *		it points to does not extend beyond the applicable region.
 *
 * Returns:	Nothing of any use (function is void).
 *
 * Narrative:
 *   IF pointer is within malloc arena
 *      determin length of string
 *      call malloc_verify() to verify data is withing applicable region
 *   return 
 *
 * Mod History:	
 *   90/01/24	cpcahil		Initial revision.
 *   90/01/29	cpcahil		Added code to ignore recursive calls.
 *   90/08/29	cpcahil		added length (for strn* functions)
 */
void
malloc_check_strn(const char *func, const char *file, int line, const char *str, int len)
{
    register int	  i;
    static int		  layers;
    int			  layer;
    register const char	* s;
    arena_range_t	  range;

    MALLOC_INIT();

    /*
     * if we are already in the malloc library somewhere, don't check
     * things again.
     */
    if( in_malloc() || !malloc_verify_access )
    {
	return;
    }
    MALLOC_CHECK_INIT();

    if( ((layer = layers++) == 0) &&  find_malloc_area(str, &range) )
    {
	for( s=str,i=0; (i < len) && *(char *)s; i++,s++)
	{
	}

	/*
	 * if we found a null byte before len, add one to s so
	 * that we ensure that the null is counted in the bytes to
	 * check.
	 */
	if( i < len )
	{
	    s++;
	}
	malloc_verify(func,file,line,&range,str,s-str);
    }

    layers--;
}

/*
 * Function:	malloc_check_data()
 *
 * Arguments:	func	- name of function calling this routine
 *		ptr	- pointer to area to check
 *		len 	- length to verify
 *
 * Purpose:	to verify that if ptr is within the malloc arena, the data 
 *		it points to does not extend beyond the applicable region.
 *
 * Returns:	Nothing of any use (function is void).
 *
 * Narrative:
 *   IF pointer is within malloc arena
 *      call malloc_verify() to verify data is withing applicable region
 *   return 
 *
 * Mod History:	
 *   90/01/24	cpcahil		Initial revision.
 *   90/01/29	cpcahil		Added code to ignore recursive calls.
 */
void
malloc_check_data(const char *func, const char *file, int line,
	const void *ptr, size_t len)
{
    static int		  layers;
    int			  layer;
    arena_range_t	  range;

    MALLOC_INIT();

    /*
     * if we are already in the malloc library somewhere, don't check
     * things again.
     */
    if( in_malloc() || !malloc_verify_access )
    {
	return;
    }
    MALLOC_CHECK_INIT();

    if( (layer = layers++) == 0 )
    {
	DEBUG3(40,"malloc_check_data(%s,0x%x,%d) called...",
		func,ptr,len);
	if( find_malloc_area(ptr, &range) )
	{
	    DEBUG0(10,"pointer in malloc arena, verifying...");
	    malloc_verify(func,file,line,&range, ptr,len);
	}
    }

    layers--;
}

/*
 * Function:	malloc_verify()
 *
 * Arguments:	func	- name of function calling the malloc check routines
 *		ptr	- pointer to area to check
 *		len 	- length to verify
 *
 * Purpose:	to verify that the data ptr points to does not extend beyond
 *		the applicable malloc region.  This function is only called 
 *		if it has been determined that ptr points into the malloc arena.
 *
 * Returns:	Nothing of any use (function is void).
 *
 * Narrative:
 *
 * Mod History:	
 *   90/01/24	cpcahil		Initial revision.
 */
void
malloc_verify(const char *func, const char *file, int line,
	const arena_range_t *range, const void *ptr, size_t len)
{
    Flink	*flink;
    DebugInfo_t	*mptr;
    char	*data;
	
    DEBUG5(40,"malloc_verify(%s, %s, %s, 0x%x,%d) called...",
		func, file, line, ptr, len);
    /*
     * Find the malloc block that includes this pointer
     */
    if (range->r_ptr)
    {
        flink = (Flink *)range->r_ptr;
        data = (char *)((Dhead *)flink + 1);
    }
    else
    {
        if (range->r_type == RANGE_BLOCK) {
	    int blen = range->un.r_block->nbpe;
	    for (flink = range->un.r_block->malloc_chain.head;
	        flink != NULL; 
	        flink = flink->f_next)
	    {
                data = (char *)((Dhead *)flink + 1);
                if ( ((ulong_t)flink < (ulong_t)ptr)
	            && ((ulong_t)(data+blen) > (ulong_t)ptr) ) {
                    break;
                }
            }
        } else {
            /*
             * Look on the malloc chain for the Arena, there
             * could be an allocated big buffer, or a Block
             */
	    for (flink = range->un.r_arena->a_malloc_chain.head;
	        flink != NULL; 
	        flink = flink->f_next)
	    {
                data = (char *)((Dhead *)flink + 1);
                if ( ((ulong_t)flink < (ulong_t)ptr)
	            && ((ulong_t)(data+_msize(data)) > (ulong_t)ptr) ) {
                    break;
                }
            }
        }
    }

    /*
     * if ptr was not in a malloc block, it must be part of
     *    some direct sbrk() stuff, so just return.
     */
    if( ! flink )
    {
	DEBUG1(10,"ptr (0x%x) not found in malloc search", ptr);
	return;
    }
    mptr = &((Dhead *)flink)->d_debug;
	
    /*
      * Now we have a valid malloc block that contains the indicated
     * pointer.  We must verify that it is withing the requested block
     * size (as opposed to the real block size which is rounded up to
     * allow for correct alignment).
     */

    if (!malloc_verify_access || malloc_verify_access >= mptr->hist_id)
	return;

    DEBUG4(60,"Checking  0x%x-0x%x, 0x%x-0x%x",
		ptr, ((char *)ptr)+len, data, data+DH_ULEN(flink));

    if( ! (((Dhead *)flink)->d_debug.flag & M_INUSE) )
    {
	malloc_errno = M_CODE_REUSE;
	malloc_warning(func,file,line,flink);
	return;
    }

    if( ((Dhead *)flink)->d_crc != LinkCRC(flink))
    {
	malloc_errno = M_CODE_BAD_CRC;
	malloc_warning(func,file,line,(void *)flink);
	return;
    }
	
    if(    (ptr < (void *)data)
       || ((((char *)ptr)) >= (char *)(data+DH_ULEN(flink)))
  	   || ((((char *)ptr)+len) > (char *)(data+DH_ULEN(flink))) )
    {
	DEBUG4(0,"pointer not within region 0x%x-0x%x, 0x%x-0x%x",
		ptr, ((char *)ptr)+len, data, data+DH_ULEN(flink));

	malloc_errno = M_CODE_OUTOF_BOUNDS;
	malloc_warning(func,file,line,flink);
    }

    return;
}

#if 0
int
malloc_check_neighbors(const char *func, const char *file, int line, 
    arena_range_t *rp, Dhead *dh)
{
    char *np = (char *)(dh+1) + _msize((void *)(dh+1));

    if (rp->r_type == RANGE_ARENA) np += sizeof(Dtail);

    if (np >= rp->r_start && np < rp->r_end) {
        Flink *flink = (Flink *)np;
        if (DH_LEN(dh) < 0 && DH_LEN(flink) > 0)
        { /* Block alloced buffer followed by a free one */
            /* Do something!  It should point to the next block element */
            ListNode *next = ((ListNode *)flink)->ln_next;

            if ((ulong_t)next < (ulong_t)rp->r_start || (ulong_t)next >= (ulong_t)rp->r_end)
            {
    	        malloc_errno = M_CODE_OVERRUN;
    	        malloc_warning(func,file,line,dh);
	        malloc_errno = M_CODE_BAD_CRC;
	        malloc_warning(func,file,line,(void *)flink);
	        return -1;
            }
        }
#if 0
        else if( ((Dhead *)flink)->d_crc != LinkCRC(flink))
        {
            /* This might only be correct if the buffer is in use */
    	    malloc_errno = M_CODE_OVERRUN;
    	    malloc_warning(func,file,line,dh);
	    malloc_errno = M_CODE_BAD_CRC;
	    malloc_warning(func,file,line,(void *)flink);
	    return -1;
		}
#endif
    }
#if 0 /* not working correctly - over-aggressive */
    if (rp->r_type == RANGE_BLOCK 
        || rp->r_type == RANGE_ARENA)
    {
	Dtail *dt = NULL;
	Flink *np = NULL;
        if (rp->r_type == RANGE_BLOCK) 
        {
            Block *b = rp->un.r_block;
	    int esize = b->nbpe + SB_OVERHEAD();
	    np = (Flink *)((char *)dh - esize);
        }
        else if (rp->r_type == RANGE_ARENA)
        {
            dt = (Dtail *)dh - 1;
            np = (Flink *)((char *)dt - DT_LEN(dt));
        }
        if ((char *)np >= rp->r_start && (char *)np < rp->r_end) 
        {
            arena_range_t **rpp = &rp; /* gcc incorrect-ness? */
            Dhead *nh = (Dhead *)np;
            DebugInfo_t *ptr = &dh->d_debug;
            DebugInfo_t *nptr = &nh->d_debug;
            if( ((Dhead *)np)->d_crc != LinkCRC(np))
            {
    	        malloc_errno = M_CODE_OVERRUN;
    	        malloc_warning(func,file,line,dh);
	        malloc_errno = M_CODE_BAD_CRC;
	        malloc_warning(func,file,line,(void *)np);
	        return -1;
            }
            if( _malloc_fill_area && _malloc_fill_area < nptr->hist_id 
                && (nptr->flag & M_INUSE) != 0)
            {
	        if (malloc_check_fill(func,file,line,np) != 0)
	            return -1;
            }
            if (_malloc_fill_area && _malloc_fill_area < ptr->hist_id
                && dt != NULL)
            {
                if (DT_LEN(dt) != DH_LEN(np))
                {
    	            malloc_errno = M_CODE_OVERRUN;
    	            malloc_warning(func,file,line,dh);
	            malloc_errno = M_CODE_OVERRUN;
	            malloc_warning(func,file,line,(void *)np);
	            return -1;
                }
            }
        }
    }
#endif
    return 0;
}
#endif

int
malloc_check_guard(const char *func, const char *file, int line, Dhead *dh)
{
    char *ptr = (char *)(dh+1);
    if (dh->d_size < 0) {
        Block *b = (Block *)((char *)dh + dh->d_size);
        ListNode *lnp = (ListNode *)ptr - 1;
        size_t esize = b->nbpe + SB_OVERHEAD();
        ListNode *l = (ListNode *)((char *)lnp + esize);
        Block *nb;
				if (l->ln_offset < 0) {
        	nb = (Block *)((char *)l + l->ln_offset);
					if (nb != b) {
    	   		malloc_errno = M_CODE_OVERRUN;
    	   		malloc_warning(func,file,line,dh);
    	   		return -1;
					}
         	if (l->ln_head.d_usize > b->nbpe) {
    	   		malloc_errno = M_CODE_OVERRUN;
    	   		malloc_warning(func,file,line,dh);
    	   		return -1;
         	}
				}
				else {
    			ListNode *nl;
    			for (nl = b->head; nl; nl = nl->ln_next) {
      			if (l == nl)
        			break;
    			}
					if (!nl) {
    	   		malloc_errno = M_CODE_OVERRUN;
    	   		malloc_warning(func,file,line,dh);
    	   		return -1;
					}
					if ((l->ln_next) && 
              ((Block *)l->ln_next <= b) || 
               ((char *)l->ln_next >= b->bend)) {
    	   		malloc_errno = M_CODE_OVERRUN;
    	   		malloc_warning(func,file,line,dh);
    	   		return -1;
					}
				}
    } else {
        Dtail *dt = HEAD_TO_DT(dh);
        int busy = DT_ISBUSY(dt);
        size_t len = DT_LEN(dt);
        if (!busy || len != DH_LEN(dh)) {
					int x;
					size_t y;
					x = busy;
					y = len;
    	    malloc_errno = M_CODE_OVERRUN;
    	    malloc_warning(func,file,line,dh);
					x = y;
    	    return -1;
        }
    }
    return 0;
}

int
malloc_check_fill(const char *func, const char *file, int line, Dhead *dh)
{
    int i = _msize((void *)(dh+1)) - DH_ULEN(dh);

    if (i) {
        char *gp = (char *)(dh + 1) + DH_ULEN(dh);
        if (i <= 0) {
    	    malloc_errno = M_CODE_OUTOF_BOUNDS;
    	    malloc_fatal(func,file,line,dh);
    	    return -1;
        }
        while (i > 0) 
        {
            if (*gp != (char)i)
            {
    	        malloc_errno = M_CODE_OVERRUN;
    	        malloc_warning(func,file,line,dh);
    	        return -1;
    	    }
	    gp++, i--;
        }
    }
    return 0;
}

void print_current_arenas(int fd)
{
	char buf[256];
    Arena	*ap;
	sprintf(buf, "Currently active Arenas\n");
	write(fd, buf, strlen(buf));
	for (ap = __arenas.a_next; ap != &__arenas; ap = ap->a_next) {
		sprintf(buf, "Arena 0x%08x, 0x%08x, size 0x%08x\n",
		(unsigned int)(Dhead *)(ap + 1), 
		(unsigned int)((Dhead *)((char *)ap + ap->a_size - sizeof(Dtail))),
		ap->a_size);
		write(fd, buf, strlen(buf));
	}
	return;
}

/*
 * $Log$
 * Revision 1.24  2007/04/24 15:44:27  shiv
 * PR:29730
 * CI:cburgess
 * CI:xtang
 *
 * this is part of work order WO790334 for IGT. Included enhancements
 * for configurability of the memory allocator.  Includes matching changes for
 * lib/c/alloc and lib/malloc as usual. This is to bring head in line
 * with the work committed to the branch.
 *
 * Revision 1.23  2006/12/22 17:38:48  seanb
 *
 * - Add more magic numbers to reduce chance of false
 *   positive.  Still not perfect so PR left open.
 * PR:43552
 * CI:ELaskavaia
 *
 * Revision 1.22  2006/12/21 21:14:56  alain
 *
 * PR:43552
 * CI:alain
 *
 * Revert a previous patch, it turns out to be lethal, Thanks Elena for finding this
 *
 * Revision 1.21  2006/12/14 21:25:13  alain
 *
 * PR:43552
 * CI:seanb
 * CI:cburgess
 *
 * The testers were finding cases were find erroneous MAGIC patterns
 * See the PR for more decriptions.
 *
 * Revision 1.20  2006/09/28 19:05:56  alain
 * PR:41782
 * CI: shiv@qnx.com
 * CI: cburgess@qnx.com
 *
 * Commiting the work done for IGT-CE on the head.
 *
 * Revision 1.19.2.2  2006/09/20 18:25:33  alain
 *
 * Oops! we did an unaligned access of the memory in find_malloc_ptr().
 * This will cause sigbus on pretty much all platforms except x86 and maybe ppc.
 *
 * Revision 1.19.2.1  2006/07/19 19:51:57  alain
 *
 *
 * We now have a new function call find_malloc_range() that does not do any
 * checking but rather (Dhead *)ptr - 1.  This is much faster in order of magnitue
 * of 10.  The bad things is that if the pointer is bad we will not be able to detect
 * it.  But to palliate there is an optiuon MALLOC_CHECK_ALLOC_PARAM that will revert
 * back to the old behaviour.
 * Now free() and realloc() only use the slow when MALLOC_CHEK_ALLOC_PARAM is set
 * malloc() always use the find_malloc_range() we assume that if it is succesfull not
 * need to check again.
 *
 * Things did not work for IGT since they use signal for IPC and the system call in QNX
 * are not restartable.  We need to make sure when the thread is created that it will ignore
 * the signals (block the signal for the controlling thread).
 *
 * Revision 1.19  2005/06/03 01:22:48  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.18  2005/03/29 18:22:44  shiv
 * PR24134
 * PR24010
 * PR24008
 * PR24184
 * The malloc lib used to report errors when mem* and str* functions
 * were called (for those that take a length parameter) with
 * a length of zero, if the other arguments are not valid..
 * In general these would not cause errors, since
 * no actual data is moved. But since the errors being reported could
 * be useful, the option to increase the level of verbosity for this
 * has been provided. the environment variable
 * MALLOC_CKACCESS_LEVEL can be used or the mallopt call
 * with the option mallopt(MALLOC_CKACCESS_LEVEL, arg)
 * can be used. By default the level is zero, a non-zero
 * level will turn on strict checking and reporting of errors
 * if the arguments are not valid.
 * Also fixed PR24184 which had an incorrect function name
 * being passed in (strchr instead of strrchr... thanx kirk)
 * Modified Files:
 * 	mallocint.h dbg/m_init.c dbg/malloc_chk.c dbg/malloc_g.c
 * 	dbg/mallopt.c dbg/memory.c dbg/string.c
 * 	public/malloc_g/malloc-lib.h
 *
 * Revision 1.17  2005/02/14 21:24:30  shiv
 * Properly do the bound checking now.
 * Modified Files:
 * 	dbg/malloc_chk.c
 *
 * Revision 1.16  2005/02/14 19:20:34  shiv
 * Need to look at the check_guard more carefully.
 *
 * Revision 1.15  2005/02/14 18:36:32  shiv
 * Over eager checking.
 * Modified Files:
 * 	dbg/malloc_chk.c
 *
 * Revision 1.14  2005/02/13 23:15:40  shiv
 * some more cleanup.
 *
 * Revision 1.13  2005/01/19 16:53:00  shiv
 * Slight mods to allow easier access to the free list
 * to allow user land to gather information about allocator.
 * Matching with libc checkin.
 * Modified Files:
 * 	dbg/dump.c dbg/malloc_chk.c dbg/malloc_g.c
 *
 * Revision 1.12  2005/01/16 20:38:45  shiv
 * Latest DBG malloc code. Lots of cleanup/optimistions
 * Modified Files:
 * 	common.mk mallocint.h common/tostring.c dbg/analyze.c
 * 	dbg/calloc.c dbg/dump.c dbg/free.c dbg/m_init.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc_g/malloc-lib.h public/malloc_g/malloc.h
 * 	std/calloc.c std/free.c std/m_init.c std/malloc_wrapper.c
 * 	std/mtrace.c std/realloc.c
 * Added Files:
 * 	dbg/dl_alloc.c dbg/malloc_control.c dbg/malloc_debug.c
 * 	dbg/new.cc public/malloc_g/malloc-control.h
 * 	public/malloc_g/malloc-debug.h
 *
 * Revision 1.11  2004/02/12 15:43:16  shiv
 * Updated copyright/licenses
 * Modified Files:
 * 	common.mk debug.h malloc-lib.h mallocint.h tostring.h
 * 	common/tostring.c dbg/analyze.c dbg/calloc.c dbg/context.h
 * 	dbg/crc.c dbg/dump.c dbg/free.c dbg/m_init.c dbg/m_perror.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc/malloc.h public/malloc_g/malloc-lib.h
 * 	public/malloc_g/malloc.h public/malloc_g/prototypes.h
 * 	std/calloc.c std/context.h std/free.c std/m_init.c
 * 	std/malloc_wrapper.c std/mtrace.c std/realloc.c test/memtest.c
 * 	test/mtrace.c
 *
 * Revision 1.10  2003/12/03 19:54:21  shiv
 * PR17098
 * fixed a case where we were doing potentially misaligned
 * accesses, causing the faulting under SH/arm.
 *
 * Revision 1.9  2003/09/25 19:06:49  shiv
 * Fixed several things in the malloc code including the
 * leak detection for both small and large blocks. re-arranged
 * a lot code, and removed pieces that were not necessary after the
 * re-org. Modified the way in which the elf sections are read to
 * determine where heap references could possibly be stored.
 * set the optimisation for the debug variant at -O0, just so
 * so that debugging the lib itself is a little easier.
 * Modified Files:
 * 	common.mk mallocint.h dbg/dump.c dbg/malloc_chk.c
 * 	dbg/malloc_chn.c dbg/malloc_g.c dbg/malloc_gc.c dbg/mtrace.c
 * 	dbg/process.c dbg/string.c
 *
 * Revision 1.8  2003/09/22 16:59:55  shiv
 * Some fixes for the way the leak detection is done and reported.
 * Modified Files:
 * 	dbg/dump.c dbg/malloc_chk.c dbg/mtrace.c
 *
 * Revision 1.7  2003/06/05 15:21:14  shiv
 * Some cleanup to match changes to the malloc code in
 * libc.
 *
 * Revision 1.6  2002/05/29 19:50:28  shiv
 * Fixed cases where we were performing unaligned
 * acceses, that caused issues on non-x86 platforms.
 * We take care to walk only aligned pointers now
 * into our structures. PR10599
 *
 * Revision 1.5  2002/04/23 17:07:45  shiv
 * PR11328: Changes made to lib/c/alloc synchronised here.
 * Also an initialisation of the arena in lib/c/alloc
 * was inconsistent with the redifnition of struct Arena
 * here.
 *
 * Revision 1.4  2001/03/01 20:37:53  furr
 * Added debug wrappers for the new aligned memory, mprobe and mcheck functions
 * Made mallopt signature compatible
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	common.mk mallocint.h dbg/m_init.c dbg/malloc_chk.c
 *  	dbg/malloc_g.c dbg/mallopt.c public/malloc/malloc.h
 *  	public/malloc_g/malloc-lib.h public/malloc_g/malloc.h
 *  	public/malloc_g/prototypes.h std/m_init.c std/malloc_wrapper.c
 *
 * Revision 1.3  2000/02/08 19:16:23  furr
 * Fix up guard code problems re. underlying implementation
 * Fixed up problems for Java such as locking on mem functions
 *
 *  Modified Files:
 *  	dbg/free.c dbg/malloc_chk.c dbg/memory.c dbg/realloc.c
 *  	dbg/string.c
 *
 * Revision 1.2  2000/02/01 19:48:05  furr
 * Fixed some traversal problems in chain checker
 * Fixed neighbor checks to ensure we don't go outside of the block or arena
 *
 *  Modified Files:
 *  	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 *
 * Revision 1.1  2000/01/31 19:03:31  bstecher
 * Create libmalloc.so and libmalloc_g.so libraries for everything. See
 * Steve Furr for details.
 *
 * Revision 1.1  2000/01/28 22:32:44  furr
 * libmalloc_g allows consistency checks and bounds checking of heap
 * blocks allocated using malloc.
 * Initial revision
 *
 *  Added Files:
 *  	Makefile analyze.c calloc_g.c crc.c dump.c free.c m_init.c
 *  	m_perror.c malloc-config.c malloc_chk.c malloc_chn.c
 *  	malloc_g.c malloc_gc.c mallopt.c memory.c process.c realloc.c
 *  	string.c tostring.c inc/debug.h inc/mallocint.h inc/tostring.h
 *  	inc/malloc_g/malloc inc/malloc_g/malloc.h
 *  	inc/malloc_g/prototypes.h test/memtest.C test/memtest.c
 *  	x86/Makefile x86/so/Makefile
 *
 * Revision 1.1  1996/08/18 21:35:07  furr
 * Initial revision
 *
 * Revision 1.12  1992/01/10  17:51:03  cpcahil
 * more void stuff that slipped by
 *
 * Revision 1.11  1992/01/10  17:28:03  cpcahil
 * Added support for overriding void datatype
 *
 * Revision 1.10  1991/12/31  02:23:29  cpcahil
 * fixed verify bug of strncpy when len was exactly same as strlen
 *
 * Revision 1.9  91/12/02  19:10:12  cpcahil
 * changes for patch release 5
 * 
 * Revision 1.8  91/11/25  14:42:01  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.7  91/11/24  00:49:29  cpcahil
 * first cut at patch 4
 * 
 * Revision 1.6  91/11/20  11:54:11  cpcahil
 * interim checkin
 * 
 * Revision 1.5  90/08/29  22:23:48  cpcahil
 * added new function to check on strings up to a specified length 
 * and used it within several strn* functions.
 * 
 * Revision 1.4  90/05/11  00:13:09  cpcahil
 * added copyright statment
 * 
 * Revision 1.3  90/02/24  21:50:22  cpcahil
 * lots of lint fixes
 * 
 * Revision 1.2  90/02/24  17:29:38  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs 
 * id string
 * 
 * Revision 1.1  90/02/24  14:57:03  cpcahil
 * Initial revision
 * 
 */
