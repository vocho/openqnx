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

#include <malloc.h>
#include <stdio.h>
#include <fcntl.h>
#include "malloc-lib.h"
#include "malloc_cache.h"
#include "assert.h"
#include <setjmp.h>


#define MAX_CACHE (2048)

static int user_cache_max_size = 32;
static pthread_mutex_t mutex = PTHREAD_RMUTEX_INITIALIZER;

static struct cache_item {
    const Dhead * dhead; // dh for heap, or pointer for non-heap (size is 0)
    unsigned int size;
}
cache [MAX_CACHE];

static int last_index = 0;


static void * _malloc_high_mark = NULL;
static void * _malloc_low_mark = NULL;
#define POINTER_IN_HEAP(ptr) (!((_malloc_high_mark && (_malloc_high_mark  - (ptr))<0) || (_malloc_low_mark - (ptr)>0)))

/**
 * Return 1 if pointer is in heap, 0 if not.
 * This is based on cache heuristics of high and low heap mark, can be false if direct memory mapping is used.
 */
int mc_cache_pointer_in_heap(void * ptr) {
	if (_malloc_high_mark != NULL) return POINTER_IN_HEAP(ptr);
	else return (ulong_t)ptr > 0x100; // it unlikely that address < 0x100 belongs to heap
}

/**
 * Return cached pointer.
 * @return
 *   CACHED_NULL (-1) - if NULL value if cached.
 *   NULL - it did not find anything in cache.
 */
static const Dhead * cache_lookup(const void *ptr) {
    int i = (last_index + user_cache_max_size-1) % user_cache_max_size;
    // go backwards to search recent items first
    for (; i != last_index; i = (i+user_cache_max_size-1) % user_cache_max_size) {
        const Dhead * dh = cache[i].dhead;
        int size;
        if (dh==NULL)
            break;
        size = cache[i].size;
        if (size==0) {
            if (dh==ptr)
                return CACHED_NULL;
        } else {
            void * alloc = DHEAD_TO_POINTER(dh);
            unsigned int dist = ptr - alloc;
            if (dist==0 || (dist > 0 && dist < size)) {
                return dh;
            }
        }
    }
    return NULL;
}
/**
 * Update cache value with give ptr and dh.
 */
static void cache_update(const void * ptr, Dhead * dh) {
    pthread_mutex_lock(&mutex);
	if (user_cache_max_size>0) {
		if (dh!=NULL) {
			// if dh is not null save it and size
			// save size because header might be corrupted when we access it
			cache[last_index].dhead = dh;
			cache[last_index].size = _msize(DHEAD_TO_POINTER(dh));
		} else {
			// if dh is NULL save ptr as dhead and set size to 0 (caching non-heap pointer)
			cache[last_index].dhead = ptr;
			cache[last_index].size=0;
		}
		last_index = (last_index+1) % user_cache_max_size;
	}
    pthread_mutex_unlock(&mutex);
    //fprintf(stderr, "li=%d\n", last_index);
}
static struct  {
    sigjmp_buf buf;
    int tid;
}
saved_context;

static struct {
	struct sigaction sigsegv;
	struct sigaction sigbus;
} old_action;

static void uninstall_sig_handler() {
    sigaction(SIGSEGV, &(old_action.sigsegv), NULL);
    sigaction(SIGBUS,  &(old_action.sigbus),  NULL);
    old_action.sigsegv.sa_sigaction = SIG_DFL;
    old_action.sigbus.sa_sigaction  = SIG_DFL;
}
static void sig_handler(int signo, siginfo_t* info, void* other) {
    int tid = gettid();
    if (tid!=saved_context.tid) {
        abort(); // this should not happened, SIGSEGV is sent to thread that cause it
    }
    siglongjmp(saved_context.buf,1);
}

static void install_sig_handler() {
    struct sigaction act;
    struct sigaction old;
	act.sa_sigaction = sig_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &act, &old);
	if (old.sa_sigaction!=&sig_handler) { // override handler if not already
		old_action.sigsegv = old;
	}
	sigaction(SIGBUS, &act, &old);
	if (old.sa_sigaction!=&sig_handler) { // override handler if not already
		old_action.sigbus = old;
	}
}



static void update_range(const Dhead * dh, arena_range_t *range) {
    if(!(dh->d_debug.flag & M_INUSE)) {
        // memory freed
        range->r_ptr = dh;
        range->r_start = NULL;
        range->r_end = NULL;
        range->r_type = -1;
        range->un.r_arena = NULL;
    } else {
        find_malloc_range(DHEAD_TO_POINTER(dh), range);
    }
}

/**
 * Return cached pointer to dhead.
 * 3 optimizations are used:
 *   1) check that pointer belong to heap area (from min heap to max heap pointer)
 *   2) check magic number, this is unsafe operation so we have to save/restore context if failed
 *   3) finally look at linear cache, default is 32,
 *      but user can set size using MALLOC_USE_CACHE env var. When this var is set to 0 none of
 *      optimizations above are used, and linear search always performed on heap.
 * @return
 *   CACHED_NULL (-1) - if NULL value if cached.
 *   NULL - it did not find anything in cache or it is not a heap pointer at all.
 */
Dhead * mc_cache_get(const void * ptr, arena_range_t * range) {

    pthread_mutex_lock(&mutex);

	if (user_cache_max_size==0) {
		pthread_mutex_unlock(&mutex);
        return NULL;
	}

	if (!POINTER_IN_HEAP(ptr)) {
		pthread_mutex_unlock(&mutex);
        return CACHED_NULL;
    }



    // since we have no control over user pointer, some "magic check" operations
    // can be unsafe, to prevent SIGSEGV we install temp handler and recover if it
    // happens
    saved_context.tid = gettid();
    switch (sigsetjmp(saved_context.buf,1)) {
    case 0: {
            const Dhead	* dh;
            sigset_t oset, set;
            sigemptyset( &set );
            sigaddset( &set, SIGSEGV );
            sigaddset( &set, SIGBUS );
            pthread_sigmask(SIG_UNBLOCK, &set, &oset );
            install_sig_handler();
            dh	= POINTER_TO_DHEAD(ptr);
            if (MAGIC_MATCH(&(dh->d_debug))) {// unsafe pointer dereference
                // this is pointer to heap, we can use it to not do the search.
                // Currently magic is 52 bits, so it is pretty unlikely
                // it is going to meet it by accident
                if (range) {
                    update_range(dh,range);
                }
            } else {
                dh = cache_lookup(ptr);
                if (dh!=NULL && dh!=CACHED_NULL) {
                    if (POINTER_IN_HEAP(DHEAD_TO_POINTER(dh))
                            && MAGIC_MATCH(&(dh->d_debug))) { // unsafe pointer dereference
                        // if magic does not match header is corrupted, safer to return null
                        if (range) {
                            update_range(dh,range);
                        }
                    } else {
                        dh = NULL;
                    }
                }
            }
            sigprocmask(SIG_SETMASK, &oset, NULL); // restore thread mask
            uninstall_sig_handler(); // restore handler
            pthread_mutex_unlock(&mutex);
            return (Dhead *)dh;
        }
    case 1: {
            /* pointer was bad, and we jumped here from the signal handler */
            uninstall_sig_handler();
            pthread_mutex_unlock(&mutex);
            return CACHED_NULL;
        }
    }
    return NULL; // unreachable code
}
static unsigned int roundUpToPowerOf2(int i) {
	unsigned int power = 1;
	if (i<=0)
		return 0;
	while (i > power) {
		power = power << 1;
	}

	return power;
}

/**
 * Sets cache size, between 0 and 2048. Should be power of 2 otherwise would
 * be slower because of use of % operation on cache size.
 */
void mc_set_cache(int size) {
    pthread_mutex_lock(&mutex);
	// TODO: when cache size is reduced, some cache entries should be
	// released.
    if (size<0)
        user_cache_max_size = 0;
    else if (size>sizeof(cache))
        user_cache_max_size = sizeof(cache);
    else
        user_cache_max_size = roundUpToPowerOf2(size);
    pthread_mutex_unlock(&mutex);

}
/**
 * This should be called on malloc alloc to store recently
 * allocated memory address in cache
 */
void mc_cache_put_malloc(void * ptr) {
    pthread_mutex_lock(&mutex);
	if (_malloc_low_mark == NULL || ptr - _malloc_low_mark < 0) {
		_malloc_low_mark = ptr;
	}
	if (ptr - _malloc_high_mark > 0) {
		_malloc_high_mark = ptr + _msize(ptr);
	}
	if (user_cache_max_size > 0) {
		cache_update(ptr, POINTER_TO_DHEAD(ptr));
	}
    pthread_mutex_unlock(&mutex);
}
/**
 * This should be called on memory free
 */
void mc_cache_release(void * ptr) {
    cache_update(ptr, POINTER_TO_DHEAD(ptr));
}

/**
 * Update cache with pre-calculated values
 */
void mc_cache_update(const void *ptr, Dhead* dh) {
    if (dh==NULL) {
        cache_update(ptr, NULL);
    } else {
        cache_update(DHEAD_TO_POINTER(dh), dh);
    }
}

void mc_cache_clear() {
	int i;
    // reset cache
    for (i = 0; i < user_cache_max_size; ++i) {
		cache[i].dhead = 0;
		cache[i].size = 0;
	}
    last_index = 0;
}


int mc_get_cache_size() {
	return user_cache_max_size;
}
