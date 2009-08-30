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
 *  sys/select.h
 *

 */

#ifndef __SELECT_H_INCLUDED
#define __SELECT_H_INCLUDED

#ifndef _LIMITS_H_INCLUDED
#include <limits.h>
#endif

#ifndef _SYS_TIME_H_
#include <sys/time.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

typedef int	__fd_mask;

#define __NFDBITS       ((int)(sizeof(__fd_mask) * CHAR_BIT))

#ifndef FD_SETSIZE
 #define FD_SETSIZE 256 
#endif

typedef struct fd_set {
    __fd_mask fds_bits[(FD_SETSIZE + __NFDBITS - 1) / __NFDBITS];
} fd_set;

#define FD_SET(fd, p)   ((p)->fds_bits[(fd)/__NFDBITS] |= \
                (1 << ((fd) % __NFDBITS)))

#define FD_CLR(fd, p)   ((p)->fds_bits[(fd)/__NFDBITS] &= \
                ~(1 << ((fd) % __NFDBITS)))

#define FD_ISSET(fd, p) ((p)->fds_bits[(fd)/__NFDBITS] & \
                (1 << ((fd) % __NFDBITS)))

#define FD_ZERO(p)  _CSTD memset((p), 0, sizeof *(p))


__BEGIN_DECLS
struct timeval;
extern int select(int __nfds, fd_set *__readfds, fd_set *__writefds, fd_set *__errorfds, struct timeval *__timeout);

#if defined(__EXT_QNX)
struct sigevent;
struct timespec;
union sigval;
extern int _select_event(int __maxfd, fd_set *__readfds, fd_set *__writefds, /* deprecated */
			fd_set *__exceptfds, const struct timespec *__ts,
			struct sigevent *__event, int (*eventwait)(const struct timespec *__ts,
			union sigval *__value, void *__arg), void *__arg);
extern int _select_block(const struct timespec *__ts, union sigval *__value, void *__arg); /* deprecated */
#endif

#if defined(__EXT_UNIX_MISC)
typedef __fd_mask	fd_mask;
#define NFDBITS		__NFDBITS
#endif

#ifndef __MEMSET_DEFINED
#define __MEMSET_DEFINED

_C_STD_BEGIN
#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif

extern void *memset( void *__s, int __c, size_t __n );
_C_STD_END

#endif

__END_DECLS

#endif

/* __SRCVERSION("select.h $Rev: 172561 $"); */
