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
#ifndef MALLOC_CACHE_H_
#define MALLOC_CACHE_H_

Dhead * mc_cache_get(const void * ptr, arena_range_t * range);
void mc_cache_put_malloc(void * ptr);
void mc_cache_release(void * ptr);
int mc_cache_get_size();
void mc_cache_set(int size);
void mc_cache_update(const void *ptr, Dhead* dh);
void mc_cache_clear();
int mc_cache_pointer_in_heap(void * ptr);

#define CACHED_NULL ((Dhead *)-1)
#define DHEAD_TO_POINTER(dh) ((void *)(((Dhead *)dh)+1))
#define POINTER_TO_DHEAD(ptr) ((Dhead *)(((Dhead *)ptr)-1))

#endif /*MALLOC_CACHE_H_*/
