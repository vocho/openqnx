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

void *
__bounds_realloc(void *cptr, size_t size)
{
	int line = libmalloc_caller();
	return( debug_realloc(NULL,line,cptr,size) );
}

void *
realloc(void *cptr, size_t size)
{
	int line = libmalloc_caller();
	return( debug_realloc(NULL,line,cptr,size) );
}

/*
 * Function:	debug_realloc()
 *
 * Purpose:	to re-allocate a data area.
 *
 * Arguments:	cptr	- pointer to area to reallocate
 *		size	- size to change area to
 *
 * Returns:	pointer to new area (may be same area)
 *
 * Narrative:	verify pointer is within malloc region
 *		obtain mlist pointer from cptr
 *		verify magic number is correct
 *		verify inuse flag is set
 *		verify connection to adjoining segments is correct
 *		save requested size
 *		round-up size to appropriate boundry
 *		IF size is bigger than what is in this segment
 *		    try to join next segment to this segment
 *		IF size is less than what is is this segment
 *		    determine leftover amount of space
 *		ELSE
 *		    allocate new segment of size bites
 *		    IF allocation failed
 *		        return NULL
 *		    copy previous data to new segment
 *		    free previous segment
 *		    return new pointer
 *		split of extra space in this segment (if any)
 *		clear bytes beyound what they had before
 *		return pointer to data 
 */

void *
__debug_realloc(const char *file, int line, void *cptr, size_t size)
{
	static int		  call_counter;
	char			* func = "realloc";
	int			  i;
	char			* new_cptr;
	Flink			* flink;
	DebugInfo_t		* ptr;
	Dhead			* dh;
	int			  in_malloc_code;
	arena_range_t		  range;

	ENTER();
	MALLOC_INIT();

	/*
	 * increment the call counter
	 */
	call_counter++;

	/*
	 * IF malloc chain checking is on, go do it.
	 */
	if( _malloc_check_on ) {
		(void) DBFmalloc_chain_check(func,file,line,1);
	}

	if( cptr == NULL ) {
		/*
		 * else we can't combine it, so lets allocate a new chunk,
		 * copy the data and free the old chunk...
		 */
		new_cptr = (char *) debug_malloc(file,line,size);

		if( new_cptr == (char *) 0) {
			LEAVE();
			return(new_cptr);
		}

		/*
		 * get the malloc internal struct for the new area.  This
		 * is needed because malloc has identifed the record as a
		 * malloc call when it is actually a realloc call.  Therefore
		 * we have to change the identification info.
		 */
		/* ok to walk since new_cptr is aligned */
		dh = (Dhead *)new_cptr - 1;
		ptr = &dh->d_debug;
		ptr->id = call_counter;
		MALLOC_PTR_FILL_DEBUG(ptr->dbg, new_cptr);
		SETTYPE(ptr,M_T_REALLOC);
		((Dhead *)dh)->d_crc = LinkCRC(dh);

		LEAVE();
		new_cptr = NULL;
		return(dh+1);
	}

	/*
	 * verify that cptr is within the malloc region...
	 */
	if (cptr != (void *)__TRUNC((ulong_t)cptr, _MALLOC_ALIGN)) {
		malloc_errno = M_CODE_BAD_PTR;
		malloc_warning(func,file,line,(void *)NULL);
        LEAVE();
		return (NULL);
	}
	/*
	 * verify that cptr is within the malloc region...
	 */
	if (malloc_verify_alloc != 0) {
		if (!find_malloc_area(cptr, &range)) {
			malloc_errno = M_CODE_BAD_PTR;
			malloc_warning(func,file,line,(void *)NULL);
			LEAVE();
			return (NULL);
		}
	}

	/* 
	 * convert pointer to debugInfo struct pointer.  
	 */
	/* cptr is aligned, else find_malloc_area would have failed */
	dh = (Dhead *)cptr - 1;
	ptr = &dh->d_debug;
	flink = (Flink *)dh;
	
	if (!MAGIC_MATCH(ptr)) {
		malloc_errno = M_CODE_BAD_MAGIC;
		malloc_warning(func,file,line,(void *)NULL);
        LEAVE();
		return(NULL);
	}

	if( dh->d_crc != LinkCRC(dh) ) {
		malloc_errno = M_CODE_BAD_CRC;
		malloc_warning(func,file,line,(void *)dh);
		LEAVE();
		return NULL;
	}


	if( ! (ptr->flag & M_INUSE) ) {
		malloc_errno = M_CODE_NOT_INUSE ;
		malloc_warning(func,file,line,flink);
        LEAVE();
		return(NULL);
	}

/*
 * Here's a new one!  Even fill checking won't help if you
 * run into the next block's header, so we can check the
 * next contiguous block to see if it's all right.
 */
 //malloc_check_neighbors(func,file,line,&range,dh);
	if( _malloc_fill_area && _malloc_fill_area < ptr->hist_id ) {
		malloc_check_fill(func,file,line,dh);
	}

	malloc_check_guard(func, file, line, dh);

 	if( (flink->f_prev && (flink->f_prev->f_next != flink) ) ||
	    (flink->f_next && (flink->f_next->f_prev != flink) ) ) {
		malloc_errno = M_CODE_BAD_CONNECT;
		malloc_warning(func,file,line,flink);
        LEAVE();
		return(NULL);
	}

	if( size > _msize(cptr) ) {
		/*
		 * copy the data and free the old chunk...
		 */

		new_cptr = debug_malloc(file,line,size);

		if( new_cptr == (char *) 0) {
	        LEAVE();
			return(new_cptr);
		}

		if (size < DH_ULEN(dh)) {
			i = size;
		} else {
			i = DH_ULEN(dh);
		}
		in_malloc_code = in_malloc();
		set_in_malloc(++in_malloc_code);
		(void) memcpy(new_cptr,(char *)(dh+1),i);
		set_in_malloc(--in_malloc_code);

		free(cptr);

		/*
		 * get the malloc internal struct for the new area.  This
		 * is needed because malloc has identifed the record as a
		 * malloc call when it is actually a realloc call.  Therefore
		 * we have to change the identification info.
		 */
		dh = (Dhead *)new_cptr - 1;
		ptr = &dh->d_debug;

		/*
		 * save the id info.
		 */	
		ptr->file = file;
		ptr->callerpc_line = (unsigned int *)line;
		ptr->id = call_counter;
		ptr->hist_id = ++malloc_hist_id;
		MALLOC_PTR_FILL_DEBUG(ptr->dbg, new_cptr);
		SETTYPE(ptr,M_T_REALLOC);

		/*
		 * if necessary, fill in the last few bytes with the fill character
		 */
		// FIX:
		if( _malloc_fill_area && _malloc_fill_area < ptr->hist_id ) {
			char *data = (char *)(dh + 1);
			set_guard(data, size);
		} else {
			dh->d_usize = size;
		}
		dh->d_crc = LinkCRC(dh);

		LEAVE();
		new_cptr = NULL;
		return(dh+1);

	} /* else... */

	/*
	 * save amount of real data in new segment (this will be used in the
	 * memset later) and then save requested size of this segment.
	 */

	if( DH_ULEN(dh) < size ) {
		i = DH_ULEN(dh);
	} else {
		if (size == 0) {
		    DBFfree("realloc", file, line, cptr);
	        LEAVE();
		    return NULL;
		}
		i = size;
	}

	/*
	 * save the id info.
	 */	
	ptr->file = file;
	ptr->callerpc_line = (unsigned int *)line;
	ptr->id = call_counter;
	ptr->hist_id = ++malloc_hist_id;
	MALLOC_PTR_FILL_DEBUG(ptr->dbg, cptr);
	SETTYPE(ptr,M_T_REALLOC);

	/*
	 * if necessary, fill in the last few bytes with the fill character
	 */
	// FIX:
	if( _malloc_fill_area && _malloc_fill_area < ptr->hist_id ) {
		char *data = (char *)(dh + 1);
		set_guard(data, size);
	} else {
		dh->d_usize = size;
	}
	dh->d_crc = LinkCRC(dh);
	
	LEAVE();
	return((char *)(dh+1));

} /* realloc(... */

void *
debug_realloc(const char *file, int line, void *cptr, size_t size)
{
	void *ncptr;
	ENTER();
	MALLOC_INIT();

	MALLOC_GETBT(__cdbt);
	
	if (__realloc_hook != NULL) {
		ncptr =  (*__realloc_hook)(file,line,cptr,size);
	} else {
		ncptr =  __debug_realloc(file,line,cptr,size);
		MALLOC_FILLCALLERD(ncptr, __cdbt, line);
	}
	LEAVE();
	return(ncptr);
}


/*
 * $Log$
 * Revision 1.12  2006/12/22 17:38:48  seanb
 * - Add more magic numbers to reduce chance of false
 *   positive.  Still not perfect so PR left open.
 * PR:43552
 * CI:ELaskavaia
 *
 * Revision 1.11  2006/09/28 19:05:57  alain
 * PR:41782
 * CI: shiv@qnx.com
 * CI: cburgess@qnx.com
 *
 * Commiting the work done for IGT-CE on the head.
 *
 * Revision 1.10.2.6  2006/07/27 19:53:30  alain
 *
 * Remove a deadlock (not calling LEAVE) in the control thread
 * new setting MALLOC_VERBOSE
 *
 * Revision 1.10.2.5  2006/07/26 19:58:24  alain
 *
 * Rename malloc_check_alloc to malloc_verify_alloc for consistency
 *
 * Revision 1.10.2.4  2006/07/26 19:53:50  alain
 *
 * Rename MALLOC_CKALLOC_PARAM to MALLOC_CKALLOC for consistency
 * add as clone to MALLOC_FILL_AREA MALLOC_CKBOUNDS for consistency
 *
 * Revision 1.10.2.3  2006/07/19 19:51:57  alain
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
 * Revision 1.10.2.2  2006/05/15 16:42:49  alain
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
 * Revision 1.10.2.1  2006/04/17 20:54:37  alain
 *
 *
 * We use the backtraces() function call to get backtrace when the builtin
 * function call __builtin_return_address() fails.
 * Also fix indentation and code formating on many of the files.
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
 * Revision 1.9  2005/02/25 03:03:37  shiv
 * More fixes for the debug malloc, for the tools to work
 * better.
 * Modified Files:
 * 	malloc-lib.h mallocint.h dbg/calloc.c dbg/dump.c dbg/m_init.c
 * 	dbg/malloc_debug.c dbg/malloc_g.c dbg/mtrace.c dbg/realloc.c
 * 	public/malloc_g/malloc.h
 *
 * Revision 1.8  2005/01/16 20:38:45  shiv
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
 * Revision 1.7  2004/02/12 15:43:17  shiv
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
 * Revision 1.6  2002/05/29 19:50:28  shiv
 * Fixed cases where we were performing unaligned
 * acceses, that caused issues on non-x86 platforms.
 * We take care to walk only aligned pointers now
 * into our structures. PR10599
 *
 * Revision 1.5  2001/02/05 18:34:30  furr
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
 * Revision 1.4  2000/04/24 15:45:50  furr
 *
 *  Modified Files:
 *  	dbg/calloc.c dbg/free.c dbg/malloc_g.c dbg/realloc.c
 *
 *  Added entry points for bounds-checking GCC
 *
 * Revision 1.3  2000/04/12 19:33:34  furr
 *
 *  Committing in .
 *  Modified Files:
 *  	malloc_g.c realloc.c
 *
 *  	- fixed the setting of the ulen in the header for mallocs and
 *  	  reallocs when fill-area boundary checking isn't enabled and
 *  	  when realloc creates a new block
 *
 * Revision 1.2  2000/02/08 19:16:23  furr
 * Fix up guard code problems re. underlying implementation
 * Fixed up problems for Java such as locking on mem functions
 *
 *  Modified Files:
 *  	dbg/free.c dbg/malloc_chk.c dbg/memory.c dbg/realloc.c
 *  	dbg/string.c
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
 * Revision 1.2  1996/08/18 21:01:21  furr
 * print the caller return address on errors
 *
 * Revision 1.1  1996/07/24 18:05:57  furr
 * Initial revision
 *
 * Revision 1.15  1992/01/10  17:28:03  cpcahil
 * Added support for overriding void datatype
 *
 * Revision 1.14  1991/12/06  08:54:19  cpcahil
 * cleanup of __STDC__ usage and addition of CHANGES file
 *
 * Revision 1.13  91/12/04  09:23:44  cpcahil
 * several performance enhancements including addition of free list
 * 
 * Revision 1.12  91/12/02  19:10:14  cpcahil
 * changes for patch release 5
 * 
 * Revision 1.11  91/11/25  14:42:05  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.10  91/11/24  00:49:32  cpcahil
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
