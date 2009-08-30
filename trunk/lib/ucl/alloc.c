/*
 * This source code may contain confidential information of QNX Software
 * Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
 * modification, disclosure, distribution or transfer of this software,
 * or any software which includes or is based upon any of this code, is
 * prohibited unless expressly authorized by QSSL by written agreement. For
 * more information (including whether this source code file has been
 * published) please email licensing@qnx.com.
 */

/* alloc.c -- memory allocation

   This file is part of the UCL real-time data compression library.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   <markus.oberhumer@jk.uni-linz.ac.at>
   http://wildsau.idv.uni-linz.ac.at/mfx/ucl.html
 */


#include "ucl_conf.h"

#undef ucl_alloc_hook
#undef ucl_free_hook
#undef ucl_alloc
#undef ucl_malloc
#undef ucl_free


/* global allocator hooks */
ucl_alloc_hook_t ucl_alloc_hook = ucl_alloc;
ucl_free_hook_t ucl_free_hook = ucl_free;


#if defined(HAVE_MALLOC_H)
#  include <malloc.h>
#endif
#if defined(__palmos__)
#  include <System/MemoryMgr.h>
#endif


/***********************************************************************
//
************************************************************************/

UCL_PUBLIC(ucl_voidp)
ucl_alloc(ucl_uint nelems, ucl_uint size)
{
    ucl_voidp p = NULL;
    unsigned long s = (unsigned long) nelems * size;

    if (nelems <= 0 || size <= 0 || s < nelems || s < size)
        return NULL;
#if defined(__palmos__)
    p = (ucl_voidp) MemPtrNew(s);
#elif (UCL_UINT_MAX <= SIZE_T_MAX)
    if (s < SIZE_T_MAX)
        p = (ucl_voidp) malloc((size_t)s);
#elif defined(HAVE_HALLOC)
    if (size < SIZE_T_MAX)
        p = (ucl_voidp) halloc(nelems,(size_t)size);
#else
    if (s < SIZE_T_MAX)
        p = (ucl_voidp) malloc((size_t)s);
#endif

    return p;
}


UCL_PUBLIC(ucl_voidp)
ucl_malloc(ucl_uint size)
{
    if (!ucl_alloc_hook)
        return NULL;

#if defined(__palmos__)
    return ucl_alloc_hook(size,1);
#elif (UCL_UINT_MAX <= SIZE_T_MAX)
    return ucl_alloc_hook(size,1);
#elif defined(HAVE_HALLOC)
    /* use segment granularity by default */
    return ucl_alloc_hook((size+15)/16,16);
#else
    return ucl_alloc_hook(size,1);
#endif
}


/***********************************************************************
//
************************************************************************/

UCL_PUBLIC(void)
ucl_free(ucl_voidp p)
{
    if (!p)
        return;

#if defined(__palmos__)
    MemPtrFree(p);
#elif (UCL_UINT_MAX <= SIZE_T_MAX)
    free(p);
#elif defined(HAVE_HALLOC)
    hfree(p);
#else
    free(p);
#endif
}


/*
vi:ts=4:et
*/
