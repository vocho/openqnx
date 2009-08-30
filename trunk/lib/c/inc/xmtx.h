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





/* xmtx.h internal header */
#ifndef _XMTX
#define _XMTX
#include <stdlib.h>
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

_C_LIB_DECL
typedef void *_Rmtx;

#ifdef __QNX__

#if (_LIBC_SO_VERSION == 2 || _LIBC_SO_VERSION == 3)
/*
 lib?cpp.so.2 contained its version of _Mtxinit that is incompatible with
 this version. Since it refers to libc.so.2, we change the name for now.
*/
#define _Mtxinit		_Mtxini		/* This is for compatibility with older libcpp versions */
#endif

 #if _IS_WRS	/* compiler test */

 #ifdef __RTP__
  #define _IS_WRS_RTP	1
/*
 * In the RTP world, the __diab_alloc_mutex function returns a pointer
 * to a mutex. Since the data structure is accessed through an opaque 
 * void * pointer, we cannot pass an address of a mutex variable - say mutx
 * and * expect the compiler to be able to do the following structure copy:
 * mutx = *(__diab_alloc_mutex())
 * Therefore, changing the prototype of the function for that initializes
 * mutexes.
 */
void _Mtxinit(_Rmtx **);

 #else /* __RTP__ */
void _Mtxinit(_Rmtx *);
 #endif /* __RTP__ */

void _Mtxdst(_Rmtx *);
void _Mtxlock(_Rmtx *);
void _Mtxunlock(_Rmtx *);


 #else /* _IS_WRS */
void _Mtxinit(_Rmtx *);
void _Mtxdst(_Rmtx *);
void _Mtxlock(_Rmtx *);
void _Mtxunlock(_Rmtx *);
 #endif /* _IS_WRS */


#include <stdio.h>
#include <pthread.h>
typedef struct
	{	/* data for recursive mutex */
	unsigned cnt;
	pthread_t	owner;
	pthread_mutex_t mtx;
	} _Rmtx_t;

#define MTXP(p)		(*(_Rmtx_t **)p)
#define MTX_OWNER(p)	(MTXP(p)->owner)

void _Unlockfilemtx(void);
void _Unlocksysmtx(void);
int  _Ftrylockfile(FILE *fp);
void _Releasefilelock(FILE *str);
#endif

 #if !_MULTI_THREAD
  #define _Mtxinit(mtx)
  #define _Mtxdst(mtx)
  #define _Mtxlock(mtx)
  #define _Mtxunlock(mtx)

typedef char _Once_t;

  #define _Once(cntrl, func)	if (*(cntrl) == 0) (func)(), *(cntrl) = 2
  #define _ONCE_T_INIT	0

 #elif _WIN32_C_LIB
typedef long _Once_t;

void _Once(_Once_t *, void (*)(void));
  #define _ONCE_T_INIT	0

 #elif _IS_WRS_RTP
  #undef _IS_WRS_RTP

typedef unsigned long __diab_atomic_level;
typedef _Rmtx __diab_mutex;

extern __diab_atomic_level __diab_atomic_enter(void);
extern void __diab_atomic_restore(__diab_atomic_level);
extern __diab_mutex __diab_alloc_mutex(void);
extern void __diab_free_mutex(__diab_mutex);
extern void __diab_lock_mutex(__diab_mutex);
extern void __diab_unlock_mutex(__diab_mutex);
extern void taskDelay(int);

typedef int _Once_t;
void _Once(_Once_t *, void (*)(void));
  #define _ONCE_T_INIT	0

#define _Mtxdst(x)	__diab_free_mutex(x)
#define _Mtxlock(x)	__diab_lock_mutex(x)
#define _Mtxunlock(x)	__diab_unlock_mutex(x)

 #elif _HAS_POSIX_C_LIB
  #include <setjmp.h>
  #include <time.h>

 #if 0x570 < __SUNPRO_CC && !defined(_RESTRICT_KYWD)	/* compiler test */
  #define _RESTRICT_KYWD restrict
 #endif /* 0x570 < __SUNPRO_CC etc. */

  #include <pthread.h>

typedef pthread_once_t _Once_t;

  #define _Once(cntrl, func)	pthread_once(cntrl, func)
  #define _ONCE_T_INIT	PTHREAD_ONCE_INIT

 #else /* library type */
  #error unknown library type
 #endif /* library type */

_END_C_LIB_DECL
#endif /* _XMTX */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("xmtx.h $Rev: 171092 $"); */
