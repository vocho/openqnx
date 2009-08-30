/* This is an opaque data type; applications should not access it directly */
struct _pthread_fastlock
{
  long int __status;   /* "Free" or "taken" or head of waiting list */
  int __spinlock;      /* Used by compare_and_swap emulation. Also,
			  adaptive SMP lock stores spin count here. */
};

/* <pthread.h>  Use pthread_attr_init() to initialize this. */
typedef struct __pthread_attr_s
{
  int __detachstate;
  int __schedpolicy;
  struct __sched_param __schedparam;
  int __inheritsched;
  int __scope;
  size_t __guardsize;
  int __stackaddr_set;
  void *stackaddr;
  size_t __stacksize;
} pthread_attr_t;


/* <pthread.H> Conditions (not abstract because of PTHREAD_COND_INITIALIZER */
typedef struct
{
  struct _pthread_fastlock __c_lock; /* Protect against concurrent access */
  _pthread_descr __c_waiting;        /* Threads waiting on this condition */
} pthread_cond_t;


/* <pthread.h> Attribute for conditional variables.  */
typedef struct
{
  int __dummy;
} pthread_condattr_t;

/* <pthread.h> Keys for thread-specific data */
typedef unsigned int pthread_key_t;


/* <pthread.h> Mutexes (not abstract because of PTHREAD_MUTEX_INITIALIZER).  */
typedef struct
{
  int __m_reserved;               /* Reserved for future use */
  int __m_count;                  /* Depth of recursive locking */
  _pthread_descr __m_owner;       /* Owner thread (if recursive or errcheck) */
  int __m_kind;                   /* Mutex kind: fast, recursive or errcheck */
  struct _pthread_fastlock __m_lock; /* Underlying fast lock */
} pthread_mutex_t;


/* <pthread.h> Attribute for mutex.  */
typedef struct
{
  int __mutexkind;
} pthread_mutexattr_t;


/* <pthread.h> Once-only execution */
typedef int pthread_once_t;


/* <pthread.h> Read-write locks.  */
typedef struct _pthread_rwlock_t
{
  struct _pthread_fastlock __rw_lock; /* Lock to guarantee mutual exclusion */
  int __rw_readers;                   /* Number of readers */
  _pthread_descr __rw_writer;         /* Identity of writer, or NULL if none */
  _pthread_descr __rw_read_waiting;   /* Threads waiting for reading */
  _pthread_descr __rw_write_waiting;  /* Threads waiting for writing */
  int __rw_kind;                      /* Reader/Writer preference selection */
  int __rw_pshared;                   /* Shared between processes or not */
} pthread_rwlock_t;


/* <pthread.h> Attribute for read-write locks.  */
typedef struct
{
  int __lockkind;
  int pshared;
} pthread_rwlockattr_t;

/* <pthread.h> POSIX spinlock data type.  */
typedef volatile int pthread_spinlock_t;

/* <pthread.h> POSIX barrier. */
typedef struct {
  struct _pthread_fastlock __ba_lock; /* Lock to guarantee mutual exclusion */
  int __ba_required;                  /* Threads needed for completion */
  int __ba_present;                   /* Threads waiting */
  _pthread_descr __ba_waiting;        /* Queue of waiting threads */
} pthread_barrier_t;

/* <pthread.h> barrier attribute */
typedef struct {
  int pshared;
} pthread_barrierattr_t;

/* <pthread.h> Thread identifiers */
typedef unsigned long int pthread_t;

/* <pthread.h> */
#define PTHREAD_MUTEX_INITIALIZER ...

/* <pthread.h> only if __USE_GNU is defined */
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP ...

/* <pthread.h> only if __USE_GNU is defined */
#define PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP ...

/* <pthread.h> only if __USE_GNU is defined */
#define PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP ...

/* <pthread.h> */
#define PTHREAD_COND_INITIALIZER ...

/* <pthread.h> */
#define PTHREAD_RWLOCK_INITIALIZER ...

/* <pthread.h> only if __USE_GNU is defined */
#define PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP ...

/* Values for attributes.  */

/* <pthread.h> These values are also defined as macros so you can use #ifdef */
enum { PTHREAD_CREATE_JOINABLE, PTHREAD_CREATE_DETACHED };

/* <pthread.h> These values are also defined as macros so you can use #ifdef */
enum { PTHREAD_INHERIT_SCHED, PTHREAD_EXPLICIT_SCHED };

/* <pthread.h> These values are also defined as macros so you can use #ifdef */
enum { PTHREAD_SCOPE_SYSTEM, PTHREAD_SCOPE_PROCESS };

/* <pthread.h> */
enum
{
  PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP
#ifdef __USE_UNIX98
  ,
  PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL
#endif
#ifdef __USE_GNU
  /* For compatibility.  */
  , PTHREAD_MUTEX_FAST_NP = PTHREAD_MUTEX_ADAPTIVE_NP
#endif
};

/* <pthread.h> These values are also defined as macros so you can use #ifdef */
enum { PTHREAD_PROCESS_PRIVATE, PTHREAD_PROCESS_SHARED };

/* <pthread.h> */
enum
{
  PTHREAD_RWLOCK_PREFER_READER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP,
  PTHREAD_RWLOCK_DEFAULT_NP = PTHREAD_RWLOCK_PREFER_WRITER_NP
};

/* <pthread.h> */
#define PTHREAD_ONCE_INIT 0

/* <pthread.h> */
# define PTHREAD_BARRIER_SERIAL_THREAD -1

/* <pthread.h> Cleanup buffers */
struct _pthread_cleanup_buffer
{
  void (*routine)(void *);		  /* Function to call.  */
  void *arg;				  /* Its argument.  */
  int __canceltype;			  /* Saved cancellation type. */
  struct _pthread_cleanup_buffer *prev; /* Chaining of cleanup functions.  */
};

/* <pthread.h> These values are also defined as macros so you can use #ifdef */
enum
{
  PTHREAD_CANCEL_ENABLE,
  PTHREAD_CANCEL_DISABLE
};

/* <pthread.h> These values are also defined as macros so you can use #ifdef */
enum
{
  PTHREAD_CANCEL_DEFERRED,
  PTHREAD_CANCEL_ASYNCHRONOUS
};

/* <pthread.h> */
#define PTHREAD_CANCELED ((void *) -1)


/* <pthread.h> Create a thread with given attributes ATTR (or default */
/* attributes if ATTR is NULL), and call function START_ROUTINE with  */
/* given arguments ARG.                                               */
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
}

/* <pthread.h> Obtain the identifier of the current thread.  */
pthread_t pthread_self(void)
{
}

/* <pthread.h> Compare two thread identifiers.  */
int pthread_equal(pthread_t thread1, pthread_t thread2)
{
}

/* <pthread.h> Terminate calling thread.  */
void pthread_exit(void *retval)
{
}

/* <pthread.h> Make calling thread wait for termination of the thread TH. */
/* The exit status of the thread is stored in *THREAD_RETURN, if          */
/* THREAD_RETURN is not NULL.                                             */
int pthread_join(pthread_t th, void **thread_return)
{
}

/* <pthread.h> Indicate that the thread TH is never to be joined with     */
/* PTHREAD_JOIN.  The resources of TH will therefore be freed immediately */
/* when it terminates, instead of waiting for another thread to perform   */
/* PTHREAD_JOIN on it.                                                    */
int pthread_detach(pthread_t th)
{
}

/* <pthread.h> Initialize thread attribute *ATTR with default attributes  */
/* (detachstate is PTHREAD_JOINABLE, scheduling policy is SCHED_OTHER, no */
/* user-provided stack).                                                  */
int pthread_attr_init(pthread_attr_t *attr)
{
}

/* <pthread.h> Destroy thread attribute *ATTR.  */
int pthread_attr_destroy(pthread_attr_t *attr)
{
}

/* <pthread.h> Set the `detachstate' attribute in *ATTR according to */
/* DETACHSTATE.                                                      */
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
}

/* <pthread.h> Return in *DETACHSTATE the `detachstate' attribute in *ATTR.  */
int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
}

/* <pthread.h> Set scheduling parameters (priority, etc) in *ATTR */
/* according to PARAM.                                            */
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *param)
{
}

/* <pthread.h> Return in *PARAM the scheduling parameters of *ATTR.  */
int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *param)
{
}

/* <pthread.h> Set scheduling policy in *ATTR according to POLICY.  */
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
}

/* <pthread.h> Return in *POLICY the scheduling policy of *ATTR.  */
int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
}

/* <pthread.h> Set scheduling inheritance mode in *ATTR according to INHERIT. */
int pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit)
{
}

/* <pthread.h> Return in *INHERIT the scheduling inheritance mode of *ATTR.  */
int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inherit)
{
}

/* <pthread.h> Set scheduling contention scope in *ATTR according to SCOPE.  */
int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
}

/* <pthread.h> Return in *SCOPE the scheduling contention scope of *ATTR.  */
int pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
}

/* <pthread.h> Set the size of the guard area at the bottom of the thread.  */
int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize)
{
}

/* <pthread.h> Get the size of the guard area at the bottom of the thread.  */
int pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *guardsize)
{
}

/* <pthread.h> Set the starting address of the stack of the thread to be    */
/* created.  Depending on whether the stack grows up or down the value must */
/* either be higher or lower than all the address in the memory block.      */
/* The minimal size of the block must be PTHREAD_STACK_SIZE.                */
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
}

/* <pthread.h> Return the previously set address for the stack.  */
int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
}

/* <pthread.h> Set the starting address and size of the stack of a thread  */
/* to be created.  This is intended to replace pthread_attr_setstackaddr() */
int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize)
{
}

/* <pthread.h> Return the previously set address for the stack.  This is */
/* intended to replace pthread_attr_getstackaddr()                       */
int pthread_attr_getstack(const pthread_attr_t *attr, void **stackaddr, size_t *stacksize)
{
}

/* <pthread.h> Add information about the minimum stack size needed  */
/* for the thread to be started.  This size must never be less than */
/* PTHREAD_STACK_SIZE and must also not exceed the system limits.   */
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
}

/* <pthread.h> Return the currently used minimal stack size.  */
int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
}

/* <pthread.h> Get thread attributes corresponding to the already running */
/* thread TH.  Only if __USE_GNU is defined.                              */
int pthread_getattr_np(pthread_t th, pthread_attr_t *attr)
{
}

/* <pthread.h> Set the scheduling parameters for TARGET_THREAD */
/* according to POLICY and *PARAM.                             */
int pthread_setschedparam(pthread_t target_thread, int policy, const struct sched_param *param)
{
}

/* <pthread.h> Return in *POLICY and *PARAM the scheduling parameters */
/* for TARGET_THREAD.                                                 */
int pthread_getschedparam(pthread_t target_thread, int *policy, struct sched_param *param)
{
}

/* <pthread.h> Determine level of concurrency.  */
int pthread_getconcurrency(void)
{
}

/* <pthread.h> Set new concurrency level to LEVEL.  */
int pthread_setconcurrency(int level)
{
}

/* <pthread.h> Yield the processor to another thread or process.    */
/* This function is similar to the POSIX `sched_yield' function but */
/* might be differently implemented in the case of a m-on-n thread  */
/* implementation.  Only if __USE_GNU is defined.                   */
int pthread_yield(void)
{
}

/* <pthread.h> Initialize MUTEX using attributes in *MUTEX_ATTR, */
/* or use the default values if later is NULL.                   */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutex_attr)
{
}

/* <pthread.h> Destroy MUTEX.  */
int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
}

/* <pthread.h> Try to lock MUTEX.  */
int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
}

/* <pthread.h> Wait until lock for MUTEX becomes available and lock it.  */
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
}

/* <pthread.h> Wait until lock becomes available, or specified time passes. */
int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime)
{
}

/* <pthread.h> Unlock MUTEX.  */
int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
}

/* <pthread.h> Initialize mutex attribute object ATTR with default */
/* attributes (kind is PTHREAD_MUTEX_TIMED_NP).                    */
int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
}

/* <pthread.h> Destroy mutex attribute object ATTR.  */
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
}

/* <pthread.h> Get the process-shared flag of the mutex attribute ATTR.  */
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared)
{
}

/* <pthread.h> Set the process-shared flag of the mutex attribute ATTR.  */
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
}

/* <pthread.h> Set the mutex kind attribute in *ATTR to KIND (either        */
/* PTHREAD_MUTEX_NORMAL, PTHREAD_MUTEX_RECURSIVE, PTHREAD_MUTEX_ERRORCHECK, */
/* or PTHREAD_MUTEX_DEFAULT).                                               */
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int kind)
{
}

/* <pthread.h> Return in *KIND the mutex kind attribute in *ATTR.  */
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *kind)
{
}

/* <pthread.h> Initialize condition variable COND using attributes ATTR, */
/* or use the default values if later is NULL.                           */
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *cond_attr)
{
}

/* <pthread.h> Destroy condition variable COND.  */
int pthread_cond_destroy(pthread_cond_t *cond)
{
}

/* <pthread.h> Wake up one thread waiting for condition variable COND.  */
int pthread_cond_signal(pthread_cond_t *cond)
{
}

/* <pthread.h> Wake up all threads waiting for condition variables COND.  */
int pthread_cond_broadcast(pthread_cond_t *cond)
{
}

/* <pthread.h> Wait for condition variable COND to be signaled or broadcast. */
/* MUTEX is assumed to be locked before.                                     */
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
}

/* <pthread.h> Wait for condition variable COND to be signaled or broadcast */
/* until ABSTIME.  MUTEX is assumed to be locked before.  ABSTIME is an     */
/* absolute time specification; zero is the beginning of the epoch          */
/* (00:00:00 GMT, January 1, 1970).                                         */
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
}

/* Functions for handling condition variable attributes.  */

/* <pthread.h> Initialize condition variable attribute ATTR.  */
int pthread_condattr_init(pthread_condattr_t *attr)
{
}

/* <pthread.h> Destroy condition variable attribute ATTR.  */
int pthread_condattr_destroy(pthread_condattr_t *attr)
{
}

/* <pthread.h> Get the process-shared flag of the condition variable */
/* attribute ATTR.                                                   */
int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared)
{
}

/* <pthread.h> Set the process-shared flag of the condition variable */
/* attribute ATTR.                                                   */
int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared)
{
}


/* <pthread.h> Initialize read-write lock RWLOCK using attributes ATTR, */
/* or use the default values if later is NULL.                          */
int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
{
}

/* <pthread.h> Destroy read-write lock RWLOCK.  */
int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
}

/* <pthread.h> Acquire read lock for RWLOCK.  */
int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
}

/* <pthread.h> Try to acquire read lock for RWLOCK.  */
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
}

/* <pthread.h> Try to acquire read lock for RWLOCK or return after */
/* specfied time.                                                  */
int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock, const struct timespec *abstime)
{
}

/* <pthread.h> Acquire write lock for RWLOCK.  */
int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
}

/* <pthread.h> Try to acquire write lock for RWLOCK.  */
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
}

/* <pthread.h> Try to acquire write lock for RWLOCK or return after */
/* specified time.                                                  */
int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock, const struct timespec *abstime)
{
}

/* <pthread.h> Unlock RWLOCK.  */
int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
}


/* <pthread.h> Initialize attribute object ATTR with default values.  */
int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
}

/* <pthread.h> Destroy attribute object ATTR.  */
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
}

/* <pthread.h> Return current setting of process-shared attribute */
/* of ATTR in PSHARED.                                            */
int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared)
{
}

/* <pthread.h> Set process-shared attribute of ATTR to PSHARED.  */
int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared)
{
}

/* <pthread.h> Return current setting of reader/writer preference.  */
int pthread_rwlockattr_getkind_np(const pthread_rwlockattr_t *attr, int *pref)
{
}

/* <pthread.h> Set reader/write preference.  */
int pthread_rwlockattr_setkind_np(pthread_rwlockattr_t *attr, int pref)
{
}

/* <pthread.h> Initialize the spinlock LOCK.  If PSHARED is nonzero the */
/* spinlock can be shared between different processes.                  */
int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
}

/* <pthread.h> Destroy the spinlock LOCK.  */
int pthread_spin_destroy(pthread_spinlock_t *lock)
{
}

/* <pthread.h> Wait until spinlock LOCK is retrieved.  */
int pthread_spin_lock(pthread_spinlock_t *lock)
{
}

/* <pthread.h> Try to lock spinlock LOCK.  */
int pthread_spin_trylock(pthread_spinlock_t *lock)
{
}

/* <pthread.h> Release spinlock LOCK.  */
int pthread_spin_unlock(pthread_spinlock_t *lock)
{
}

/* <pthread.h> */
int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count)
{
}

/* <pthread.h> */
int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
}

/* <pthread.h> */
int pthread_barrierattr_init(pthread_barrierattr_t *attr)
{
}

/* <pthread.h> */
int pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{
}

/* <pthread.h> */
int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared)
{
}

/* <pthread.h> */
int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
{
}

/* <pthread.h> */
int pthread_barrier_wait(pthread_barrier_t *barrier)
{
}


/* <pthread.h> Create a key value identifying a location in the     */
/* thread-specific data area.  Each thread maintains a distinct     */
/* thread-specific data area.  DESTR_FUNCTION, if non-NULL, is      */
/* called with the value associated to that key when the key is     */
/* destroyed.  DESTR_FUNCTION is not called if the value associated */
/* is NULL when the key is destroyed.                               */
int pthread_key_create(pthread_key_t *key, void (*destr_function) (void *))
{
}

/* <pthread.h> Destroy KEY.  */
int pthread_key_delete(pthread_key_t key)
{
}

/* <pthread.h> Store POINTER in the thread-specific data slot identified */
/* by KEY.                                                               */
int pthread_setspecific(pthread_key_t key, const void *pointer)
{
}

/* <pthread.h> Return current value of the thread-specific data slot */
/* identified by KEY.                                                */
void *pthread_getspecific(pthread_key_t key)
{
}


/* <pthread.h> Guarantee that the initialization function INIT_ROUTINE */
/* will be called only once, even if pthread_once is executed several  */
/* times with the same ONCE_CONTROL argument. ONCE_CONTROL must point  */
/* to a static or extern variable initialized to PTHREAD_ONCE_INIT.    */
int pthread_once(pthread_once_t *once_control, void (*init_routine) (void))
{
}


/* <pthread.h> Set cancelability state of current thread to STATE,  */
/* returning old state in *OLDSTATE if OLDSTATE is not NULL.        */
int pthread_setcancelstate(int state, int *oldstate)
{
}

/* <pthread.h> Set cancellation state of current thread to TYPE, returning */
/* the old type in *OLDTYPE if OLDTYPE is not NULL.                        */
int pthread_setcanceltype(int type, int *oldtype)
{
}

/* <pthread.h> Cancel THREAD immediately or at the next possibility.  */
int pthread_cancel(pthread_t thread)
{
}

/* <pthread.h> Test for pending cancellation for the current thread and */
/* terminate the thread as per pthread_exit(PTHREAD_CANCELED) if it has */
/* been canceled.                                                       */
void pthread_testcancel(void)
{
}


/* <pthread.h> Install a cleanup handler: ROUTINE will be called with    */
/* arguments ARG when the thread is canceled or calls pthread_exit.      */
/* ROUTINE will also be called with arguments ARG when the matching      */
/* pthread_cleanup_pop is executed with non-zero EXECUTE argument.       */
/* pthread_cleanup_push and pthread_cleanup_pop are macros and must      */
/* always be used in matching pairs at the same nesting level of braces. */
#define pthread_cleanup_push(routine,arg) ...

/* <pthread.h> Remove a cleanup handler installed by the matching      */
/* pthread_cleanup_push.  If EXECUTE is non-zero, the handler function */
/* is called.                                                          */
#define pthread_cleanup_pop(execute) ...

/* <pthread.h> Install a cleanup handler as pthread_cleanup_push does, */
/* but also saves the current cancellation type and set it to deferred */
/* cancellation.  Only if __USE_GNU is defined.                        */
#define pthread_cleanup_push_defer_np(routine,arg) ...

/* <pthread.h> Remove a cleanup handler as pthread_cleanup_pop does, but    */
/* also restores the cancellation type that was in effect when the matching */
/* pthread_cleanup_push_defer was called.  Only if __USE_GNU is defined.    */
# define pthread_cleanup_pop_restore_np(execute) ...

/* <pthread.h> Get ID of CPU-time clock for thread THREAD_ID.  */
int pthread_getcpuclockid(pthread_t thread_id, clockid_t *clock_id)
{
}


/* <pthread.h> Modify the signal mask for the calling thread.  The */
/* arguments have the same meaning as for sigprocmask(2).          */
int pthread_sigmask(int how, const sigset_t *newmask, sigset_t *oldmask)
{
}

/* <pthread.h> Send signal SIGNO to the given thread. */
int pthread_kill(pthread_t thread, int signo)
{
}


/* <pthread.h> Install handlers to be called when a new process is created   */
/* with FORK.  The PREPARE handler is called in the parent process just      */
/* before performing FORK. The PARENT handler is called in the parent        */
/* process just after FORK.  The CHILD handler is called in the child        */
/* process.  Each of the three handlers can be NULL, meaning that no handler */
/* needs to be called at that point.  PTHREAD_ATFORK can be called several   */
/* times, in which case the PREPARE handlers are called in LIFO order (last  */
/* added with PTHREAD_ATFORK, first called before FORK), and the PARENT and  */
/* CHILD handlers are called in FIFO (first added, first called).            */
int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
}

/* <pthread.h> Terminate all threads in the program except the calling */
/* process.  Should be called just before invoking one of the exec*()  */
/* functions.                                                          */
void pthread_kill_other_threads_np(void)
{
}
