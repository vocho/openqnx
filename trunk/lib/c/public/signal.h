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
 *  signal.h    Signal definitions
 *

 */
#ifndef _SIGNAL_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _SIGNAL_H_INCLUDED
#endif

#ifndef _SIGNAL_H_DECLARED
#define _SIGNAL_H_DECLARED

#ifndef __SIGINFO_H_INCLUDED
#include <sys/siginfo.h>
#endif

_C_STD_BEGIN

typedef int     sig_atomic_t;

_C_STD_END

#if defined(__PTHREAD_T)
typedef __PTHREAD_T	pthread_t;
#undef __PTHREAD_T
#endif

#if defined(__SIGSET_T)
typedef __SIGSET_T	sigset_t;
#undef __SIGSET_T
#endif

#if defined(__STACK_T)
typedef __STACK_T	stack_t;
#undef __STACK_T
#endif

#if defined(__UCONTEXT_T)
typedef __UCONTEXT_T	ucontext_t;
#undef __UCONTEXT_T
#endif

#include <_pack64.h>

#if defined(__cplusplus) || defined(__STRICT_ANSI__) || defined(__STRICT_PROTYPES__)
	#define _SIG_ARGS	int
#else
	#define _SIG_ARGS
#endif

struct sigaction {
#if defined(__WATCOMC__) && defined(__EXT)
	union {
		void		(*sa_handler)(_SIG_ARGS);
		void		(*sa_sigaction)(int, siginfo_t *, void *);
		};
#else
#define sa_handler		__sa_un._sa_handler
#define sa_sigaction	__sa_un._sa_sigaction
	union {
		void		(*_sa_handler)(_SIG_ARGS);
		void		(*_sa_sigaction)(int, siginfo_t *, void *);
		} __sa_un;
#endif
    int         sa_flags;
    sigset_t    sa_mask;
};

#define SIG_ERR             ((void(*)(_SIG_ARGS)) -1)
#define SIG_DFL             ((void(*)(_SIG_ARGS))  0)
#define SIG_IGN             ((void(*)(_SIG_ARGS))  1)
#define SIG_HOLD            ((void(*)(_SIG_ARGS))  2)

#define SIGHUP      1   /* hangup */
#define SIGINT      2   /* interrupt */
#define SIGQUIT     3   /* quit */
#define SIGILL      4   /* illegal instruction (not reset when caught) */
#define SIGTRAP     5   /* trace trap (not reset when caught) */
#define SIGIOT      6   /* IOT instruction */
#define SIGABRT     6   /* used by abort */
#define SIGEMT      7   /* EMT instruction */
#define SIGDEADLK	7	/* Mutex deadlock */
#define SIGFPE      8   /* floating point exception */
#define SIGKILL     9   /* kill (cannot be caught or ignored) */
#define SIGBUS      10  /* bus error */
#define SIGSEGV     11  /* segmentation violation */
#define SIGSYS      12  /* bad argument to system call */
#define SIGPIPE     13  /* write on pipe with no reader */
#define SIGALRM     14  /* real-time alarm clock */
#define SIGTERM     15  /* software termination signal from kill */
#define SIGUSR1     16  /* user defined signal 1 */
#define SIGUSR2     17  /* user defined signal 2 */
#define SIGCHLD     18  /* death of child */
#define SIGCLD      SIGCHLD
#define SIGPWR      19  /* power-fail restart */
#define SIGWINCH    20  /* window change */
#define SIGURG      21  /* urgent condition on I/O channel */
#define SIGPOLL     22  /* System V name for SIGIO */
#define SIGIO       SIGPOLL
#define SIGSTOP     23  /* sendable stop signal not from tty */
#define SIGTSTP     24  /* stop signal from tty */
#define SIGCONT     25  /* continue a stopped process */
#define SIGTTIN     26  /* attempted background tty read */
#define SIGTTOU     27  /* attempted background tty write */
#define SIGVTALRM   28  /* virtual timer expired */
#define SIGPROF     29  /* profileing timer expired */
#define SIGXCPU     30  /* exceded cpu limit */
#define SIGXFSZ     31  /* exceded file size limit */

#define _SIGMIN     1
#define _SIGMAX     64
#define _NSIG       57
#if defined(__EXT_QNX)
#define NSIG        _NSIG  /* valid user signals range from 1 to NIG-1	*/
#define MAXSIG      64     /* for kernel use:  NSIG-1 <= MAXSIG */
#define SIG_TERMER_NOINHERIT 0x80000000 /* termination should _not_ inherit killer's prio */
#endif
#define SIGRTMIN    41
#define SIGRTMAX    56

/*The range SIGRTMIN through SIGRTMAX inclusive includes at least RTSIG_MAX signal numbers. */

/*
 * The top 8 signals are special. They are always masked. Attempts to unmask
 * will be ignored. This makes them handy for doing synchronous I/O via
 * sigwaitinfo(). As effecient as a pulse without the need for a channel
 * and connection. Select uses this for example.
 */
#define SIGSELECT   (SIGRTMAX + 1)
#define SIGPHOTON   (SIGRTMAX + 2)

/* sigprocmask() flags */
#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2
#define SIG_PENDING 5     /* Neutrino specific */

#define SA_NOCLDSTOP    0x0001	/* Do not generate SIGCHLD when child stops */
#define SA_SIGINFO		0x0002	/* Queue signals and call with siginfo filled */
#define SA_RESETHAND	0x0004	/* Reset handler to SIG_DFL when handling signal */
/* #define SA_ONSTACK		0x0008 (not supported yet) */	/* Handle signal on signal stack */
#define SA_NODEFER		0x0010	/* Don't apply the sigmask when the signal is being delivered */
#define SA_NOCLDWAIT	0x0020	/* Do not generate zombies on unwaited child */
/* #define SA_RESTART      0x0040 (not supported yet) */	/* Restart the kernel call on signal return */
#define SA_MASK			0x00ff	/* Mask to special bits */

#define SS_ONSTACK		1		/* Take signals on alternate stack */
#define SS_DISABLE		2		/* Disable takeing signals on alternate stack */

struct sigstack {
	int       ss_onstack;  /* non-zero when signal stack is in use */
	void     *ss_sp;       /* signal stack pointer */
};


struct timespec;

__BEGIN_DECLS

#if defined(__EXT_ANSIC_199012)
_C_STD_BEGIN
extern int raise( int __sig );
extern void (*signal( int __sig, void (*__func)(_SIG_ARGS) ))(_SIG_ARGS);
_C_STD_END
#endif

#if defined(__EXT_POSIX1_198808)
extern int kill( pid_t __pid, int __signum );
extern int sigaction( int __signo, const struct sigaction *__act, struct sigaction *__oact );
extern int sigaddset( sigset_t *__set, int __signo );
extern int sigdelset( sigset_t *__set, int __signo );
extern int sigemptyset( sigset_t *__set );
extern int sigfillset( sigset_t *__set );
extern int sigismember( const sigset_t *__set, int __signo );
extern int sigpending( sigset_t *__set );
extern int sigprocmask( int __how, const sigset_t *__set, sigset_t *__oset );
extern int sigsuspend( const sigset_t *__sigmask );
#endif

#if defined(__EXT_POSIX1_199506)
extern int sigqueue( pid_t __pid, int __signum, const union sigval __val );
extern int sigwait( const sigset_t *__set, int *__sig );
extern int sigwaitinfo( const sigset_t *__sigmask, siginfo_t *__info );
extern int sigtimedwait(const sigset_t *__set, siginfo_t *__info, const struct timespec *__timeout);
#endif

#if defined(__EXT_QNX)
extern void __signalstub(void);
#endif

#if defined(__EXT_XOPEN_EX)
/*
 -- Not currently supported --

extern void (*bsd_signal(int __sig, void (*func)(int)))(int);
extern int sigaltstack(const stack_t *__ss, stack_t *__oss);
extern int sighold(int __sig);
extern int sigignore(int __sig);
extern int siginterrupt(int __sig, int __flag);
extern int sigrelse(int __sig);
extern void (*sigset(int __sig, void (*__disp)(int)))(int);
extern int sigstack(struct sigstack *__ss, struct sigstack *__oss);
*/
extern int killpg(pid_t __pgrp, int __signum);
extern int sigpause(int __sig);
#endif
#if defined(__EXT_XOPEN_EX) || defined(__EXT_POSIX1_200112)
extern int pthread_kill(pthread_t __thr, int __sig);
extern int pthread_sigmask(int __how, const sigset_t *__set, sigset_t *__oset);
#endif

#if defined(__EXT_UNIX_HIST)
_C_STD_BEGIN
extern void (*signal(int __sig, void (*__func)() ))();
_C_STD_END
#endif

#if defined(__EXT_UNIX_MISC)
extern const char * const sys_siglist[];
extern const int sys_nsig;
#endif

#include <_packpop.h>

__END_DECLS

#endif

#ifdef _STD_USING
using std::sig_atomic_t; using std::raise; using std::signal;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("signal.h $Rev: 172382 $"); */
