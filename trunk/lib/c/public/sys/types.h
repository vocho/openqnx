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
 *  sys/types.h Defined system types
 *

 */
#ifndef __TYPES_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define __TYPES_H_INCLUDED
#endif

#ifndef __TYPES_H_DECLARED
#define __TYPES_H_DECLARED

_C_STD_BEGIN

#if defined(__MODE_T)
typedef __MODE_T	mode_t;
#undef __MODE_T
#endif

#if defined(__DEV_T)
typedef __DEV_T		dev_t;
#undef __DEV_T
#endif

#if defined(__SSIZE_T)
typedef __SSIZE_T	ssize_t;
#undef __SSIZE_T
#endif

#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif

#if defined(__TIME_T)
typedef __TIME_T	time_t;
#undef __TIME_T
#endif

#if defined(__CLOCK_T)
typedef __CLOCK_T	clock_t;
#undef __CLOCK_T
#endif

_C_STD_END

#if defined(__PID_T)
typedef __PID_T		pid_t;
#undef __PID_T
#endif

#if defined(__BLKCNT_T)
typedef __BLKCNT_T		blkcnt_t;
#undef __BLKCNT_T
#endif

#if defined(__BLKSIZE_T)
typedef __BLKSIZE_T		blksize_t;
#undef __BLKSIZE_T
#endif

#if defined(__FSBLKCNT_T)
typedef __FSBLKCNT_T		fsblkcnt_t;
#undef __FSBLKCNT_T
#endif

#if defined(__FSFILCNT_T)
typedef __FSFILCNT_T		fsfilcnt_t;
#undef __FSFILCNT_T
#endif

#if defined(__NLINK_T)
typedef __NLINK_T	nlink_t;
#undef __NLINK_T
#endif

#if defined(__OFF_T)
typedef __OFF_T		off_t;
#undef __OFF_T
#endif

#if defined(__OFF64_T)
typedef __OFF64_T	off64_t;
#undef __OFF64_T
#endif

#if defined(__INO_T)
typedef __INO_T		ino_t;
#undef __INO_T
#endif

#if defined(__INO64_T)
typedef __INO64_T	ino64_t;
#undef __INO64_T
#endif

#if defined(__ID_T)
typedef __ID_T		id_t;
#undef __ID_T
#endif

#if defined(__UID_T)
typedef __UID_T		uid_t;
#undef __UID_T
#endif

#if defined(__GID_T)
typedef __GID_T		gid_t;
#undef __GID_T
#endif

#if defined(__USECONDS_T)
typedef __USECONDS_T useconds_t;
#undef __USECONDS_T
#endif

#if defined(__SUSECONDS_T)
typedef __SUSECONDS_T suseconds_t;
#undef __SUSECONDS_T
#endif

#if defined(__TIMER_T)
typedef __TIMER_T	timer_t;
#undef __TIMER_T
#endif

#if defined(__CLOCKID_T)
typedef __CLOCKID_T	clockid_t;
#undef __CLOCKID_T
#endif

#if defined(__PTHREAD_T)
typedef __PTHREAD_T	pthread_t;
#undef __PTHREAD_T
#endif

#if defined(__SYNC_T)
typedef __SYNC_T	sync_t;
#undef __SYNC_T
#endif

#if defined(__SYNC_ATTR_T)
typedef __SYNC_ATTR_T	sync_attr_t;
#undef __SYNC_ATTR_T
#endif

#if defined(__PTHREAD_ATTR_T)
#if defined(__TIMESPEC_INTERNAL)
__TIMESPEC_INTERNAL;
#undef __TIMESPEC_INTERNAL
#endif
#if defined(__SCHED_PARAM_INTERNAL)
__SCHED_PARAM_INTERNAL;
#undef __SCHED_PARAM_INTERNAL
#endif
typedef __PTHREAD_ATTR_T	pthread_attr_t;
#undef __PTHREAD_ATTR_T
#endif

#if defined(__PTHREAD_COND_T)
typedef __PTHREAD_COND_T	pthread_cond_t;
#undef __PTHREAD_COND_T
#endif

#if defined(__PTHREAD_CONDATTR_T)
typedef __PTHREAD_CONDATTR_T	pthread_condattr_t;
#undef __PTHREAD_CONDATTR_T
#endif

#if defined(__PTHREAD_KEY_T)
typedef __PTHREAD_KEY_T	pthread_key_t;
#undef __PTHREAD_KEY_T
#endif

#if defined(__PTHREAD_MUTEX_T)
typedef __PTHREAD_MUTEX_T	pthread_mutex_t;
#undef __PTHREAD_MUTEX_T
#endif

#if defined(__PTHREAD_MUTEXATTR_T)
typedef __PTHREAD_MUTEXATTR_T	pthread_mutexattr_t;
#undef __PTHREAD_MUTEXATTR_T
#endif

#if defined(__PTHREAD_ONCE_T)
typedef __PTHREAD_ONCE_T	pthread_once_t;
#undef __PTHREAD_ONCE_T
#endif

#if defined(__PTHREAD_RWLOCK_T)
typedef __PTHREAD_RWLOCK_T	pthread_rwlock_t;
#undef __PTHREAD_RWLOCK_T
#endif

#if defined(__PTHREAD_RWLOCKATTR_T)
typedef __PTHREAD_RWLOCKATTR_T	pthread_rwlockattr_t;
#undef __PTHREAD_RWLOCKATTR_T
#endif

#if defined(__KEY_T)
typedef __KEY_T	key_t;
#undef __KEY_T
#endif

typedef char *          caddr_t;
typedef _Paddrt         paddr_t;  
typedef _Paddr32t		paddr32_t;
typedef _Paddr64t		paddr64_t;
typedef _Int32t         daddr_t;

#if defined(__EXT_QNX)
typedef _Uint16t		msg_t;      /* Used for message passing     */

#if defined(__IOVEC_T)
typedef __IOVEC_T	iov_t;
#undef __IOVEC_T
#endif

#endif

#if defined(__EXT_QNX)
#if defined(__CLOCKADJUST)
struct _clockadjust __CLOCKADJUST;
#undef __CLOCKADJUST
#endif
#endif

#if defined(__EXT_UNIX_MISC)
#define minor(device)                   ((int)((device) & 0x3ff))
#define major(device)                   ((int)(((device) >> 10) & 0x3f))
#define makedev(node,major,minor)       ((dev_t)(((node) << 16) | ((major) << 10) | (minor)))

#ifndef _UCHAR_T_DEFINED
#define _UCHAR_T_DEFINED
typedef unsigned char   uchar_t;
#endif
#ifndef _USHORT_T_DEFINED
#define _USHORT_T_DEFINED
typedef unsigned short  ushort_t;
#endif
#ifndef _UINT_T_DEFINED
#define _UINT_T_DEFINED
typedef unsigned int    uint_t;
#endif
#ifndef _ULONG_T_DEFINED
#define _ULONG_T_DEFINED
typedef unsigned long   ulong_t;
#endif
#ifndef _U_CHAR_DEFINED
#define _U_CHAR_DEFINED
typedef unsigned char   u_char;
#endif
#ifndef _U_SHORT_DEFINED
#define _U_SHORT_DEFINED
typedef unsigned short  u_short;
#endif
#ifndef _U_INT_DEFINED
#define _U_INT_DEFINED
typedef unsigned int    u_int;
#endif
#ifndef _U_LONG_DEFINED
#define _U_LONG_DEFINED
typedef unsigned long   u_long;
#endif
#ifndef _FIXPT_T_DEFINED
#define _FIXPT_T_DEFINED
typedef unsigned long   fixpt_t;
#endif
#ifndef _SEL_T_DEFINED
#define _SEL_T_DEFINED
typedef unsigned short  sel_t;
#endif
#endif

#ifdef BSD
typedef struct _uquad   { u_long val[2]; } u_quad;
typedef struct _quad    {   long val[2]; } quad;
typedef quad *          qaddr_t;
#endif

#if defined(__EXT_QNX)
#if defined(__ITIMER)
struct _itimer __ITIMER;
#undef __ITIMER
#endif
#endif

#endif

#ifdef _STD_USING
using _CSTD mode_t; using _CSTD dev_t; using _CSTD clock_t;
using _CSTD size_t; using _CSTD ssize_t; using _CSTD time_t;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("types.h $Rev: 173311 $"); */
