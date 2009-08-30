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
 *  pthread.h	Thread definitions
 *

 */
#ifndef _PTHREAD_H_INCLUDED
#define _PTHREAD_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __CPUINLINE_H_INCLUDED
#include <sys/cpuinline.h>
#endif

#if !defined(__EXT_POSIX1_199309) && (defined(__EXT_POSIX1_198808) || defined(__EXT_POSIX1_199009))
#error POSIX Threads needs P1003.1b-1993 or later
#endif

#ifndef _TIME_H_INCLUDED
 #include <time.h>
#endif

#ifndef _SCHED_H_INCLUDED
 #include <sched.h>
#endif

#ifndef __STORAGE_H_INCLUDED
 #include <sys/storage.h>
#endif

#if defined(__PTHREAD_T)
typedef __PTHREAD_T		pthread_t;
#undef __PTHREAD_T
#endif

#if defined(__SYNC_T)
typedef __SYNC_T		sync_t;
#undef __SYNC_T
#endif

#if defined(__PTHREAD_MUTEX_T)
typedef __PTHREAD_MUTEX_T	pthread_mutex_t;
#undef __PTHREAD_MUTEX_T
#endif

#if defined(__PTHREAD_COND_T)
typedef __PTHREAD_COND_T	pthread_cond_t;
#undef __PTHREAD_COND_T
#endif

#if defined(__SYNC_ATTR_T)
typedef __SYNC_ATTR_T	sync_attr_t;
#undef __SYNC_ATTR_T
#endif

#if defined(__PTHREAD_MUTEXATTR_T)
typedef __PTHREAD_MUTEXATTR_T	pthread_mutexattr_t;
#undef __PTHREAD_MUTEXATTR_T
#endif

#if defined(__PTHREAD_CONDATTR_T)
typedef __PTHREAD_CONDATTR_T	pthread_condattr_t;
#undef __PTHREAD_CONDATTR_T
#endif


#if defined(__SCHED_PARAM_T)
typedef __SCHED_PARAM_T	sched_param_t;
#undef __SCHED_PARAM_T
#endif

#if defined(__PTHREAD_ATTR_T)
typedef __PTHREAD_ATTR_T	pthread_attr_t;
#undef __PTHREAD_ATTR_T
#endif

#if defined(__EXT_POSIX1_200112)		/* Approved 1003.1j */

#if defined(__CLOCKID_T)	/* Needed for cond_get/setclock */
typedef __CLOCKID_T	clockid_t;
#undef __CLOCKID_T
#endif

#endif

#include _NTO_CPU_HDR_(smpxchg.h)

#ifndef __mutex_smp_cmpxchg
	#define __mutex_smp_cmpxchg(d, c, s)	_smp_cmpxchg((d), (c), (s))
#endif

#ifndef __mutex_smp_xchg
	#define __mutex_smp_xchg(d, n)	_smp_xchg((d), (n))
#endif

__BEGIN_DECLS

#include <_pack64.h>

/*
 * Synchronization manifests and structures.
 */
/* On count */
#define _NTO_SYNC_NONRECURSIVE	0x80000000	/* mutexes */
#define _NTO_SYNC_NOERRORCHECK	0x40000000	/* mutexes */
#define _NTO_SYNC_PRIOCEILING	0x20000000	/* mutexes */
#define _NTO_SYNC_PRIONONE		0x10000000	/* mutexes */
#define _NTO_SYNC_COUNTMASK		0x00ffffff	/* mutexes */
/* On owner */
#define _NTO_SYNC_MUTEX_FREE	0x00000000	/*    0  mutexes, and old cond, sem, spin */
#define _NTO_SYNC_WAITING		0x80000000	/* Top bit used with mutexes */
#define _NTO_SYNC_OWNER_MASK	0x7fffffff	/* Owner used with mutexes */
#define _NTO_SYNC_INITIALIZER	0xffffffff	/*   -1  count is 0=mutexes, 0(old),-5(new)=cond */
#define _NTO_SYNC_DESTROYED		0xfffffffe	/*   -2  mutexes, cond, sem, spin */
#define _NTO_SYNC_NAMED_SEM		0xfffffffd	/*   -3  sem (count is handle) */
#define _NTO_SYNC_SEM			0xfffffffc	/*   -4  sem (count is value) */
#define _NTO_SYNC_COND			0xfffffffb	/*   -5  cond (count is clockid) */
#define _NTO_SYNC_SPIN			0xfffffffa	/*   -6  spin (count is internal) */
#define _NTO_SYNC_DEAD			0xffffff00	/* -256  mutex (when a process dies with a mutex locked) */

#define PTHREAD_DETACHSTATE_MASK		0x01
#define PTHREAD_CREATE_JOINABLE				0x00
#define PTHREAD_CREATE_DETACHED				0x01

#define PTHREAD_INHERITSCHED_MASK		0x02
#define PTHREAD_INHERIT_SCHED				0x00
#define PTHREAD_EXPLICIT_SCHED				0x02

#define PTHREAD_CONTENTIONSCOPE_MASK	0x04
#define PTHREAD_SCOPE_SYSTEM				0x00
#define PTHREAD_SCOPE_PROCESS				0x04

#define PTHREAD_MULTISIG_MASK			0x08
#define PTHREAD_MULTISIG_ALLOW				0x00
#define PTHREAD_MULTISIG_DISALLOW			0x08

#define PTHREAD_CSTATE_MASK				0x10
#define PTHREAD_CANCEL_ENABLE				0x00
#define PTHREAD_CANCEL_DISABLE				0x10

#define PTHREAD_CTYPE_MASK				0x20
#define PTHREAD_CANCEL_DEFERRED				0x00
#define PTHREAD_CANCEL_ASYNCHRONOUS			0x20

#define PTHREAD_CANCEL_PENDING			0x40

#define PTHREAD_NOTLAZYSTACK_MASK		0x80
#define PTHREAD_STACK_LAZY					0x00
#define PTHREAD_STACK_NOTLAZY				0x80

#define PTHREAD_CANCELED ((void *)-1)

#if defined(__PTHREAD_KEY_T)
typedef __PTHREAD_KEY_T pthread_key_t;
#undef __PTHREAD_KEY_T
#endif

#define PTHREAD_PRIO_INHERIT	0
#define PTHREAD_PRIO_NONE		1
#define PTHREAD_PRIO_PROTECT	2

/* thread attribute prototypes */
extern int pthread_attr_destroy(pthread_attr_t *__attr);
extern int pthread_attr_getdetachstate(const pthread_attr_t *__attr, int *__detachstate);
extern int pthread_attr_getstack(const pthread_attr_t *__attr, void **__stackaddr, size_t *__stacksize);
extern int pthread_attr_getstackaddr(const pthread_attr_t *__addr, void **__stackaddr);
extern int pthread_attr_getstacksize(const pthread_attr_t *__attr, size_t *__stacksize);
extern int pthread_attr_init(pthread_attr_t *__attr);
extern int pthread_attr_setdetachstate(pthread_attr_t *__attr, int __detachstate);
extern int pthread_attr_setstack(pthread_attr_t *__attr, void *__stackaddr, size_t __stacksize);
extern int pthread_attr_setstackaddr(pthread_attr_t *__attr, void *__stackaddr);
extern int pthread_attr_setstacksize(pthread_attr_t *__attr, size_t __stacksize);

#if defined(__EXT_XOPEN_EX)
extern int pthread_attr_getguardsize(const pthread_attr_t *__attr, size_t *__guardsize);
extern int pthread_attr_setguardsize(pthread_attr_t *__attr, size_t __guardsize);
extern int pthread_getconcurrency(void);
extern int pthread_setconcurrency(int __new_level);
#endif

#if defined(__EXT_QNX)
extern int pthread_attr_getstacklazy(const pthread_attr_t *__attr, int *__lazystack);
extern int pthread_attr_setstacklazy(pthread_attr_t *__attr, int __lazystack);
extern int pthread_attr_getstackprealloc(const pthread_attr_t *__attr, size_t *__prealloc);
extern int pthread_attr_setstackprealloc(pthread_attr_t *__attr, size_t __prealloc);
#if defined(__SLIB_DATA_INDIRECT) && !defined(pthread_attr_default) && !defined(__SLIB)
  const pthread_attr_t * __get_pthread_attr_default_ptr(void);
  #define pthread_attr_default (const pthread_attr_t)(*(__get_pthread_attr_default_ptr()))
#else
  extern const pthread_attr_t pthread_attr_default;
#endif
#endif

/* scheduling related functions */
extern int pthread_attr_getinheritsched(const pthread_attr_t *__attr, int *__inheritsched);
extern int pthread_attr_getschedparam(const pthread_attr_t *__attr, struct sched_param *__param);
extern int pthread_attr_getschedpolicy(const pthread_attr_t *__attr, int *__policy);
extern int pthread_attr_getscope(const pthread_attr_t *__attr, int *__contentionscope);
extern int pthread_attr_setinheritsched(pthread_attr_t *__attr, int __inheritsched);
extern int pthread_attr_setschedparam(pthread_attr_t *__attr, const struct sched_param *__param);
extern int pthread_attr_setschedpolicy(pthread_attr_t *__attr, int __policy);
extern int pthread_attr_setscope(pthread_attr_t *__attr, int __contentionscope);

/* thread creation prototypes */
extern int pthread_cancel(pthread_t __thr);
extern int pthread_create(pthread_t *__thr, const pthread_attr_t *__attr, void *(*__start_routine)(void *), void *__arg);
extern int pthread_detach(pthread_t __thr);
extern int pthread_equal(pthread_t __t1, pthread_t __t2);
extern void pthread_exit(void *__value_ptr) __attribute__((__noreturn__));
extern int pthread_join(pthread_t __thr, void **__value_ptr);
extern pthread_t pthread_self(void);
extern int pthread_setcancelstate(int __state, int *__oldstate);
extern int pthread_setcanceltype(int __type, int *__oldtype);
#ifdef __INLINE_FUNCTIONS__
#define pthread_self()				(__tls()->__tid)
#define pthread_equal(__t1, __t2)	((__t1) == (__t2))
#endif

/* dynamic thread scheduling parameters */
extern int pthread_getschedparam(const pthread_t __thr, int *__policy, struct sched_param *__param);
extern int pthread_setschedparam(pthread_t __thr, int __policy, const struct sched_param *__param);
#if defined(__EXT_POSIX1_200112)
extern int pthread_setschedprio(pthread_t __thr, int __prio);
#endif

extern void pthread_testcancel(void);
#ifdef __INLINE_FUNCTIONS__
#define pthread_testcancel() if((__tls()->__flags & (PTHREAD_CSTATE_MASK|PTHREAD_CANCEL_PENDING)) == \
				(PTHREAD_CANCEL_ENABLE|PTHREAD_CANCEL_PENDING)) pthread_exit(PTHREAD_CANCELED);
#endif

#if defined(__EXT_QNX)
extern int pthread_getname_np(pthread_t __thr, char *__buff, int __buff_size);
extern int pthread_setname_np(pthread_t __thr, const char *__name);
extern int __getset_thread_name(pid_t __pid, pthread_t __thr, const char *__newname, int __newname_len, char *__prevname, int __prevname_len);
#endif


/* thread cancelation handlers */

struct __cleanup_handler;
struct __cleanup_handler {
    struct __cleanup_handler *__next;
    void (*__routine)(void *__arg);
    void *__save;
};

#if defined(__EXT_QNX)
extern void __cleanup_push(struct __cleanup_handler *__cleanup, void (* __func)(void *__arg), void *__arg);
extern struct __cleanup_handler *__cleanup_pop(int __ex);
#endif

#define pthread_cleanup_push(__func, __arg) \
	{ \
	struct __cleanup_handler __cleanup_handler; \
	__cleanup_handler.__routine = (__func); \
	__cleanup_handler.__save = (__arg); \
	__cleanup_handler.__next = (struct __cleanup_handler *)__tls()->__cleanup; \
	__tls()->__cleanup = (void *)&__cleanup_handler;

#define pthread_cleanup_pop(__ex) \
	__tls()->__cleanup = (void *)__cleanup_handler.__next; \
	((__ex) ? __cleanup_handler.__routine(__cleanup_handler.__save) : (void)0);\
	}

/* pthread_key prototypes */
extern void *pthread_getspecific(pthread_key_t __key);
extern int pthread_key_create(pthread_key_t *__key, void (*__destructor)(void *));
extern int pthread_key_delete(pthread_key_t __key);
extern int pthread_setspecific(pthread_key_t __key, const void *__value);
#ifdef __INLINE_FUNCTIONS__
#define pthread_getspecific(key)	((key) < __tls()->__numkeys ? __tls()->__keydata[key] : 0)
#endif


/* pthread synchronization prototypes */
#define PTHREAD_MUTEX_INITIALIZER	{ _NTO_SYNC_NONRECURSIVE, _NTO_SYNC_INITIALIZER }
#if defined(__EXT_XOPEN_EX)
#define PTHREAD_MUTEX_DEFAULT		0	/* ((int)(_NTO_SYNC_NONRECURSIVE)) */
#define PTHREAD_MUTEX_ERRORCHECK	1	/* ((int)(_NTO_SYNC_NONRECURSIVE)) */
#define PTHREAD_MUTEX_RECURSIVE		2	/* (0) */
#define PTHREAD_MUTEX_NORMAL		3	/* ((int)(_NTO_SYNC_NONRECURSIVE|_NTO_SYNC_NOERRORCHECK)) */
#endif
#if defined(__EXT_QNX)
#define PTHREAD_RMUTEX_INITIALIZER	{ 0, _NTO_SYNC_INITIALIZER }
#endif

#define PTHREAD_COND_INITIALIZER	{ (int)_NTO_SYNC_COND, _NTO_SYNC_INITIALIZER }

#define PTHREAD_PROCESSSHARED_MASK		0x01
#define PTHREAD_PROCESS_PRIVATE				0x00
#define PTHREAD_PROCESS_SHARED				0x01

#define PTHREAD_RECURSIVE_MASK			0x02
#define PTHREAD_RECURSIVE_DISABLE			0x00
#define PTHREAD_RECURSIVE_ENABLE			0x02

#define PTHREAD_ERRORCHECK_MASK			0x04
#define PTHREAD_ERRORCHECK_ENABLE			0x00
#define PTHREAD_ERRORCHECK_DISABLE			0x04

#define _NTO_ATTR_FLAGS					0x0000ffff	/* These flags are verified for each type */

#define _NTO_ATTR_MASK					0x000f0000
#define _NTO_ATTR_UNKNOWN				0x00000000
#define _NTO_ATTR_MUTEX					0x00010000
#define _NTO_ATTR_COND					0x00020000
#define _NTO_ATTR_RWLOCK				0x00030000
#define _NTO_ATTR_BARRIER				0x00040000

#define PTHREAD_MUTEX_TYPE				0x40000000	/* non-default mutex type */
/*
 * Used to identify that more than the first three ints
 * of struct _sync_attr are valid. This is for compatibility with old apps
 */
#define _NTO_ATTR_EXTRA_FLAG			0x80000000

/*
 * These are inline mutex macros that do not perform error checking
 * and does not handle recursive. i.e. type PTHREAD_MUTEX_NORMAL
 * They can be used with mutexes initialized with PTHREAD_MUTEX_DEFAULT,
 * PTHREAD_MUTEX_ERRORCHECK, or PTHREAD_MUTEX_NORMAL. 
 */
#define __mutex_lock(__m)		(__mutex_smp_cmpxchg(&(__m)->__owner, 0, __tls()->__owner) ? SyncMutexLock_r(__m) : 0 /* EOK */)
#define __mutex_trylock(__m)	(__mutex_smp_cmpxchg(&(__m)->__owner, 0, __tls()->__owner) ? 16 /* EBUSY */ : 0 /* EOK */)
#define __mutex_unlock(__m)		((__mutex_smp_xchg(&(__m)->__owner, 0) & _NTO_SYNC_WAITING) ? __cpu_membarrier(), SyncMutexUnlock_r((__m)) : 0 /* EOK */)
#if defined(__EXT_QNX)
#define _mutex_lock(__m)	__mutex_lock(__m)
#define _mutex_trylock(__m)	__mutex_trylock(__m)
#define _mutex_unlock(__m)	__mutex_unlock(__m)
#endif


/* synchronization stuff */
extern int pthread_mutexattr_destroy(pthread_mutexattr_t *__attr);
extern int pthread_mutexattr_init(pthread_mutexattr_t *__attr);

/* We always allow process shared */
extern int pthread_mutexattr_getpshared(const pthread_mutexattr_t *__attr, int *__pshared);
extern int pthread_mutexattr_setpshared(pthread_mutexattr_t *__attr,int __pshared);

/* synchronization scheduling */
extern int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *__attr, int *__prioceiling);
extern int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *__attr, int *__protocol);
extern int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *__attr, int __prioceiling);
extern int pthread_mutexattr_setprotocol(pthread_mutexattr_t *__attr, int __protocol);

/* mutex recursion */
extern int pthread_mutex_destroy(pthread_mutex_t *__mutex);
extern int pthread_mutex_init(pthread_mutex_t *__mutex, const pthread_mutexattr_t *__attr);
extern int pthread_mutex_lock(pthread_mutex_t *__mutex);
#if defined(__EXT_POSIX1_200112)		/* Approved 1003.1d D14 */
extern int pthread_mutex_timedlock(pthread_mutex_t *__mutex, const struct timespec *__abs_timeout);
#endif
extern int pthread_mutex_trylock(pthread_mutex_t *__mutex);
extern int pthread_mutex_unlock(pthread_mutex_t *__mutex);
#if defined(__INLINE_FUNCTIONS__) && 0	/* Not needed yet... */
#define pthread_mutex_lock(__m)		(__mutex_smp_cmpxchg(&(__m)->__owner, 0, __tls()->__owner) ? (pthread_mutex_lock)(__m) : ((__m)->__count++, 0/* EOK */))
#define pthread_mutex_trylock(__m)	(__mutex_smp_cmpxchg(&(__m)->__owner, 0, __tls()->__owner) ? 16 /* EBUSY */ : ((__m)->__count++, 0 /* EOK */))
#define pthread_mutex_unlock(__m)	(((__m)->__owner & ~_NTO_SYNC_WAITING) == __tls()->__owner ? (--(__m)->__count <= 0 && \
									(__cpu_membarrier(), (__mutex_smp_xchg(&(__m)->__owner, 0) & _NTO_SYNC_WAITING)) ? SyncMutexUnlock_r(__m) : 0 /* EOK */) : 1 /* EPERM */)
#if defined(__EXT_QNX)		/* Approved 1003.1d D14 */
#define pthread_mutex_timedlock(__m, __t)	(__mutex_smp_cmpxchg(&(__m)->__owner, 0, __tls()->__owner) ? (pthread_mutex_timedlock)((__m), (__t)) : 0 /* EOK */)
#endif
#endif
#if defined(__EXT_QNX)
extern int pthread_mutexattr_getrecursive(const pthread_mutexattr_t *__attr, int *__recursive);
extern int pthread_mutexattr_setrecursive(pthread_mutexattr_t *__attr, int __recursive);
#endif
#if defined(__EXT_XOPEN_EX)
extern int pthread_mutexattr_gettype(const pthread_mutexattr_t *__attr, int *__type);
extern int pthread_mutexattr_settype(pthread_mutexattr_t *__attr, int __type);
#endif

/* dynamically change the priority ceiling of a mutex */
extern int pthread_condattr_destroy(pthread_condattr_t *__attr);
extern int pthread_condattr_init(pthread_condattr_t *__attr);
extern int pthread_mutex_getprioceiling(const pthread_mutex_t *__mutex, int *__prioceiling);
extern int pthread_mutex_setprioceiling(pthread_mutex_t *__mutex, int __prioceiling, int *__old_ceiling);

/* Next two functions are not supported currently */
extern int pthread_cond_broadcast(pthread_cond_t *__cond);
extern int pthread_cond_destroy(pthread_cond_t *__cond);
extern int pthread_cond_init(pthread_cond_t *__cond, const pthread_condattr_t *__attr);
extern int pthread_cond_signal(pthread_cond_t *__cond);
extern int pthread_cond_wait(pthread_cond_t *__cond, pthread_mutex_t *__mutex);
extern int pthread_condattr_getpshared(const pthread_condattr_t *__attr, int *__pshared);
extern int pthread_condattr_setpshared(pthread_condattr_t *__attr, int __pshared);
#if defined(__EXT_POSIX1_200112)		/* Approved 1003.1d D14 */
extern int pthread_cond_timedwait(pthread_cond_t *__cond, pthread_mutex_t *__mutex, const struct timespec *__abstime);
#endif

/* pthread_once prototypes */
#ifdef __PTHREAD_ONCE_T
typedef __PTHREAD_ONCE_T	pthread_once_t;
#undef __PTHREAD_ONCE_T
#endif

#define PTHREAD_ONCE_INIT		{ 0, PTHREAD_MUTEX_INITIALIZER }

extern int pthread_once(pthread_once_t *__once_control, void (*__init_routine)(void));
#ifdef __INLINE_FUNCTIONS__
extern int __pthread_once(pthread_once_t *__once_control, void (*__init_routine)(void));
#define pthread_once(_c, _f)	((_c)->__once ? 0 : __pthread_once((_c),(_f)) )
#endif

#if defined(__EXT_POSIX1_200112)		/* Approved 1003.1d D14 */
extern int pthread_getcpuclockid(pthread_t __id, clockid_t *__clock_id);
#endif

#if defined(__EXT_POSIX1_200112)		/* Approved 1003.1j D10 */
extern int pthread_condattr_getclock(const pthread_condattr_t *__attr, clockid_t *__id);
extern int pthread_condattr_setclock(pthread_condattr_t *__attr, int __id);

/* POSIX barriers */
#define PTHREAD_BARRIER_SERIAL_THREAD	((int)-1)

typedef struct _sync_attr pthread_barrierattr_t;
typedef struct {
	unsigned int		__barrier;
	unsigned int		__count;
	pthread_mutex_t		__lock;
	pthread_cond_t		__bcond;
} pthread_barrier_t;

/* barrier attribute prototypes */
extern int pthread_barrierattr_init(pthread_barrierattr_t *__attr);
extern int pthread_barrierattr_destroy(pthread_barrierattr_t *__attr);
extern int pthread_barrierattr_getpshared(const pthread_barrierattr_t *__attr, int *__pshared);
extern int pthread_barrierattr_setpshared(pthread_barrierattr_t *__attr, int __pshared);

/* barrier prototypes */
extern int pthread_barrier_destroy(pthread_barrier_t *__b);
extern int pthread_barrier_init(pthread_barrier_t *__b, const pthread_barrierattr_t *__attr, unsigned int __count);
extern int pthread_barrier_wait(pthread_barrier_t *__b);

typedef struct _sync_attr pthread_rwlockattr_t;

extern int pthread_rwlockattr_init(pthread_rwlockattr_t *__attr);
extern int pthread_rwlockattr_destroy(pthread_rwlockattr_t *__attr);

extern int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *__attr, int *__pshared);
extern int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *__attr, int __pshared);

typedef struct {
	int				__active;			/* -1 = writer else # of active readers */
	void			*__spare;
	int				__blockedwriters;	/* # of waiting readers */
	int				__blockedreaders;	/* # of waiting writers */
	int				__heavy;			/* the rwlock is under heavy contention */
	pthread_mutex_t	__lock;			/* the controlling mutex */
	pthread_cond_t	__rcond;			/* condition variable for readers */
	pthread_cond_t	__wcond;			/* condition variable for writers */
	unsigned		__owner;			/* used to prevent and detect deadlocks */
} pthread_rwlock_t;

extern int pthread_rwlock_destroy(pthread_rwlock_t *);
extern int pthread_rwlock_init(pthread_rwlock_t *, const pthread_rwlockattr_t *);
extern int pthread_rwlock_rdlock(pthread_rwlock_t *);
extern int pthread_rwlock_tryrdlock(pthread_rwlock_t *);
extern int pthread_rwlock_timedrdlock(pthread_rwlock_t *, const struct timespec *__abs_timeout);
extern int pthread_rwlock_wrlock(pthread_rwlock_t *);
extern int pthread_rwlock_trywrlock(pthread_rwlock_t *);
extern int pthread_rwlock_timedwrlock(pthread_rwlock_t *, const struct timespec *__abs_timeout);
extern int pthread_rwlock_unlock(pthread_rwlock_t *);

/* Posix spin locks */
typedef struct _sync pthread_spinlock_t;
extern int (*_spin_lock_v)(struct _sync *__sync);
extern int (*_spin_trylock_v)(struct _sync *__sync);
extern int (*_spin_unlock_v)(struct _sync *__sync);

/* pthread spin lock prototypes */
extern int pthread_spin_init(pthread_spinlock_t *__lock, int __pshared);
extern int pthread_spin_destroy(pthread_spinlock_t *__lock);
extern int pthread_spin_lock(pthread_spinlock_t *__lock);
#define pthread_spin_lock(__lock)    ((_spin_lock_v)((struct _sync *)(__lock)))
extern int pthread_spin_trylock(pthread_spinlock_t *__lock);
#define pthread_spin_trylock(__lock) ((_spin_trylock_v)((struct _sync *)(__lock)))
extern int pthread_spin_unlock(pthread_spinlock_t *__lock);
#define pthread_spin_unlock(__lock)  ((_spin_unlock_v)((struct _sync *)(__lock)))
#endif


#if defined(__EXT_QNX)		/* QNX Extensions (1003.1j D5) */
extern int pthread_timedjoin(pthread_t __thr, void **__value_ptr, const struct timespec *__abstime);
/* Unconditional thread termination */
#define PTHREAD_ABORTED ((void *)(-2))
extern int pthread_abort(pthread_t __thr);
#endif

#if defined(__EXT_QNX)
/* QNX Extensions */
#define PTHREAD_BARRIER_INITIALIZER(__b)	{ \
	(__b), \
	(__b), \
	PTHREAD_MUTEX_INITIALIZER, \
	PTHREAD_COND_INITIALIZER \
	}

/* In UNIX98, but removed for alignment with approved 1003.1j */
#define PTHREAD_RWLOCK_INITIALIZER	{ 0, 0, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, \
	PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, -2U }

/* sleepon */
typedef struct _sleepon_handle sleepon_t;
extern sleepon_t	_sleepon_default;
extern int _sleepon_init(sleepon_t **__list, unsigned __flags);
extern int _sleepon_destroy(sleepon_t *__handle);
extern int _sleepon_lock(sleepon_t *__handle);
extern int _sleepon_unlock(sleepon_t *__handle);
extern int _sleepon_wait(sleepon_t *__handle, const volatile void *__addr, _Uint64t __nsec);
extern int _sleepon_signal(sleepon_t *__handle, const volatile void *__addr);
extern int _sleepon_broadcast(sleepon_t *__handle, const volatile void *__addr);

extern int pthread_sleepon_lock(void);
extern int pthread_sleepon_unlock(void);
extern int pthread_sleepon_wait(const volatile void *__addr);
extern int pthread_sleepon_timedwait(const volatile void *__addr, _Uint64t __nsec);
extern int pthread_sleepon_signal(const volatile void *__addr);
extern int pthread_sleepon_broadcast(const volatile void *__addr);
#ifdef __INLINE_FUNCTIONS__
#define pthread_sleepon_lock()					_sleepon_lock(&_sleepon_default)
#define pthread_sleepon_unlock()				_sleepon_unlock(&_sleepon_default)
#define pthread_sleepon_wait(__addr)			_sleepon_wait(&_sleepon_default, (__addr), 0)
#define pthread_sleepon_timedwait(__addr, __nsec)_sleepon_wait(&_sleepon_default, (__addr), (__nsec))
#define pthread_sleepon_signal(__addr)			_sleepon_signal(&_sleepon_default, (__addr))
#define pthread_sleepon_brodcast(__addr)		_sleepon_brodcast(&_sleepon_default, (__addr))
#endif

#endif

#include <_packpop.h>

__END_DECLS

#endif

/* __SRCVERSION("pthread.h $Rev: 207498 $"); */
