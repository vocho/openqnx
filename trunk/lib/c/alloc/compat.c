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
 * compat.c
 *
 * Functions kept for compatibility with other Unix/POSIX systems
 * and/or with differing standards (SVID, XPG, etc.)
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <pthread.h>

//Must use <> include for building libmalloc.so
#include <malloc-lib.h>

LIBC_WEAK(mallinfo, __mallinfo);
LIBC_WEAK(mallopt, __mallopt);
LIBC_WEAK(mprobe, __mprobe);
LIBC_WEAK(mcheck, __mcheck);

#define PTHREAD_CALL(p) p

extern pthread_mutex_t _malloc_mutex;
extern int _malloc_check_on;

struct mallinfo 
mallinfo(void) 
{
    struct mallinfo info;
    PTHREAD_CALL(_mutex_lock(&_malloc_mutex));
    info.keepcost = 0;
    info.arena = _malloc_stats.m_heapsize;
    info.ordblks = _malloc_stats.m_blocks;
    info.smblks = _malloc_stats.m_small_blocks;
    info.hblks = _malloc_stats.m_hblocks;
    info.hblkhd = _malloc_stats.m_small_overhead;
    info.usmblks = _malloc_stats.m_small_allocmem;
    info.fsmblks = _malloc_stats.m_small_freemem;
    info.uordblks = _malloc_stats.m_allocmem;
    info.fordblks = _malloc_stats.m_freemem;
    PTHREAD_CALL(_mutex_unlock(&_malloc_mutex));
    return info;
}

int
mcheck(void (*abort_fn)(enum mcheck_status __status)) 
{
    int status = -1;
    PTHREAD_CALL(_mutex_lock(&_malloc_mutex));
    if (!_malloc_check_on) {
	status = 0;
	_malloc_check_on = 2;
	if (abort_fn != NULL) _malloc_abort = abort_fn;
    }
    PTHREAD_CALL(_mutex_unlock(&_malloc_mutex));
    return status;
}

enum mcheck_status
mprobe(void *ptr)
{
    Dhead *dh = (Dhead *)ptr - 1;
    enum mcheck_status status = MCHECK_OK;
    if (_malloc_check_on ) {
	if (dh->d_size < 0) {
	    Block *b = (Block *)((char *)dh + dh->d_size);
	    if (b->magic != BLOCK_MAGIC
#ifdef MALLOC_GUARD
		|| dh->d_usize > b->nbpe
#endif
		|| ((char *)ptr + b->nbpe > b->bend)) {
		status = MCHECK_HEAD;
	    } else {
#ifdef MALLOC_GUARD
		status = _malloc_guard_status(ptr, dh, b->nbpe);
#endif
	    }
	} else {
	    if (!DH_ISBUSY(dh)) {
		status = MCHECK_FREE;
	    } else {
#ifdef MALLOC_GUARD
		status = _malloc_guard_status(ptr, dh, DH_LEN(dh) - D_OVERHEAD());
#endif
		if (status <= MCHECK_OK) {
		    Dtail *dt = HEAD_TO_DT(dh);
		    if (!DT_ISBUSY(dt) || (DT_LEN(dt) != DH_LEN(dh))) {
			status = MCHECK_TAIL;
		    }
		}
	    }
	}
    } else {
	status = MCHECK_DISABLED;
    }
    return status;
}

int
mallopt(int cmd, int value)
{
    int status = 0;
    PTHREAD_CALL(_mutex_lock(&_malloc_mutex));
    switch (cmd) {
	case M_MXFAST:	/* SysV - quietly ignore */
	case M_NLBLKS:	/* SysV */
	case M_GRAIN:	/* SysV */
            break;
	case M_TRIM_THRESHOLD:	/* GNU - quietly ignore */
	case M_TOP_PAD:		/* GNU - quietly ignore */
	case M_MMAP_THRESHOLD:	/* GNU - quietly ignore */
	case M_MMAP_MAX:	/* GNU - quietly ignore */
            break;
        default:
            status = malloc_opts(cmd, (void *)value);	/* Use dlist opts */
            break;
    }
    PTHREAD_CALL(_mutex_unlock(&_malloc_mutex));
    return status;
}

__SRCVERSION("compat.c $Rev: 167420 $");
