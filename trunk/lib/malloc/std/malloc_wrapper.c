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
/* PDB: this file is compiled */




#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include "malloc-lib.h"
//#error malloc_wrapper.c WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
pthread_mutex_t	  _mallocg_lock = PTHREAD_RMUTEX_INITIALIZER;

extern void _malloc_init_direct();
extern void * _malloc_direct(const char *file, int line, size_t size);
extern void * _calloc_direct(const char *file, int line, size_t nelems, size_t size);
extern void * _realloc_direct(const char *file, int line, void *ptr, size_t size);
extern void _free_direct(const char *file, int line, void *ptr);

void (*__malloc_init_hook)() = &_malloc_init_direct;
void * (*__malloc_hook)(const char *file, int line, size_t size) = &_malloc_direct;
void * (*__calloc_hook)(const char *file, int line, size_t nelems, size_t size) = &_calloc_direct;
void * (*__realloc_hook)(const char *file, int line, void *ptr, size_t size) = &_realloc_direct;
void (*__free_hook)(const char *file, int line, void *ptr) = &_free_direct;

int _mallopt(int cmd, int value);

int
mallopt(int cmd, int value)
{
	return _mallopt(cmd,value);
}

ssize_t
_msize (void *ptr)
{
        Dhead *dh = (Dhead *)ptr - 1;

        if (dh->d_size < 0) {
                Block *b = (Block *)((char *)dh + dh->d_size);
                if (b->magic == BLOCK_MAGIC)
                        return (b->nbpe);
                panic("size!");
        }

        return DH_LEN(dh) - D_OVERHEAD();
}


ssize_t
_musize (void *ptr)
{
    Dhead *dh = (Dhead *)ptr - 1;

    return DH_ULEN(dh);
}

void *
_malloc_direct(const char *file, int line, size_t size)
{
    return _malloc(size);
}

void *
__dbmalloc(const char *file, int line, size_t size)
{
		void *return_ptr;
    ENTER();
    MALLOC_INIT();
    if (__malloc_hook != NULL) {
    	return_ptr = (*__malloc_hook)(file,line,size);
    } else {
    	return_ptr = _malloc(size);
    }
    LEAVE();
		return(return_ptr);
}

void *
malloc(size_t size)
{
    int line = (int)libmalloc_caller();
    return( __dbmalloc(NULL,line,size) );
}

void *
malloc_pc(size_t size, unsigned int *pc)
{
    return( __dbmalloc(NULL,(int)pc,size) );
}

#ifndef NONEAR
void *
_nmalloc(size_t size)
{
    int line = (int)libmalloc_caller();
    return __dbmalloc(NULL, line, size);
}
#endif

/*
 * $Log$
 * Revision 1.8  2005/06/03 01:22:48  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.7  2005/01/16 20:38:46  shiv
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
 * Revision 1.6  2004/03/02 21:23:50  shiv
 * PR17515, some fixes for the locking.
 *
 * Revision 1.5  2004/02/12 15:43:17  shiv
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
 * Revision 1.4  2004/01/12 14:50:06  shiv
 * PR17515: incomplete previous fix, needed to make the
 * changes in this file also..
 * Modified Files:
 * 	std/malloc_wrapper.c
 *
 * Revision 1.3  2003/11/04 18:06:35  shiv
 * Fixed several compiler warnings.
 *
 * Revision 1.2  2001/03/01 20:37:55  furr
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
