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
/*
 * $Id: mallocint.h 162945 2008-02-08 18:42:25Z sboucher $
 */
#ifndef _MALLOCINT_H_
#define _MALLOCINT_H_

/*
 * this file contains macros that are internal to the malloc library
 * and therefore, are not needed in malloc.h.
 */

/*-
 * internal definitions for allocation library.
 */
#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <sys/storage.h>
#include <inttypes.h>

#ifndef MALLOC_PC
#define MALLOC_PC
#endif

#ifndef MALLOC_GUARD
#define MALLOC_GUARD
#endif

#define M_INUSE 	0x01
#define M_FREED 	0x02		/* RESERVED */
#define M_REFERENCED 	0x04
#define M_UNREFERENCED	0x08		/* File/line information available */
#define M_MAGIC 	0x03156100
#define M_MAGIC_BITS	0xffffff00
#define M_MAGIC2	0x1b95fd86

#define MAGIC_MATCH(p) (((p)->flag & M_MAGIC_BITS) == M_MAGIC && (p)->magic2 == M_MAGIC2)

#define M_FILL		'\01'
#define M_FREE_FILL	'\02'

#define MALLOC_INIT()	if( malloc_inited == 0 ) malloc_init()

/*
 * malloc_freeseg() operation arguments
 */
#define M_FREE_REMOVE	1
#define M_FREE_ADD	2
/*
 * Malloc types
 */
#define M_T_MALLOC	0x10
#define M_T_REALLOC	0x20
#define M_T_CALLOC	0x30
#define M_T_SPLIT	0x40
#define M_T_BITS	0x70

#define GETTYPE(_ptr)		(_ptr->flag & M_T_BITS)
#define SETTYPE(_ptr,_type) 	(_ptr->flag = ((_ptr->flag & ~M_T_BITS)|_type))


/*
 * For heap marking
 */
typedef struct mse {
    int mse_start;
    int mse_end;
} mse_t;

/* 
 * circular stack
 * top is next insertion point
 * empty: bot == top
 * full: (top + 1) % n == bot
 */
typedef struct mark_stack {
    int n; /* maximum number of entries + 1 */
    int size;
    int bot;
    int top;
    int overflow;
    mse_t *entries;
} mark_stack_t;

/*
 * For statistics counting
 */
struct _stat_bin {
    unsigned  long size;
    unsigned  long nallocs;
    unsigned  long nfrees;
};
extern const unsigned __n_stat_bins;
extern struct _stat_bin __stat_bins[];

extern int __malloc_bt_depth;
extern int __malloc_error_btdepth;
extern int __malloc_trace_minsz;
extern int __malloc_trace_maxsz;
extern int __malloc_use_dladdr;
extern struct qtime_entry *__malloc_qtp;
extern const char **_argv;
extern unsigned int *__cdbt[]; \

/*
 * prototypes
 */

#if defined(__STDC__) || defined(__cplusplus)
# define __stdcargs(s) s
#else
# define __stdcargs(s) ()
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <backtrace.h>

#define libmalloc_caller() ({ \
    bt_addr_t _bt[2]={0,0}; \
    btl_get_backtrace(&btl_acc_self,_bt,2); \
    _bt[1]; \
    })
    
#if defined(__X86__)
  static __inline__ int __attribute__((__unused__)) GET_CPU(void) {
    register  int           __cpu;
    __asm__ (
      "movl %%fs:12,%0"
      : "=r" (__cpu):);
    return __cpu;
  }
#elif defined(__ARM__)
#define GET_CPU() (0)
#else
#define GET_CPU()       (_cpupage_ptr->cpu)
#endif

#define MYCLOCKCYCLES() (__malloc_qtp->nsec)

#include "builtinbt.h"

#define MALLOC_PTR_FILL_DEBUG(__dd, __ptr) \
	{ \
		__dd = __malloc_add_dbg_info(__dd, (GET_CPU()+1), \
                  pthread_self(), (MYCLOCKCYCLES()), __ptr); \
	}

#define MALLOC_FILLCALLERD(__cptr, __cd, __line) \
{ \
  if ((__cptr) && (__malloc_bt_depth > 0)) { \
    int __i; \
    int __j; \
		int __tl; \
    Flink     * __flink; \
    DebugInfo_t   *__ptr; \
    /* this should be fine, since the allocator returns
    values that are aligned */ \
		__tl = __line; \
    __flink = (Flink *)((Dhead *)__cptr - 1); \
    __ptr = &(((Dhead *)__flink)->d_debug); \
    for (__i=0; __i < CALLERPCDEPTH; __i++) { \
		if (__cd[__i] == (unsigned int *)__tl) \
			break; \
	} \
    for (__j=__i; __j < CALLERPCDEPTH; __j++) { \
		if (__cd[__j] == 0) \
			break; \
		if (__j-__i < __malloc_bt_depth) \
			__malloc_add_dbg_bt(__ptr->dbg, __cd[__j]); \
    } \
    /* must set the CRC again */ \
    /* ((Dhead *)__flink)->d_crc = LinkCRC(__flink); */ \
  } \
}

#define MALLOC_TRACE_FCHECK(__sz) \
	(((__malloc_trace_minsz == -1) || (__sz >= __malloc_trace_minsz)) && \
	 ((__malloc_trace_maxsz == -1) || (__sz <= __malloc_trace_maxsz))) ? \
	 1 : 0
	

/*
 * perform string copy, but make sure that we don't go beyond the
 * end of the
 * buffer (leaving room for trailing info (5 bytes) like the
 * return and a null
 */
#define COPY(s,t,buf,len) while( (*(s) = *((t)++) ) \
               && ( (s) < ((buf)+(len)-5) ) ) { (s)++; }

#define MALLOCG_VERSION "1.2"
#define START_STR "= START:" MALLOCG_VERSION
#define END_STR   "= END:" MALLOCG_VERSION
#define STARTEV_STR  "STARTEV:" MALLOCG_VERSION
/*#define ENDEV_STR  "FINI:" MALLOCG_VERSION */
#define ENDEV_STR  "ENDEV:" MALLOCG_VERSION

#define M_WARN  1
#define M_ERROR 2
#define M_FATAL 3
#define M_LEAK  4


#if !defined(__cplusplus)
void		  exit __stdcargs((int));
char		* getenv __stdcargs((const char *));
#else
void std::exit(int);
char *std::getenv(const char *);
#endif

/* malloc.c */
void *_malloc __stdcargs((size_t size));
void *_malloc_pc __stdcargs((size_t size, unsigned int *pc));
void malloc_aligned_memset __stdcargs((void *ptr, int byte, register int len));
void malloc_safe_memset __stdcargs((void *ptr, int byte, register int len));
void malloc_fatal __stdcargs((const char *funcname, const char *file, int line, const void *mptr));
void malloc_warning __stdcargs((const char *funcname, const char *file, int line, const void *mptr));
void malloc_dump_info_block __stdcargs((const void *mptr, int id));
char *malloc_int_suffix __stdcargs((long i));
void malloc_dump_line(int fd, const char *file, unsigned long line, const char *prefix);
/* free.c */
void _free __stdcargs((void *cptr));
/* realloc.c */
void *_realloc __stdcargs((void *cptr, size_t size));
/* calloc.c */
void *_calloc __stdcargs((size_t nelem, size_t elsize));
/* string.c */
/* malloc_chk.c */
int find_malloc_area __stdcargs((const void *ptr, arena_range_t *range));
Block *find_malloc_block __stdcargs((const void *ptr));
Arena *find_malloc_arena __stdcargs((const void *ptr));
void *find_malloc_ptr __stdcargs((const void *ptr, arena_range_t *range));
Dhead *find_malloc_range __stdcargs((const void *ptr, arena_range_t *range));
void malloc_verify __stdcargs((const char *func, const char *file, int line, const arena_range_t *range, const void *ptr, size_t len));
ssize_t _msize __stdcargs((void *cptr));
char *_mptr __stdcargs((const char *cptr));
int malloc_check_neighbors(const char *func,const char *file, int line, arena_range_t *range, Dhead *flink);
int malloc_check_fill(const char *func,const char *file, int line, Dhead *flink);
/* malloc_chn.c */
/* malloc_gc.c */
int DBmalloc_mark(const char *file, int line, int todo);
int DBFmalloc_mark(const char *func, const char *file, int line, ulong_t top, int todo);
int malloc_dump_unreferenced(int fd, int todo);
int DBmalloc_dump_unreferenced(int fd, const char *file, int line, int todo);
/* memory.c */
/* tostring.c */
int tostring __stdcargs((char *buf, unsigned long val, int len, int base, int fill));
int tostring64 __stdcargs((char *buf, uint64_t val, int len, int base, int fill));
/* timestamp.c */
void _reset_timestamp __stdcargs((void));
uint64_t _get_timestamp __stdcargs((void));
/* m_perror.c */
void malloc_perror __stdcargs((char *str));
/* m_init.c */
void malloc_init __stdcargs((void));
/* mallopt.c */
/* dump.c */
void malloc_list __stdcargs((int fd, unsigned long histid1, unsigned long histid2));
void malloc_list_items __stdcargs((int fd, int list_type, unsigned long histid1, unsigned long histid2));
/* leak.c */
unsigned long malloc_size __stdcargs((unsigned long *histptr));
unsigned long DBmalloc_size __stdcargs((const char *file, int line, unsigned long *histptr));
/* process.c */
void * malloc_stack_top();
extern void *__debug_malloc __stdcargs((const char *file, int line, size_t size));
extern void *__debug_calloc __stdcargs((const char *file, int line, size_t nelem, size_t elsize));
extern void *__debug_realloc __stdcargs((const char *file, int line, void *ptr, size_t size));
extern void __debug_free __stdcargs((const char *file, int line, void *ptr));
/* malloc_cache.c */
void mc_set_cache(int size);

#ifdef __cplusplus
};
#endif

#undef __stdcargs


/*
 * global variables
 */
extern int		  malloc_inited;
extern int		  _malloc_check_on; /* define in c/alloc */
extern int		  malloc_verify_alloc;
extern int		  malloc_verify_access;
extern int		  malloc_verify_access_level;
extern int		  malloc_errfd;
extern int		  malloc_eventfd;
extern int		  malloc_errno;
extern char		* malloc_err_strings[];
extern int		  malloc_fatal_level;
extern ulong_t	  malloc_hist_id;
extern int		  malloc_warn_level;
extern int		  _malloc_fill_area;
extern int		  malloc_verbose;
extern int		  _malloc_free_check; /* define in c/alloc/dlist.c */
extern unsigned	  _amblksize;
extern ulong_t	  _malloc_start;
extern ulong_t	  _malloc_end;
extern int		__malloc_show_links;
extern char __malloc_trace_filename[];
extern int __malloc_detail;
extern int		  malloc_handle_signals;

extern void (*__malloc_init_hook)();
extern void * (*__malloc_hook)(const char *file, int line, size_t size);
extern void * (*__calloc_hook)(const char *file, int line, size_t nelems, size_t size);
extern void * (*__realloc_hook)(const char *file, int line, void *ptr, size_t size);
extern void (*__free_hook)(const char *file, int line, void *ptr);

#ifndef MALLOC_WRAPPER
#ifndef weak_alias
#define weak_alias(alias,sym) \
    __asm__(".weak " #alias " ; " #alias " = " #sym);
#endif

weak_alias(malloc,_malloc)
weak_alias(malloc_pc,_malloc_pc)
weak_alias(memalign,_memalign)
weak_alias(memalign_pc,_memalign_pc)
weak_alias(posix_memalign,_posix_memalign)
weak_alias(realloc,_realloc)
weak_alias(calloc,_calloc)
weak_alias(free,_free)
weak_alias(mallopt,_mallopt)
weak_alias(mprobe,_mprobe)

#define malloc	_malloc
#define memalign	_memalign
#define memalign_pc	_memalign_pc
#define posix_memalign	_posix_memalign
#define realloc	_realloc
#define calloc	_calloc
#define free	_free
#define malloc_pc	_malloc_pc
#define mallopt	_mallopt
#define mprobe	_mprobe

#define in_malloc()	in_malloc_code
#define in_string()	in_string_code
#else
#include <pthread.h>
extern pthread_mutex_t	_mallocg_lock;
extern pthread_key_t	_mallocg_in_malloc;
extern pthread_key_t	_mallocg_in_string;
#define ENTER()	pthread_mutex_lock(&_mallocg_lock);
#define LEAVE()	pthread_mutex_unlock(&_mallocg_lock);
#define in_malloc()	(int)pthread_getspecific(_mallocg_in_malloc)
#define in_string()	(int)pthread_getspecific(_mallocg_in_string)
#define set_in_malloc(x) pthread_setspecific(_mallocg_in_malloc,(void *)(x))
#define set_in_string(x) pthread_setspecific(_mallocg_in_string,(void *)(x))
#endif

#define SB_OVERHEAD()    __ROUND((sizeof(ListNode)), _MALLOC_ALIGN)

#endif /* _MALLOCINT_H_ */
