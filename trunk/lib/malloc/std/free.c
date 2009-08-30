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





#include <stdio.h>
#include "malloc-lib.h"

void 
__dbfree(const char *file, int line, void *cptr)
{
    ENTER();
    MALLOC_INIT();
    if (__free_hook != NULL) {
    	(*__free_hook)(file,line,cptr);
    } else {
    	_free(cptr);
    }
    LEAVE();
}

void 
_free_direct(const char *file, int line, void *cptr)
{
    _free(cptr);
}

void
free(void *cptr)
{
    int line = (int)libmalloc_caller();
    __dbfree((char *)NULL, line, cptr);
}

#ifndef NONEAR
void
_nfree(void *cptr)
{
    int line = (int)libmalloc_caller();
    __dbfree((char *)NULL, line, cptr);
}
#endif

/*
 * $Log$
 * Revision 1.4  2005/06/03 01:22:48  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.3  2005/01/16 20:38:46  shiv
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
 * Revision 1.2  2004/02/12 15:43:17  shiv
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
 * Revision 1.1  2001/02/09 22:28:12  furr
 * Added necessary rule changes and source code to support non-debugging
 * malloc library.
 *   - Keeps same info as libc/alloc with guard code and caller pc turned
 *     on
 *   - Supports malloc hooks for functionality such as mtrace, memusage
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	common.mk malloc-lib.h mallocint.h dbg/tostring.c
 *  	public/malloc_g/malloc.h public/malloc_g/prototypes.h
 *  	test/memtest.c
 *  Added Files:
 *  	common/tostring.c public/malloc/malloc.h
 *  	public/malloc_g/malloc-lib.h std/calloc.c std/context.h
 *  	std/free.c std/m_init.c std/malloc_wrapper.c std/mtrace.c
 *  	std/realloc.c
 *
 */
