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

/*
 * Function:	calloc()
 *
 * Purpose:	to call debug_calloc to do the allocation
 *
 * Arguments:	nelem	- number of elements
 *		elsize	- size of each element
 *
 * Returns:	whatever debug_calloc returns
 *
 * Narrative:	call debug_calloc and return it's return
 */
void *
calloc(size_t nelem, size_t elsize)
{
	int line = libmalloc_caller();
	return( debug_calloc((char *)NULL,line,nelem,elsize) );
}

void *
__bounds_calloc(size_t nelem, size_t elsize)
{
	int line = libmalloc_caller();
	return( debug_calloc((char *)NULL,line,nelem,elsize) );
}

/*
 * Function:	debug_calloc()
 *
 * Purpose:	to allocate and nullify a data area
 *
 * Arguments:	nelem	- number of elements
 *		elsize	- size of each element
 *
 * Returns:	NULL	- if malloc fails
 *		or pointer to allocated space
 *
 * Narrative:	determine size of area to malloc
 *		malloc area.
 *		if malloc succeeds
 *		    fill area with nulls
 *		return ptr to malloc'd region
 */

void *
__debug_calloc(const char *file, int line, size_t nelem, size_t elsize)
{
	static ulong_t	 call_counter;
	void		*ptr;
	Dhead *dh;
	size_t		 size;

	ENTER();
	MALLOC_INIT();

	/*
	 * increment the call counter
	 */
	call_counter++;

	/*
	 * calculate the size to allocate
	 */
	size = elsize * nelem;

	if( (ptr = debug_malloc(file,line,size)) != NULL) {
		/* this is ok, since the ptr from debug_malloc is always aligned */
		Dhead *dh = (Dhead *)ptr - 1;
		DebugInfo_t *mptr = &dh->d_debug;
		/*
		 * change the id information put in by malloc so that the 
		 * record appears like it got added by calloc
		 */
		mptr->id = call_counter;
		MALLOC_PTR_FILL_DEBUG(mptr->dbg, ptr);
		SETTYPE(mptr,M_T_CALLOC);

		/*
		 * clear the area that was allocated
		 */
        dh->d_crc = LinkCRC(dh);
		(void) memset(ptr,'\0',size);
	}

	LEAVE();
	dh = (Dhead *)ptr-1;
	ptr = NULL;
	return(dh+1);

} /* debug_calloc(... */

void *
debug_calloc(const char *file, int line, size_t nelem, size_t elsize)
{
	void *cptr;
	Dhead *dh;
	ENTER();
	MALLOC_INIT();

	MALLOC_GETBT(__cdbt);

	if (__calloc_hook != NULL) {
		cptr =  (*__calloc_hook)(file,line,nelem,elsize);
	} else {
		cptr =  __debug_calloc(file,line,nelem,elsize);
		MALLOC_FILLCALLERD(cptr, __cdbt, line);
	}
	LEAVE();
	dh = (Dhead *)cptr-1;
	cptr = NULL;
	return(dh+1);
}

/*
 * Function:	cfree()
 *
 * Purpose:	to free an area allocated by calloc (actually frees any
 *		allocated area)
 *
 * Arguments:	cptr	- pointer to area to be freed
 *
 * Returns:	nothing of any use
 *
 * Narrative:	just call the appropriate function to perform the free
 *
 * Note:	most systems do not have such a call, but for the few that do,
 *		it is here.
 */
int
cfree( void *cptr )
{
	int line = libmalloc_caller();
	debug_cfree((const char *)NULL, line, cptr);
	return 0;
}

void
__debug_cfree(const char *file, int line, void *cptr)
{
	DBFfree("cfree",file,line,cptr);
}

void 
debug_cfree(const char *file, int line, void *cptr)
{
    if (__free_hook != NULL) {
    	(*__free_hook)(file,line,cptr);
    } else {
    	DBFfree("cfree",file,line,cptr);
    }
}

/*
 * $Log$
 * Revision 1.9  2006/09/28 19:05:56  alain
 * PR:41782
 * CI: shiv@qnx.com
 * CI: cburgess@qnx.com
 *
 * Commiting the work done for IGT-CE on the head.
 *
 * Revision 1.8.2.3  2006/07/27 19:53:29  alain
 *
 * Remove a deadlock (not calling LEAVE) in the control thread
 * new setting MALLOC_VERBOSE
 *
 * Revision 1.8.2.2  2006/05/15 16:42:49  alain
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
 * Revision 1.8.2.1  2006/04/17 20:54:37  alain
 *
 *
 * We use the backtraces() function call to get backtrace when the builtin
 * function call __builtin_return_address() fails.
 * Also fix indentation and code formating on many of the files.
 *
 * Revision 1.8  2005/06/03 01:22:48  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.7  2005/02/25 03:03:37  shiv
 * More fixes for the debug malloc, for the tools to work
 * better.
 * Modified Files:
 * 	malloc-lib.h mallocint.h dbg/calloc.c dbg/dump.c dbg/m_init.c
 * 	dbg/malloc_debug.c dbg/malloc_g.c dbg/mtrace.c dbg/realloc.c
 * 	public/malloc_g/malloc.h
 *
 * Revision 1.6  2005/01/16 20:38:45  shiv
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
 * Revision 1.5  2004/02/12 15:43:16  shiv
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
 * Revision 1.4  2002/05/29 19:50:28  shiv
 * Fixed cases where we were performing unaligned
 * acceses, that caused issues on non-x86 platforms.
 * We take care to walk only aligned pointers now
 * into our structures. PR10599
 *
 * Revision 1.3  2001/02/05 18:34:30  furr
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
 * Revision 1.2  2000/04/24 15:45:50  furr
 *
 *  Modified Files:
 *  	dbg/calloc.c dbg/free.c dbg/malloc_g.c dbg/realloc.c
 *
 *  Added entry points for bounds-checking GCC
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
 * Revision 1.2  1996/08/18 20:59:57  furr
 * print the caller return address on errors
 *
 * Revision 1.1  1996/07/24 18:06:06  furr
 * Initial revision
 *
 * Revision 1.11  1992/01/10  17:28:03  cpcahil
 * Added support for overriding void datatype
 *
 * Revision 1.10  1991/12/06  17:58:42  cpcahil
 * added cfree() for compatibility with some wierd systems
 *
 * Revision 1.9  91/12/02  19:10:08  cpcahil
 * changes for patch release 5
 * 
 * Revision 1.8  91/11/25  14:41:51  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.7  91/11/24  00:49:21  cpcahil
 * first cut at patch 4
 * 
 * Revision 1.6  90/05/11  00:13:07  cpcahil
 * added copyright statment
 * 
 * Revision 1.5  90/02/24  20:41:57  cpcahil
 * lint changes.
 * 
 * Revision 1.4  90/02/24  17:25:47  cpcahil
 * changed $header to $id so full path isn't included.
 * 
 * Revision 1.3  90/02/24  13:32:24  cpcahil
 * added function header.  moved log to end of file.
 * 
 * Revision 1.2  90/02/22  23:08:26  cpcahil
 * fixed rcs_header line
 * 
 * Revision 1.1  90/02/22  23:07:38  cpcahil
 * Initial revision
 * 
 */
