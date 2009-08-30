/* xmtx.c -- mutex support */
#include <stdio.h>
#include "xmtx.h"


 #if !_MULTI_THREAD || _WIN32_C_LIB && __EDG__ && !__INTEL_COMPILER

 #elif _WIN32_C_LIB
  #include "wrapwin.h"

/* Win32 critical sections are recursive, but
   Win32 does not have once-function */

  #define MTXP(p)	(*(CRITICAL_SECTION **)p)

void _Once(_Once_t *_Cntrl, void (*_Func)(void))
	{	/* execute _Func exactly one time */
	_Once_t old;
	if (*_Cntrl == 2)
		;
	else if ((old = InterlockedExchange(_Cntrl, 1)) == 0)
		{	/* execute _Func, mark as executed */
		_Func();
		*_Cntrl = 2;
		}
	else if (old == 2)
		*_Cntrl = 2;
	else
		while (*_Cntrl != 2)
			Sleep(1);
	}

void (_Mtxinit)(_Rmtx *_Mtx)
	{	/* initialize mutex */
	MTXP(_Mtx) = (CRITICAL_SECTION *)malloc(sizeof (CRITICAL_SECTION));
	InitializeCriticalSection(MTXP(_Mtx));
	}

void (_Mtxdst)(_Rmtx *_Mtx)
	{	/* delete mutex */
	DeleteCriticalSection(MTXP(_Mtx));
	free(MTXP(_Mtx));
	}

void (_Mtxlock)(_Rmtx *_Mtx)
	{	/* lock mutex */
	EnterCriticalSection(MTXP(_Mtx));
	}

void (_Mtxunlock)(_Rmtx *_Mtx)
	{	/* unlock mutex */
	LeaveCriticalSection(MTXP(_Mtx));
	}

  #else /* _HAS_POSIX_C_LIB */
/* pthread mutexes are not, in general, recursive, but
   pthreads library does support once-functions */

#ifndef __QNX__
typedef struct
	{	/* data for recursive mutex */
	unsigned cnt;
	pthread_t owner;
	pthread_mutex_t mtx;
	} _Rmtx_t;

  #define MTXP(p)	(*(_Rmtx_t **)p)
#endif

 #if _HAS_DINKUM_CLIB
  #define NLOCKS	(_MAX_LOCK + FOPEN_MAX)

 #else /* _HAS_DINKUM_CLIB */
  #define NLOCKS	_MAX_LOCK
 #endif /* _HAS_DINKUM_CLIB */

static _Rmtx_t locks[NLOCKS];
static size_t lockno = {0};

void (_Mtxinit)(_Rmtx *_Mtx)
	{	/* initialize mutex */
	if (lockno < NLOCKS)
		MTXP(_Mtx) = &locks[lockno++];
	else
		MTXP(_Mtx) = (_Rmtx_t *)malloc(sizeof (_Rmtx_t));
	MTXP(_Mtx)->cnt = 0;
#ifndef __QNX__
	MTXP(_Mtx)->owner = 0;
#endif
	pthread_mutex_init(&MTXP(_Mtx)->mtx, 0);
	}

void (_Mtxdst)(_Rmtx *_Mtx)
	{	/* delete mutex */
	_Rmtx_t *p = MTXP(_Mtx);

	pthread_mutex_destroy(&p->mtx);
	if (p < &locks[0] || &locks[NLOCKS] <= p)
		free(p);
	}

void (_Mtxlock)(_Rmtx *_Mtx)
	{	/* lock mutex */
	pthread_t thr = pthread_self();

	if (MTXP(_Mtx)->cnt && pthread_equal(MTXP(_Mtx)->owner, thr))
		++MTXP(_Mtx)->cnt;
	else if (pthread_mutex_lock(&MTXP(_Mtx)->mtx) == 0)
		{	/* mark this thread as owner */
		MTXP(_Mtx)->owner = thr;
		++MTXP(_Mtx)->cnt;
		}
	}

void (_Mtxunlock)(_Rmtx *_Mtx)
	{	/* unlock mutex */
	if (--MTXP(_Mtx)->cnt == 0)
		pthread_mutex_unlock(&MTXP(_Mtx)->mtx);
	}
 #endif /* !_MULTI_THREAD etc. */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xmtx.c $Rev: 153052 $");
