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

/*
 * Function:	malloc_size()
 *
 * Purpose:	to determine the amount of memory in use
 *
 * Arguments:	histidptr - pointer to hist id area
 *
 * Returns:	Number of bytes currently in use
 *
 * Narrative:	make sure the malloc chain is ok
 *		for each element in the malloc chain
 *		    if it is in use
 *		        add size to total size
 * 		if hist id is wanted
 *                  set it
 *		return total in-use size
 *		
 */

ulong_t 
malloc_size(ulong_t *histptr)
{
    return( DBmalloc_size((char *)NULL,0,histptr) );
}

ulong_t
DBmalloc_size(const char *file, int line, ulong_t *histptr)
{
    unsigned long	  size = 0;
    DebugInfo_t		* ptr;
    Arena		* ap;
    Band		* band;
    int			  nb;
    Flink		* flink;

    ENTER();
    MALLOC_INIT();

    /*
     * make sure the chain is ok (otherwise we will have a problem
     * parsing through it
     */
    (void) DBFmalloc_chain_check("malloc_size",file,line,1);

    /*
     * Go through malloc chains for each Arena
     */
	for (ap = __arenas.a_next; ap != &__arenas; ap = ap->a_next) {
	if (ap->a_size < sizeof(*ap)) panic("a_size");

        /*
         * for each element in the malloc chain
         */
	for(flink = ap->a_malloc_chain.head; flink; flink = flink->f_next)
	{
	    ptr = &((Dhead *)flink)->d_debug;

	    /*
	     * if the element is in use
	     */
	    if( (ptr->flag&M_INUSE) == M_INUSE)
	    {
		/* 
		 * add its requested size into the total size
		 */
		size += DH_ULEN(flink);
	    }
	}
    }

    /*
     * Go through malloc chains for every Block in every Band
     */
    for (nb = 0; nb < *__pnband; nb++) 
    {
	Block *bp;

	band = __pBands[nb];

	/*
	 * For each Block on the allocated list
	 */
        for (bp = band->alist; bp; bp = bp->next)
        {
            /*
             * for each element in the malloc chain
             */
	    for(flink = bp->malloc_chain.head; flink; flink = flink->f_next)
	    {
	        ptr = &((Dhead *)flink)->d_debug;

	        /*
	         * if the element is in use
	         */
	        if( (ptr->flag&M_INUSE) == M_INUSE)
	        {
		    /* 
		     * add its requested size into the total size
		     */
		    size += DH_ULEN(flink);
	        }
	    }
        }

	/*
	 * For each Block on the depleted list
	 */
        for (bp = band->dlist; bp; bp = bp->next)
        {
	    for(flink = bp->malloc_chain.head; flink; flink = flink->f_next)
	    {
	        ptr = &((Dhead *)flink)->d_debug;

	        /*
	         * if the element is in use
	         */
	        if( (ptr->flag&M_INUSE) == M_INUSE)
	        {
		    /* 
		     * add its requested size into the total size
		     */
		    size += DH_ULEN(flink);
	        }
	    }
        }
    }

    /*
     * if the hist id is desired, give it to em.
     */
    if( histptr != NULL )
    {
	*histptr = malloc_hist_id;
    }

    /*
     * return the size
     */
    LEAVE();
    return(size);

} /* malloc_size(... */


/*
 * $Log$
 * Revision 1.5  2007/04/24 15:44:27  shiv
 * PR:29730
 * CI:cburgess
 * CI:xtang
 *
 * this is part of work order WO790334 for IGT. Included enhancements
 * for configurability of the memory allocator.  Includes matching changes for
 * lib/c/alloc and lib/malloc as usual. This is to bring head in line
 * with the work committed to the branch.
 *
 * Revision 1.4  2005/06/03 01:22:48  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.3  2005/01/16 20:38:45  shiv
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
 * Revision 1.2  2004/02/12 15:43:16  shiv
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
 * Revision 1.1  2000/01/31 19:03:30  bstecher
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
 * Revision 1.4  1992/01/10  17:28:03  cpcahil
 * Added support for overriding void datatype
 *
 * Revision 1.3  1991/12/02  19:10:09  cpcahil
 * changes for patch release 5
 *
 * Revision 1.2  91/11/25  14:41:54  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.1  91/11/24  00:49:26  cpcahil
 * first cut at patch 4
 * 
 * Revision 1.9  91/11/20  11:54:11  cpcahil
 * interim checkin
 * 
 * Revision 1.8  90/08/29  21:22:52  cpcahil
 * miscellaneous lint fixes
 * 
 * Revision 1.7  90/05/11  00:13:10  cpcahil
 * added copyright statment
 * 
 * Revision 1.6  90/02/25  11:01:20  cpcahil
 * added support for malloc chain checking.
 * 
 * Revision 1.5  90/02/24  21:50:31  cpcahil
 * lots of lint fixes
 * 
 * Revision 1.4  90/02/24  17:29:39  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs 
 * id string
 * 
 * Revision 1.3  90/02/24  17:20:00  cpcahil
 * attempt to get rid of full path in rcs header.
 * 
 * Revision 1.2  90/02/24  15:14:20  cpcahil
 * 1. added function header 
 * 2. changed calls to malloc_warning to conform to new usage
 * 3. added setting of malloc_errno
 *  4. broke up bad pointer determination so that errno's would be more
 *    descriptive
 * 
 * Revision 1.1  90/02/22  23:17:43  cpcahil
 * Initial revision
 * 
 */
