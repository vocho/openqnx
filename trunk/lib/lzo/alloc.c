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




/* alloc.c -- memory allocation

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#include "lzo_conf.h"

#undef lzo_alloc_hook
#undef lzo_free_hook
#undef lzo_alloc
#undef lzo_malloc
#undef lzo_free


/* global allocator hooks */
lzo_bytep (__LZO_ENTRY *lzo_alloc_hook)(lzo_uint,lzo_uint) = lzo_alloc;
void (__LZO_ENTRY *lzo_free_hook)(lzo_voidp) = lzo_free;


#if defined(HAVE_MALLOC_H)
#  include <malloc.h>
#endif
#if defined(__palmos__)
#  include <System/MemoryMgr.h>
#endif


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(lzo_bytep)
lzo_alloc(lzo_uint nelems, lzo_uint size)
{
	lzo_bytep p = NULL;
	unsigned long s = (unsigned long) nelems * size;

	if (nelems <= 0 || size <= 0 || s < nelems || s < size)
		return NULL;

#if defined(__palmos__)
	p = (lzo_bytep) MemPtrNew(s);
#elif (LZO_UINT_MAX <= SIZE_T_MAX)
	if (s < SIZE_T_MAX)
		p = (lzo_bytep) malloc((size_t)s);
#elif defined(HAVE_HALLOC)
	if (size < SIZE_T_MAX)
		p = (lzo_bytep) halloc(nelems,(size_t)size);
#else
	if (s < SIZE_T_MAX)
		p = (lzo_bytep) malloc((size_t)s);
#endif

	return p;
}


LZO_PUBLIC(lzo_bytep)
lzo_malloc(lzo_uint size)
{
	if (!lzo_alloc_hook)
		return NULL;

#if defined(__palmos__)
	return lzo_alloc_hook(size,1);
#elif (LZO_UINT_MAX <= SIZE_T_MAX)
	return lzo_alloc_hook(size,1);
#elif defined(HAVE_HALLOC)
	/* use segment granularity by default */
	return lzo_alloc_hook((size+15)/16,16);
#else
	return lzo_alloc_hook(size,1);
#endif
}


/***********************************************************************
//
************************************************************************/

LZO_PUBLIC(void)
lzo_free(lzo_voidp p)
{
	if (!p)
		return;

#if defined(__palmos__)
	MemPtrFree(p);
#elif (LZO_UINT_MAX <= SIZE_T_MAX)
	free(p);
#elif defined(HAVE_HALLOC)
	hfree(p);
#else
	free(p);
#endif
}


/*
vi:ts=4
*/
