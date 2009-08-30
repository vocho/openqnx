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
 *  unix.h
 *

 *
 */
#ifndef _UNIX_H_INCLUDED
#define _UNIX_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <termcap.h>
#include <strings.h>
#include <stdlib.h>
#include <signal.h>
#include <process.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>

#include <_pack64.h>

union wait {
        int     w_status;                       /* used in syscall */
        /*
         * Terminated process status.
         */
#define w_termsig       w_T.w_Termsig
#define w_coredump      w_T.w_Coredump
#define w_retcode       w_T.w_Retcode
        struct {
                unsigned short  w_Termsig:7;    /* termination signal */
                unsigned short  w_Coredump:1;   /* core dump indicator */
                unsigned short  w_Retcode:8;    /* exit code if w_termsig==0 */
        } w_T;
        /*
         * Stopped process status.  Returned
         * only for traced children unless requested
         * with the WUNTRACED option bit.
         */
#define w_stopval       w_S.w_Stopval
#define w_stopsig       w_S.w_Stopsig
        struct {
                unsigned short  w_Stopval:8;    /* == W_STOPPED if stopped */
                unsigned short  w_Stopsig:8;    /* signal that stopped us */
        } w_S;
};

#include <_packpop.h>

typedef void (*sig_t)(int);

#define killpg(p,s)     kill(-(p),s)
#define nap             delay
#define signal          signal
#define sigmask(s)      (1L<<((s)-1))

#ifndef HZ
#define HZ				_sysconf(3)         /* 3 == _SC_CLK_TCK */
#endif

#define L_SET           SEEK_SET
#define L_INCR          SEEK_CUR
#define L_XTND          SEEK_END

#ifndef MAXNAMLEN
#ifndef NAME_MAX
#define MAXNAMLEN	_POSIX_NAME_MAX /* ick */
#else
#define MAXNAMLEN       NAME_MAX
#endif
#endif
#ifndef MAXPATHLEN
#define MAXPATHLEN      _POSIX_PATH_MAX
#endif

#ifndef NOFILE
#define NOFILE          20
#endif

#ifndef FASYNC
#define FASYNC			O_ASYNC
#endif

#ifndef roundup
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))
#endif

/*-
	These are for NetBSD compatibility
 */
#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#define __IDSTRING(name,string) \
        static const char name[] __attribute__((__unused__)) = string
#else
#define __IDSTRING(name,string) \
        static const char name[] = string
#endif

#ifndef __RCSID
#define __RCSID(s) __IDSTRING(rcsid,s)
#endif

#ifndef __COPYRIGHT
#define __COPYRIGHT(s) __IDSTRING(copyright,s)
#endif

/*
 * Round p (pointer or byte index) up to a correctly-aligned value
 * for all data types (int, long, ...).   The result is u_int and
 * must be cast to any desired pointer type.
 *
 * ALIGNED_POINTER is a boolean macro that checks whether an address
 * is valid to fetch data elements of type t from on this architecture.
 * This does not reflect the optimal alignment, just the possibility
 * (within reasonable limits).
 *
 */
#define ALIGNBYTES              (sizeof(int) - 1)
#define ALIGN(p)                (((unsigned int)(p) + ALIGNBYTES) &~ ALIGNBYTES)
#define ALIGNED_POINTER(p,t)    1

/*-
	End of NetBSD compatibility
 */

__BEGIN_DECLS

#ifndef alloca
extern void *  alloca(unsigned __size);
#endif
extern char *  re_comp(char *__s);
extern int     re_exec(char *__s);
extern int     setlogin(const char *__name);

extern int sigblock(int __mask);
extern int sigsetmask(int __mask);
extern int sigunblock(int __mask);

struct winsize;
struct termios;
extern int login_tty(int __fd);
extern int openpty(int *__amaster, int *__aslave, char *__name, struct termios *__termp, struct winsize *__winp);
extern pid_t forkpty(int *__amaster, char *__name, struct termios *__termp, struct winsize *__winp);

extern int setlinebuf(FILE *__stream);
extern void setbuffer(FILE *__stream, char *__buf, size_t __size);

extern int rdchk(int __fd);

__END_DECLS

#endif

/* __SRCVERSION("unix.h $Rev: 173156 $"); */
