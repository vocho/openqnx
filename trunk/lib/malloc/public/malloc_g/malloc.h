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
 *  malloc.h	Memory allocation functions
 *
 */
#ifndef _MALLOC_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _MALLOC_H_INCLUDED
#endif

#ifndef _MALLOC_H_DECLARED
#define _MALLOC_H_DECLARED

#include <_pack64.h>

_C_STD_BEGIN

#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif

_C_STD_END

#if defined(__EXT_QNX) && !defined(_alloca)
 #ifndef _ALLOCA_H_INCLUDED
  #include <alloca.h>
 #endif
#endif


__BEGIN_DECLS
_C_STD_BEGIN
extern void *calloc(size_t __n, size_t __size);
extern void *malloc(size_t __size);
extern void *realloc(void *__ptr, size_t __size);
extern void free(void *__ptr);
_C_STD_END

#if defined(__EXT_QNX)		/* Approved 1003.1d D14 */
extern int posix_memalign(void **__memptr, _CSTD size_t __alignment, _CSTD size_t __size);
#endif

#if defined(__EXT_QNX)
extern int cfree(void *__ptr);
extern void *_scalloc(_CSTD size_t __size);
extern void *_smalloc(_CSTD size_t __size);
extern void *_srealloc(void *__ptr, _CSTD size_t __old_size, _CSTD size_t __new_size);
extern void _sfree(void *__ptr, _CSTD size_t __size);
#endif

#if defined(__EXT_QNX)		/* SVID/XPG/ELIX functions */
/* This shouldn't really be an enum for C++ compatibility */
enum malloc_opt_cmds {
	MALLOC_VERIFY,
	MALLOC_VERIFY_ON,
	MALLOC_STATS,
	MALLOC_FREE_CHECK,
	MALLOC_ARENA_SIZE,
	MALLOC_MONOTONIC_GROWTH,
  MALLOC_MEMORY_HOLD,
  MALLOC_ARENA_CACHE_MAXSZ,
  MALLOC_ARENA_CACHE_MAXBLK,
  MALLOC_ARENA_CACHE_FREE_NOW,
	MALLOC_LAST
};

/* Don't enum these, so we don't get coercion errors if the caller is C++ */
#define M_MXFAST	(int)MALLOC_LAST
#define M_NLBLKS	(int)(MALLOC_LAST+1)
#define M_GRAIN		(int)(MALLOC_LAST+2)
#define M_TRIM_THRESHOLD	(int)(MALLOC_LAST+3)
#define M_TOP_PAD	(int)(MALLOC_LAST+4)
#define M_MMAP_THRESHOLD	(int)(MALLOC_LAST+5)
#define M_MMAP_MAX	(int)(MALLOC_LAST+6)

struct malloc_stats {
	unsigned	m_small_freemem;/* memory in free small blocks */
	unsigned	m_freemem;	/* memory in free big blocks */
	unsigned	m_small_overhead;/* space in header block headers */
	unsigned	m_overhead;	/* space used by block headers */
	unsigned	m_small_allocmem;/* space in small blocks in use */
	unsigned	m_allocmem;	/* space in big blocks in use */
	unsigned	m_coreallocs;	/* number of core allocations performed */
	unsigned	m_corefrees;	/* number of core de-allocations performed */
	unsigned	m_heapsize;	/* size of the arena */
	unsigned	m_frees;	/* number of frees performed */
	unsigned	m_allocs;	/* number of allocations performed */
	unsigned	m_reallocs;	/* number of realloc functions performed */
	unsigned	m_small_blocks;	/* number of small blocks */
	unsigned	m_blocks;	/* number of big blocks */
	unsigned	m_hblocks;	/* number of header blocks */
};

struct mallinfo {
	int	arena;	/* size of the arena */
	int	ordblks;/* number of big blocks in use */
	int	smblks;	/* number of small blocks in use */
	int	hblks;	/* number of header blocks in use */
	int	hblkhd;	/* space in header block headers */
	int	usmblks;/* space in small blocks in use */
	int	fsmblks;/* memory in free small blocks */
	int	uordblks;/* space in big blocks in use */
	int	fordblks;/* memory in free big blocks */
	int	keepcost;/* penalty if M_KEEP is used -- not used */
};

enum mcheck_status {
    MCHECK_DISABLED = -1, MCHECK_OK = 0, MCHECK_HEAD, MCHECK_TAIL, MCHECK_FREE
};

extern struct mallinfo mallinfo(void);
extern int mallopt(int __cmd, int __value);
extern enum mcheck_status mprobe(void *__ptr);
extern int mcheck(void (*__abort_fn)(enum mcheck_status __status));

extern void *memalign(_CSTD size_t __alignment, _CSTD size_t __size);
extern void *valloc(_CSTD size_t __size);
#endif
__END_DECLS

#include <_packpop.h>

#include <_packpop.h>
#ifndef MALLOC_DEBUG
#define MALLOC_DEBUG
#endif 
#ifndef MALLOC_PC
#define MALLOC_PC
#endif
#ifndef MALLOC_GUARD
#define MALLOC_GUARD
#endif
#include <malloc_g/malloc-lib.h>

#endif

#ifdef _STD_USING
using std::malloc; using std::calloc;
using std::free; using std::realloc;
#endif

#ifdef __cplusplus
#ifndef CHECKED_PTR_T
#define CHECKED_PTR_T

typedef struct Dhead Dhead;

template <class T> class CheckedPtr {
protected:
    T *bp;
    T *cp;
    _CSTD size_t siz;
public:
    CheckedPtr(void) {bp = NULL; cp = NULL; siz = -1;}
    CheckedPtr(const CheckedPtr<T> &p) {bp = p.bp; cp = p.cp; siz = p.siz;}
    CheckedPtr(T *p);
    CheckedPtr<T> &operator=(T *p);
    CheckedPtr<T> &operator++() { cp++; return *this; }
    CheckedPtr<T> &operator--() { cp--; return *this; }
    CheckedPtr<T> operator++(int)	{ CheckedPtr<T> temp(*this); cp++; return temp; }
    CheckedPtr<T> operator--(int)	{ CheckedPtr<T> temp(*this); cp--; return temp; }
    CheckedPtr<T> &operator+=(const CheckedPtr<T> &p) { cp += p.cp; }
    CheckedPtr<T> &operator-=(const CheckedPtr<T> &p) { cp -= p.cp; }
    bool operator==(const T *p) { return(cp == p); }
    bool operator==(const CheckedPtr<T> p) { return(cp == p.cp); }
    T &operator*();
    T &operator[](int i);
    T *operator->();
    operator void*() {return cp;}
};

template<class T> CheckedPtr<T>::CheckedPtr(T *p)
{
    char *mptr;
    if ((mptr = _mptr((char *)p)) != NULL) {
        bp = (T *)mptr;
        cp = p;
        siz = _musize((char *)bp);
    }
    else {
        cp = p;
        siz = -1;
    }
}

template<class T> CheckedPtr<T> & CheckedPtr<T>::operator =(T *p)
{
    void *mptr;
    if ((mptr = _mptr((char *)p)) != NULL) {
        bp = (T *)mptr;
        cp = p;
        siz = _musize((char *)bp);
    }
    else {
        bp = p;
        cp = p;
        siz = -1;
    }
    return *this;
}

extern void *libmalloc_caller_fn(void);

template<class T>  T & CheckedPtr<T>::operator*() 
{
    if (cp == NULL) {
       void *line = (void *)libmalloc_caller_fn();
       malloc_warning("CheckedPtr<T>::operator*()", NULL, (int)line, NULL);
    }
    if (siz > 1) {
        if (cp < bp || cp >= (T *)((char *)bp + siz)) {
            void *line = (void *)libmalloc_caller_fn();
	    malloc_warning("CheckedPtr<T>::operator*()", NULL, (int)line, (void *)((Dhead *)bp-1));
        }
    }
    return *cp;
}

template<class T>  T * CheckedPtr<T>::operator->() 
{
    if (cp == NULL) {
       void *line = (void *)libmalloc_caller_fn();
       malloc_warning("CheckedPtr<T>::operator->()", NULL, (int)line, NULL);
    }
    if (siz > 1) {
        if (cp < bp || cp >= (T *)((char *)bp + siz)) {
            void *line = (void *)libmalloc_caller_fn();
	    malloc_warning("CheckedPtr<T>::operator->()", NULL, (int)line, (void *)((Dhead *)bp-1));
        }
    }
    return cp;
}

template<class T>  T & CheckedPtr<T>::operator[](int i) 
{
    T *np = cp + i;
    if (cp == NULL) {
       void *line = (void *)libmalloc_caller_fn();
       malloc_warning("CheckedPtr<T>::operator[]()", NULL, (int)line, NULL);
    }
    if (siz > 1) {
        if (np < bp || np >= (T *)((char *)bp + siz)) {
            void *line = (void *)libmalloc_caller_fn();
	    malloc_warning("CheckedPtr<T>::operator[]()", NULL, (int)line, (void *)((Dhead *)bp-1));
        }
    }
    return cp[i];
}
#endif /* CHECKED_PTR_T */
#endif /* __cplusplus */


#endif /* _MALLOC_H_INCLUDED */
