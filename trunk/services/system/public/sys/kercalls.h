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
 *  sys/kercalls.h
 *

 */
#ifndef __KERCALLS_H_INCLUDED
#define __KERCALLS_H_INCLUDED

enum {
	__KER_NOP,						/*  0		0x00 */
	__KER_TRACE_EVENT,				/*  1		0x01 */
	__KER_RING0,					/*  2		0x02 */
	__KER_SPARE1,					/*  3		0x03 */
	__KER_SPARE2,					/*  4		0x04 */
	__KER_SPARE3,					/*  5		0x05 */
	__KER_SPARE4,					/*  6		0x06 */

	__KER_SYS_CPUPAGE_GET,			/*  7		0x07 */
	__KER_SYS_CPUPAGE_SET,			/*  8		0x08 */
	__KER_SYS_SPARE1,				/*  9		0x09 */

	__KER_MSG_CURRENT,				/* 10		0x0a */
	__KER_MSG_SENDV,				/* 11		0x0b */
	__KER_MSG_SENDVNC,				/* 12		0x0c */
	__KER_MSG_ERROR,				/* 13		0x0d */
	__KER_MSG_RECEIVEV,				/* 14		0x0e */
	__KER_MSG_REPLYV,				/* 15		0x0f */
	__KER_MSG_READV,				/* 16		0x10 */
	__KER_MSG_WRITEV,				/* 17		0x11 */
	__KER_MSG_READWRITEV,			/* 18       0x12 */
	__KER_MSG_INFO,					/* 19		0x13 */
	__KER_MSG_SEND_PULSE,			/* 20		0x14 */
	__KER_MSG_DELIVER_EVENT,		/* 21		0x15 */
	__KER_MSG_KEYDATA,				/* 22		0x16 */
	__KER_MSG_READIOV,				/* 23		0x17 */
	__KER_MSG_RECEIVEPULSEV,		/* 24		0x18 */
	__KER_MSG_VERIFY_EVENT,			/* 25		0x19 */

	__KER_SIGNAL_KILL,				/* 26		0x1a */
	__KER_SIGNAL_RETURN,			/* 27		0x1b */
	__KER_SIGNAL_FAULT,				/* 28		0x1c */
	__KER_SIGNAL_ACTION,			/* 29		0x1d */
	__KER_SIGNAL_PROCMASK,			/* 30		0x1e */
	__KER_SIGNAL_SUSPEND,			/* 31		0x1f */
	__KER_SIGNAL_WAITINFO,			/* 32		0x20 */
	__KER_SIGNAL_SPARE1,			/* 33		0x21 */
	__KER_SIGNAL_SPARE2,			/* 34		0x22 */

	__KER_CHANNEL_CREATE,			/* 35		0x23 */
	__KER_CHANNEL_DESTROY,			/* 36		0x24 */
	__KER_CHANCON_ATTR,				/* 37		0x25 */
	__KER_CHANNEL_SPARE1,			/* 38		0x26 */

	__KER_CONNECT_ATTACH,			/* 39		0x27 */
	__KER_CONNECT_DETACH,			/* 40		0x28 */
	__KER_CONNECT_SERVER_INFO,		/* 41		0x29 */
	__KER_CONNECT_CLIENT_INFO,		/* 42		0x2a */
	__KER_CONNECT_FLAGS,			/* 43		0x2b */
	__KER_CONNECT_SPARE1,			/* 44		0x2c */
	__KER_CONNECT_SPARE2,			/* 45		0x2d */

	__KER_THREAD_CREATE,			/* 46		0x2e */
	__KER_THREAD_DESTROY,			/* 47		0x2f */
	__KER_THREAD_DESTROYALL,		/* 48		0x30 */
	__KER_THREAD_DETACH,			/* 49		0x31 */
	__KER_THREAD_JOIN,				/* 50		0x32 */
	__KER_THREAD_CANCEL,			/* 51		0x33 */
	__KER_THREAD_CTL,				/* 52		0x34 */
	__KER_THREAD_SPARE1,			/* 53		0x35 */
	__KER_THREAD_SPARE2,			/* 54		0x36 */

	__KER_INTERRUPT_ATTACH,			/* 55		0x37 */
	__KER_INTERRUPT_DETACH_FUNC,	/* 56		0x38 */
	__KER_INTERRUPT_DETACH,			/* 57		0x39 */
	__KER_INTERRUPT_WAIT,			/* 58		0x3a */
	__KER_INTERRUPT_MASK,			/* 59		0x3b */
	__KER_INTERRUPT_UNMASK,			/* 60		0x3c */
	__KER_INTERRUPT_SPARE1,			/* 61		0x3d */
	__KER_INTERRUPT_SPARE2,			/* 62		0x3e */
	__KER_INTERRUPT_SPARE3,			/* 63		0x3f */
	__KER_INTERRUPT_SPARE4,			/* 64		0x40 */

	__KER_CLOCK_TIME,				/* 65		0x41 */
	__KER_CLOCK_ADJUST,				/* 66		0x42 */
	__KER_CLOCK_PERIOD,				/* 67		0x43 */
	__KER_CLOCK_ID,					/* 68		0x44 */
	__KER_CLOCK_SPARE2,				/* 69		0x45 */

	__KER_TIMER_CREATE,				/* 70		0x46 */
	__KER_TIMER_DESTROY,			/* 71		0x47 */
	__KER_TIMER_SETTIME,			/* 72		0x48 */
	__KER_TIMER_INFO,				/* 73		0x49 */
	__KER_TIMER_ALARM,				/* 74		0x4a */
	__KER_TIMER_TIMEOUT,			/* 75		0x4b */
	__KER_TIMER_SPARE1,				/* 76		0x4c */
	__KER_TIMER_SPARE2,				/* 77		0x4d */

	__KER_SYNC_CREATE,				/* 78		0x4e */
	__KER_SYNC_DESTROY,				/* 79		0x4f */
	__KER_SYNC_MUTEX_LOCK,			/* 80		0x50 */
	__KER_SYNC_MUTEX_UNLOCK,		/* 81		0x51 */
	__KER_SYNC_CONDVAR_WAIT,		/* 82		0x52 */
	__KER_SYNC_CONDVAR_SIGNAL,		/* 83		0x53 */
	__KER_SYNC_SEM_POST,			/* 84		0x54 */
	__KER_SYNC_SEM_WAIT,			/* 85		0x55 */
	__KER_SYNC_CTL,					/* 86		0x56 */
	__KER_SYNC_MUTEX_REVIVE,		/* 87		0x57 */

	__KER_SCHED_GET,				/* 88		0x58 */
	__KER_SCHED_SET,				/* 89		0x59 */
	__KER_SCHED_YIELD,				/* 90		0x5a */
	__KER_SCHED_INFO,				/* 91		0x5b */
	__KER_SCHED_CTL,				/* 92		0x5c */

	__KER_NET_CRED,					/* 93		0x5d */
	__KER_NET_VTID,					/* 94		0x5e */
	__KER_NET_UNBLOCK,				/* 95		0x5f */
	__KER_NET_INFOSCOID,			/* 96		0x60 */
	__KER_NET_SIGNAL_KILL,			/* 97		0x61 */

	__KER_NET_SPARE1,				/* 98		0x62 */
	__KER_NET_SPARE2,				/* 99		0x63 */

	__KER_MT_CTL,                         /* 100          0x64 */

	__KER_BAD						/* 101 		0x65 */
	} ;

#endif

/* __SRCVERSION("kercalls.h $Rev: 153052 $"); */
