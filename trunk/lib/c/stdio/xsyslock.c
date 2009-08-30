/* xsyslock.c -- thread locking and unlocking functions */
#include <stdio.h>
#include "xmtx.h"
#ifdef __QNX__
#include <errno.h>
#endif

 #if defined(__BORLANDC__)
  #pragma warn -par
 #endif /* defined(__BORLANDC__) */

_STD_BEGIN

 #if !_MULTI_THREAD
void (_Locksyslock)(int lockno)
	{	/* set a system lock */
	}

void (_Unlocksyslock)(int lockno)
	{	/* clear a system lock */
	}

 #else /* _MULTI_THREAD */
  #define INIT	_Once(&syslock_o, _Initlocks)
  #define TIDY	_CSTD _Atexit(&_Clearlocks)

static _Once_t syslock_o = _ONCE_T_INIT;

 #if _HAS_DINKUM_CLIB
  #define _MAX_FLOCK	FOPEN_MAX

#ifdef __QNX__

typedef struct _file_list {
	_Rmtx *_Flock;
	struct _file_list *_Fnext;
} _file_list;

static int _file_list_count;
static _Rmtx _Flist_mtx;
struct _file_list *_Flist_head = NULL;

#else /* __QNX__ */

static _Rmtx file_mtx[_MAX_FLOCK];
static int max_flock = 0;

#endif /* __QNX__ */
 #endif /* _HAS_DINKUM_CLIB */

static int max_lock = 0;
static _Rmtx mtx[_MAX_LOCK];

#ifdef __QNX__

/* We don't need the atexit handlers */
#undef TIDY
#define TIDY

#define UNLOCK_MTX(_Mtx, thr) \
if(MTXP(_Mtx) && pthread_equal(MTX_OWNER(_Mtx), thr)) { \
	if (MTXP(_Mtx)->cnt) { \
		pthread_mutex_unlock(&MTXP(_Mtx)->mtx); \
		MTXP(_Mtx)->cnt   = 0; \
	} \
}

void _Unlockfilemtx() {
	FILE *fp;
	pthread_t thr = pthread_self();

	_Locksyslock(_LOCK_STREAM);
	for (fp = _Files[0]; fp; fp = fp->_NextFile) {
		UNLOCK_MTX(&fp->_Flock, thr);
	}
	_Unlocksyslock(_LOCK_STREAM);
}

void _Unlocksysmtx() {
	int   loop;
	_Rmtx *_Mtx;
	pthread_t thr = pthread_self();

	for (loop = 0; loop < _MAX_LOCK; ++loop) {
		_Mtx = &mtx[loop];
		UNLOCK_MTX(_Mtx, thr);
	}
}

static void _Lockfilealloc(FILE *str) {
	struct _file_list *fptr;
	_Rmtx *_Mtx;

	_Mtxlock(&_Flist_mtx);
	// somebody might have jumped in and grabbed a lock for us before 
	// we locked, so test str->_Flock again.  Note I'm being
	// paranoid here since the lock should be allocated before fdopen
	// returns and so it should only be possible for one thread to be
	// doing this (since fdopen must return before another thread can
	// get hold of the str pointer).  But paranoia is good...
	if ( str->_Flock == NULL ) {
		if (_Flist_head->_Fnext == NULL) // make lock
		{
			_Mtxinit(&_Mtx);
			str->_Flock = _Mtx;
			_Mtxunlock(&_Flist_mtx);
		}
		else // just take one
		{
			fptr = _Flist_head->_Fnext;
			_Flist_head->_Fnext = fptr->_Fnext;
			fptr->_Fnext = NULL;
			str->_Flock = fptr->_Flock;
			_file_list_count--;
			_Mtxunlock(&_Flist_mtx);
			free(fptr);
		}
	} else {
		_Mtxunlock(&_Flist_mtx);
	}
}

int _Ftrylockfile(FILE *fp) {
	int       ret = EOK;
	pthread_t tid = pthread_self();
	_Rmtx **_Mtx;

	if (fp != 0)
	{
		if (fp->_Flock == NULL)	// need to acquire lock
		{
			_Lockfilealloc(fp);
		}
		_Mtx = (_Rmtx **)&fp->_Flock;

		if (MTXP(_Mtx)->cnt && pthread_equal(MTX_OWNER(_Mtx), tid)) {
			++MTXP(_Mtx)->cnt;
		} else if ((ret = pthread_mutex_trylock(&MTXP(_Mtx)->mtx)) == EOK) {
			++MTXP(_Mtx)->cnt;
		}
	}

	return (ret);
}
#endif

void _Clearlocks(void)
	{	/* clear all locks at program termination */
#ifdef __QNX__
	/* note that this will only clear locks left in lock queue */
	struct _file_list *fptr, *nextptr;

	fptr = _Flist_head;

	while (fptr)
	{
		nextptr = fptr->_Fnext;
		if (fptr->_Flock)
			_Mtxdst(fptr->_Flock);
		free(fptr);
		fptr = nextptr;
	}

	_file_list_count = 0;

#else /* __QNX__ */

	int count;

 #if _HAS_DINKUM_CLIB
	max_flock = 0;

	for (count = 0; count < _MAX_FLOCK; ++count)
		_Mtxdst(&file_mtx[count]);
 #endif /* _HAS_DINKUM_CLIB */

	max_lock = 0;

	for (count = 0; count < _MAX_LOCK; ++count)
		_Mtxdst(&mtx[count]);
#endif /* __QNX__ */
	}

void _Initlocks(void)
	{	/* initialize all locks on first call to _Locksyslock */
	int count;

#ifdef __QNX__

	struct _file_list *fptr, *tmpptr;

	fptr = _Flist_head;

	while (fptr != NULL)
	{
		tmpptr = fptr->_Fnext;
		if (fptr->_Flock != NULL)
		{
			_Mtxdst(fptr->_Flock);
		}
		free(fptr);
		fptr = tmpptr;
	}

	_Flist_head = malloc(sizeof(struct _file_list));
	_Flist_head->_Fnext = NULL;
	_Mtxinit(&_Flist_mtx);
#else /* __QNX__ */

  #if _WIN32_C_LIB
	count = 0;
	if (count != 0)
		{	/* drag in our tolower, but don't force TLS setup */
		int (tolower)(int);

		(tolower)('A');	/* block linking of conflicting code */
		}
  #endif /* _WIN32_C_LIB */

 #if _HAS_DINKUM_CLIB
	for (count = 0; count < _MAX_FLOCK; ++count)
		_Mtxinit(&file_mtx[count]);
	max_flock = _MAX_FLOCK;
 #endif /* _HAS_DINKUM_CLIB */
#endif /* __QNX__ */

	for (count = 0; count < _MAX_LOCK; ++count)
		_Mtxinit(&mtx[count]);
	max_lock = _MAX_LOCK;

	TIDY;
	}

 #if _HAS_DINKUM_CLIB

#ifdef __QNX__

void (_Releasefilelock)(FILE *str)
{
	struct _file_list *fptr, *tmpptr;

	if (str != 0)
	{
		_Mtxlock(&_Flist_mtx);
		if (_file_list_count < _MAX_FLOCK)
		{
			fptr = _Flist_head;
			tmpptr = fptr->_Fnext;

			fptr->_Fnext = malloc(sizeof(struct _file_list));
			fptr = fptr->_Fnext;
			fptr->_Fnext = tmpptr;
			fptr->_Flock = str->_Flock;
			_file_list_count++;
			_Mtxunlock(&_Flist_mtx);
		}
		else
		{
			_Mtxunlock(&_Flist_mtx);
			_Mtxdst(str->_Flock);
		}
	}
}
#endif /* __QNX__ */

void (_Lockfilelock)(FILE *str)
	{	/* set a file lock */

	INIT;

#ifndef __QNX__

	if (str != 0 && str->_Idx < max_flock)
		_Mtxlock(&file_mtx[str->_Idx]);

#else /* !__QNX__ */

	if (str != 0)
	{
		if (str->_Flock == NULL) // need to acquire lock
		{
			_Lockfilealloc(str);
		}
		_Mtxlock(&str->_Flock);
	}
#endif /* !__QNX__ */
	}

void (_Unlockfilelock)(FILE *str)
	{	/* clear a system lock */
#ifndef __QNX__
	if (str != 0 && str->_Idx < max_flock)
		_Mtxunlock(&file_mtx[str->_Idx]);
#else /* !__QNX__ */
	if (str != 0)
		_Mtxunlock(&str->_Flock);
#endif /* !__QNX__ */
	}
 #endif /* _HAS_DINKUM_CLIB */

void (_Locksyslock)(int lockno)
	{	/* set a system lock */
	INIT;
	if (lockno < max_lock)
		_Mtxlock(&mtx[lockno]);
	}

void (_Unlocksyslock)(int lockno)
	{	/* clear a system lock */
	if (lockno < max_lock)
		_Mtxunlock(&mtx[lockno]);
	}
 #endif /* _MULTI_THREAD */

_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xsyslock.c $Rev: 209305 $");
