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
#include "debug.h"

extern void	__update_stats_nfrees(int, int);
extern int __check_ctid();
int malloc_verify_alloc;


/*
 * Function:	free()
 *
 * Purpose:	to deallocate malloced data
 *
 * Arguments:	ptr	- pointer to data area to deallocate
 *
 * Returns:	nothing of any value
 *
 * Narrative:
 *		verify pointer is within malloc region
 *		get mlist pointer from passed address
 *		verify magic number
 *		verify inuse flag
 *		verify pointer connections with surrounding segments
 *		turn off inuse flag
 *		verify no data overrun into non-malloced area at end of segment
 *		IF possible join segment with next segment
 *		IF possible join segment with previous segment
 *		Clear all data in segment (to make sure it isn't reused)
 *
 */

#undef free

void
free(void *cptr)
{
	int line = libmalloc_caller();
	debug_free((char *)NULL, line, cptr);
}

void
__bounds_free(void *cptr)
{
	int line = libmalloc_caller();
	debug_free((char *)NULL, line, cptr);
}


#ifndef NONEAR
void
_nfree(void *cptr)
{
	int line = libmalloc_caller();
	debug_free((char *)NULL, line, cptr);
}
#endif

void
__debug_free(const char *file, int line, void *cptr)
{
	DBFfree("free", file, line, cptr);
}

void 
debug_free(const char *file, int line, void *cptr)
{
	ENTER();
	MALLOC_INIT();
	MALLOC_GETBT(__cdbt);

	if (__free_hook != NULL) {
		(*__free_hook)(file,line,cptr);
	} else {
		DBFfree("free",file,line,cptr);
	}
	LEAVE();
}

void
DBFfree(const char *func, const char *file, int line, void *cptr)
{
	register DebugInfo_t	* ptr;
	Flink			* flink;
	arena_range_t		  range; 

	/*
	 * initialize the malloc sub-system.
	 */
	ENTER();
	MALLOC_INIT();

	/*
	 * IF malloc chain checking is on, go do it.
	 */
	if( _malloc_check_on ) {
		(void) DBFmalloc_chain_check(func,file,line,1);
	}


	if( cptr == NULL ) {
		goto err;
	}


	/*
	 * verify that cptr is within the malloc region...
	 */
	if(cptr != (void *)__TRUNC((ulong_t)cptr, _MALLOC_ALIGN)) {
		malloc_errno = M_CODE_BAD_PTR;
		malloc_warning(func,file,line,(void *)NULL);
		goto err;
	}

	
	/*
	 * verify that cptr is within the malloc region...
	 */
	if (malloc_verify_alloc != 0) {
		if(!find_malloc_area(cptr, &range)) {
			malloc_errno = M_CODE_BAD_PTR;
			malloc_warning(func,file,line,(void *)NULL);
			goto err;
		}
	} else {
		find_malloc_range(cptr, &range);
	}

	/* we know now that cptr is aligned */ 
	flink = (Flink *)((Dhead *)cptr - 1);
	ptr = &((Dhead *)flink)->d_debug;

	/*
	 * check the magic number 
	 */	
	if( (ptr->flag&M_MAGIC_BITS) != M_MAGIC ) {
		if( range.r_ptr && (Dhead *)flink != range.r_ptr ) {
			malloc_errno = M_CODE_PTR_INSIDE;
			malloc_warning(func,file,line,(void *)(range.r_ptr));
			goto err;
		}

		malloc_errno = M_CODE_BAD_MAGIC;
		malloc_warning(func,file,line,(void *)NULL);
		goto err;
	}

	/*
	 * if this segment is not flagged as being in use
	 */
	if( ! (ptr->flag & M_INUSE) ) {
		malloc_errno = M_CODE_NOT_INUSE;
		malloc_warning(func,file,line,flink);
		goto err;
	}

	if( ((Dhead *)flink)->d_crc != LinkCRC(flink)) {
		malloc_errno = M_CODE_BAD_CRC;
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
			(next && (next->f_prev != flink) ) ) {
			malloc_errno = M_CODE_BAD_CONNECT;
			malloc_warning(func,file,line,flink);
			goto err;
		}
	}

	/*
	 * Here's a new one!  Even fill checking won't help if you
	 * run into the next block's header, so we can check the
	 * next contiguous block to see if it's all right.
	 */
	//malloc_check_neighbors(func,file,line,&range,(Dhead *)flink);

	/*
	 * if we are filling the area with fill bytes, check to see if the
	 * program has gone beyond the end of the assigned area
	 */
	if( _malloc_fill_area && _malloc_fill_area < ptr->hist_id ) {
		malloc_check_fill(func,file,line,(Dhead *)flink);
	}
	malloc_check_guard(func, file, line, (Dhead *)flink);

	/*
	 * flag block as freed
	 */
	ptr->flag &= ~M_INUSE;

	DEBUG3(10,"pointers: prev: 0x%.7x,  ptr: 0x%.7x, next: 0x%.7x",
		flink->f_prev, flink, flink->f_next);
    
	DEBUG3(10,"size:     prev: %9d,  ptr: %9d, next: %9d",
		_msize((Dhead *)flink->f_prev + 1), _msize((Dhead *)flink + 1),
		_msize((Dhead *)flink->f_next + 1));
    
	DEBUG3(10,"flags:    prev: 0x%.7x,  ptr: 0x%.7x, next: 0x%.7x",
		LinkDebug(flink->f_prev)->flag, ptr->flag, LinkDebug(flink->f_next)->flag);
    
	/*
	 * fill this block with '\02's to ensure that nobody is using a 
	 * pointer to already freed data...
	 */
	// FIX:
	if( _malloc_fill_area && _malloc_fill_area < ptr->hist_id ) {
		malloc_aligned_memset((Dhead *)flink+1,M_FREE_FILL,_msize((Dhead *)flink + 1));
	}

	/*
	 * Now remove it from the malloc chain
	 */
	{
		Flink	*next = flink->f_next;
		Flink	*prev = flink->f_prev;
		chain_t	*chain = (range.r_type == RANGE_BLOCK)
			? &range.un.r_block->malloc_chain 
			: &range.un.r_arena->a_malloc_chain;
		if (next == NULL || prev == NULL) {
			if (prev == NULL) {
				if (next != NULL) {
					/* We have lined up Dhead/Flink/ListNode elements */
					next->f_prev = prev;
					((Dhead *)next)->d_crc = LinkCRC(next);
				} else {
					chain->tail = NULL;
				}
				chain->head = next;
			} else {
				chain->tail = prev;
				prev->f_next = NULL;
				((Dhead *)prev)->d_crc = LinkCRC(prev);
			}
		} else {
			/* We have lined up Dhead/Flink/ListNode elements */
			next->f_prev = prev;
			prev->f_next = next;
			((Dhead *)next)->d_crc = LinkCRC(next);
			((Dhead *)prev)->d_crc = LinkCRC(prev);
		}
	}
	__malloc_del_dbg_info(ptr->dbg);
	ptr->dbg = NULL;
	((Dhead *)flink)->d_crc = LinkCRC(flink);

	if (!__check_ctid()) {
		__update_stats_nfrees(((Dhead *)flink)->d_usize, 1);
	}
	_free(cptr);

err:
	LEAVE();
}

/*
 * $Log$
 * Revision 1.11  2006/09/28 19:05:56  alain
 * PR:41782
 * CI: shiv@qnx.com
 * CI: cburgess@qnx.com
 *
 * Commiting the work done for IGT-CE on the head.
 *
 * Revision 1.10.2.8  2006/08/09 19:11:21  alain
 *
 * Add 2 new macros MALLOC_TRACEBTDEPTH and MALLOC_EVENTBTDEPTH to be able
 * to set bactrace depth.
 *
 * Revision 1.10.2.7  2006/07/26 19:58:24  alain
 *
 * Rename malloc_check_alloc to malloc_verify_alloc for consistency
 *
 * Revision 1.10.2.6  2006/07/26 19:53:49  alain
 *
 * Rename MALLOC_CKALLOC_PARAM to MALLOC_CKALLOC for consistency
 * add as clone to MALLOC_FILL_AREA MALLOC_CKBOUNDS for consistency
 *
 * Revision 1.10.2.5  2006/07/19 19:51:56  alain
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
 * Revision 1.10.2.4  2006/05/15 16:42:49  alain
 *
 * bt.c:add to backtrace() a starting address to eliminate undesirable backtrace
 * from the debug malloc library itself.
 *
 * mtrace.c: new format all  tracing event will have a STARTEV and ENDEV, the
 * backtrace is now after the {m,re,c}alloc line.
 *
 * calloc.c, realloc.c, free.c, malloc_g.c:
 * adjust the format to changes describe above.
 *
 * m_inti.c mallocpt.c:
 * new environment MALLOC_USE_DLADDR to enable or disable the use of dladdr() call.
 *
 * Revision 1.10.2.3  2006/04/17 20:54:37  alain
 *
 *
 * We use the backtraces() function call to get backtrace when the builtin
 * function call __builtin_return_address() fails.
 * Also fix indentation and code formating on many of the files.
 *
 * Revision 1.10.2.2  2006/03/16 20:48:52  alain
 *
 * Provide a configurable way of setting the statistics counters.
 * Do some indentations fixes.
 *
 * Revision 1.10.2.1  2006/03/13 17:19:08  alain
 *
 *
 * Cleaning up of the indentation and remove of dead code.
 *
 * Revision 1.10  2005/06/03 01:22:48  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.9  2005/03/28 21:29:15  shiv
 * PR24104
 * PR24105
 * Corrected the back trace for a free call and the size
 * being reported while tracing a free call.
 * Modified Files:
 * 	free.c mtrace.c
 *
 * Revision 1.8  2005/02/13 23:15:40  shiv
 * some more cleanup.
 *
 * Revision 1.7  2005/01/16 20:38:45  shiv
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
 * Revision 1.6  2004/02/12 15:43:16  shiv
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
 * Revision 1.5  2002/05/29 19:50:28  shiv
 * Fixed cases where we were performing unaligned
 * acceses, that caused issues on non-x86 platforms.
 * We take care to walk only aligned pointers now
 * into our structures. PR10599
 *
 * Revision 1.4  2001/02/05 18:34:30  furr
 * Added mtrace support
 * - produce output compatible with GNU tools
 *    - no code from GNU used, but see glibc mtrace.pl for what it expects
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	mallocint.h dbg/calloc.c dbg/free.c dbg/m_init.c
 *  	dbg/malloc_g.c dbg/realloc.c public/malloc_g/prototypes.h
 *  	test/memtest.c
 *  Added Files:
 *  	dbg/mtrace.c
 *
 * Revision 1.3  2000/04/24 15:45:50  furr
 *
 *  Modified Files:
 *  	dbg/calloc.c dbg/free.c dbg/malloc_g.c dbg/realloc.c
 *
 *  Added entry points for bounds-checking GCC
 *
 * Revision 1.2  2000/02/08 19:16:23  furr
 * Fix up guard code problems re. underlying implementation
 * Fixed up problems for Java such as locking on mem functions
 *
 *  Modified Files:
 *  	dbg/free.c dbg/malloc_chk.c dbg/memory.c dbg/realloc.c
 *  	dbg/string.c
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
 * Revision 1.2  1996/08/18 21:00:44  furr
 * print the caller return address on errors
 *
 * Revision 1.1  1996/07/24 18:05:51  furr
 * Initial revision
 *
 * Revision 1.17  1992/01/28  14:30:18  cpcahil
 * Changes from the c.s.r second review
 *
 * Revision 1.16  1992/01/10  17:28:03  cpcahil
 * Added support for overriding void datatype
 *
 * Revision 1.15  1991/12/06  17:58:44  cpcahil
 * added cfree() for compatibility with some wierd systems
 *
 * Revision 1.14  91/12/06  08:54:17  cpcahil
 * cleanup of __STDC__ usage and addition of CHANGES file
 * 
 * Revision 1.13  91/12/04  09:23:37  cpcahil
 * several performance enhancements including addition of free list
 * 
 * Revision 1.12  91/12/02  19:10:09  cpcahil
 * changes for patch release 5
 * 
 * Revision 1.11  91/11/25  14:41:53  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.10  91/11/24  00:49:25  cpcahil
 * first cut at patch 4
 * 
 * Revision 1.9  90/08/29  21:22:48  cpcahil
 * miscellaneous lint fixes
 * 
 * Revision 1.8  90/05/11  00:13:08  cpcahil
 * added copyright statment
 * 
 * Revision 1.7  90/02/25  11:00:18  cpcahil
 * added support for malloc chain checking.
 * 
 * Revision 1.6  90/02/24  21:50:18  cpcahil
 * lots of lint fixes
 * 
 * Revision 1.5  90/02/24  17:29:13  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs 
 * id string
 * 
 * Revision 1.4  90/02/24  15:15:32  cpcahil
 * 1. changed ALREADY_FREE errno to NOT_INUSE so that the same errno could
 *    be used by both free and realloc (since it was the same error).
 * 2. fixed coding bug
 * 
 * Revision 1.3  90/02/24  14:23:45  cpcahil
 * fixed malloc_warning calls
 * 
 * Revision 1.2  90/02/24  13:59:10  cpcahil
 * added function header.
 * Modified calls to malloc_warning/malloc_fatal to use new code error messages
 * Added support for malloc_errno setting of error codes.
 * 
 * Revision 1.1  90/02/22  23:17:43  cpcahil
 * Initial revision
 * 
 */
