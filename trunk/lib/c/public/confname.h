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
 *  confname.h
 *

 */

#ifndef _CONFNAME_H_INCLUDED
#define _CONFNAME_H_INCLUDED

#define _CS_NONE			0		/* End of lists in syspage */
#define _CS_SET				256		/* Bit for SET commands (after subracting 1 from command) */

/* 1-256 are Unix International GET commands */
#if defined(__EXT_POSIX2)
#define _CS_PATH			1		/* default path to find system utilities */
#endif
#if defined(__EXT_UNIX_MISC)
#define _CS_HOSTNAME		2		/* Name of this node within the communications network */
#define _CS_RELEASE			3		/* Current release level of this implementation */
#define _CS_VERSION			4		/* Current version of this release */
#define _CS_MACHINE			5		/* Name of the hardware type on which the system is running */
#define _CS_ARCHITECTURE	6		/* Name of the instructions set architechure */
#define _CS_HW_SERIAL		7		/* A serial number assiciated with the hardware */
#define _CS_HW_PROVIDER		8		/* The name of the hardware manufacturers */
#define _CS_SRPC_DOMAIN		9		/* The secure RPC domain */
#define _CS_SYSNAME			11		/* Name of this implementation of the operating system */
/* 257 - 512 are Unix International SET commands */
#define _CS_SET_HOSTNAME	(_CS_SET+_CS_HOSTNAME)		/* Set the node name within the communications network */
#define _CS_SET_SRPC_DOMAIN	(_CS_SET+_CS_SRPC_DOMAIN)	/* Set the secure RPC domain name */

#define _CS_VENDOR			513		/* Start of vendor specific commands */
#endif
#if defined(__EXT_QNX)

#define _CS_LIBPATH			200		/* default path for runtime to find standard shared objects */

#define _CS_DOMAIN			201		/* Domain of this node within the communications network */
#define _CS_RESOLVE			202		/* In memory /etc/resolve.conf */
#define _CS_TIMEZONE		203		/* timezone string (TZ style) */
#define _CS_LOCALE			204		/* locale string */
  
#define _CS_SET_DOMAIN		(_CS_SET+_CS_DOMAIN)	/* Set the domain for this node */
#define _CS_SET_RESOLVE		(_CS_SET+_CS_RESOLVE)	/* Set the in-memory /etc/resolve.conf */
#define _CS_SET_TIMEZONE	(_CS_SET+_CS_TIMEZONE)	/* Set the timezone for this node */
#define _CS_SET_LOCALE		(_CS_SET+_CS_LOCALE)	/* Set the locale for this node */

/* 16385-16640 are QNX GET commands */
#define _CS_DANH			16385

/* 16641-16896 are QNX SET commands */
#endif

/* Symbolic constants for confstr */

#if defined(__EXT_XOPEN_EX)
#define _CS_XBS5_ILP32_OFF32_CFLAGS			100
#define _CS_XBS5_ILP32_OFF32_LDFLAGS		101
#define _CS_XBS5_ILP32_OFF32_LIBS			102
#define _CS_XBS5_ILP32_OFF32_LINTFLAGS		103
#define _CS_XBS5_ILP32_OFFBIG_CFLAGS		104
#define _CS_XBS5_ILP32_OFFBIG_LDFLAGS		105
#define _CS_XBS5_ILP32_OFFBIG_LIBS			106
#define _CS_XBS5_ILP32_OFFBIG_LINTFLAGS		107
#define _CS_XBS5_LP64_OFF64_CFLAGS			108
#define _CS_XBS5_LP64_OFF64_LDFLAGS			109
#define _CS_XBS5_LP64_OFF64_LIBS			110
#define _CS_XBS5_LP64_OFF64_LINTFLAGS		111
#define _CS_XBS5_LPBIG_OFFBIG_CFLAGS		112
#define _CS_XBS5_LPBIG_OFFBIG_LDFLAGS		113
#define _CS_XBS5_LPBIG_OFFBIG_LIBS			114
#define _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS		115
#endif

#if defined(__EXT_POSIX1_200112)
#define _CS_POSIX_V6_ILP32_OFF32_CFLAGS		116
#define _CS_POSIX_V6_ILP32_OFF32_LDFLAGS	117
#define _CS_POSIX_V6_ILP32_OFF32_LIBS		118
#define _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS	119
#define _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS	120
#define _CS_POSIX_V6_ILP32_OFFBIG_LIBS		121
#define _CS_POSIX_V6_LP64_OFF64_CFLAGS		122
#define _CS_POSIX_V6_LP64_OFF64_LDFLAGS		123
#define _CS_POSIX_V6_LP64_OFF64_LIBS		124
#define _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS	125
#define _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS	126
#define _CS_POSIX_V6_LPBIG_OFFBIG_LIBS		127
#define _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS	128
#endif

/* Symbolic constants for sysconf */
#if defined(__EXT_POSIX1_199009)
#define _SC_ARG_MAX			1
#define _SC_CHILD_MAX		2
#define _SC_CLK_TCK			3
#define _SC_NGROUPS_MAX		4
#define _SC_OPEN_MAX		5
#define _SC_JOB_CONTROL		6
#define _SC_SAVED_IDS		7
#define _SC_VERSION			8
#define _SC_PASS_MAX		9
#define _SC_LOGNAME_MAX		10
#define _SC_PAGESIZE		11
#define _SC_PAGE_SIZE		_SC_PAGESIZE
#define _SC_XOPEN_VERSION	12
#define _SC_STREAM_MAX		13
#define _SC_TZNAME_MAX		14
#endif

#if defined(__EXT_POSIX1_199309)
#define _SC_AIO_LISTIO_MAX		15
#define _SC_AIO_MAX				16
#define _SC_AIO_PRIO_DELTA_MAX	17
#define _SC_DELAYTIMER_MAX		18
#define _SC_MQ_OPEN_MAX			19
#define _SC_MQ_PRIO_MAX			20
#define _SC_RTSIG_MAX			21
#define _SC_SEM_NSEMS_MAX		22
#define _SC_SEM_VALUE_MAX		23
#define _SC_SIGQUEUE_MAX		24
#define _SC_TIMER_MAX			25

#define _SC_ASYNCHRONOUS_IO		26
#define _SC_FSYNC				27
#define _SC_MAPPED_FILES		28
#define _SC_MEMLOCK				29
#define _SC_MEMLOCK_RANGE		30
#define _SC_MEMORY_PROTECTION	31
#define _SC_MESSAGE_PASSING		32
#define _SC_PRIORITIZED_IO		33
#define _SC_PRIORITY_SCHEDULING	34
#define _SC_REALTIME_SIGNALS	35
#define _SC_SEMAPHORES			36
#define _SC_SHARED_MEMORY_OBJECTS	37
#define _SC_SYNCHRONIZED_IO		38
#define _SC_TIMERS				39
#endif

#if defined(__EXT_POSIX1_199506)
#define _SC_GETGR_R_SIZE_MAX	40
#define _SC_GETPW_R_SIZE_MAX	41
#define _SC_LOGIN_NAME_MAX		42
#define _SC_THREAD_DESTRUCTOR_ITERATIONS	43
#define _SC_THREAD_KEYS_MAX		44
#define _SC_THREAD_STACK_MIN	45
#define _SC_THREAD_THREADS_MAX	46
#define _SC_TTY_NAME_MAX		47

#define _SC_THREADS				48
#define _SC_THREAD_ATTR_STACKADDR	49
#define _SC_THREAD_ATTR_STACKSIZE	50
#define _SC_THREAD_PRIORITY_SCHEDULING	51
#define _SC_THREAD_PRIO_INHERIT		52
#define _SC_THREAD_PRIO_PROTECT		53
#define _SC_THREAD_PROCESS_SHARED	54
#define _SC_THREAD_SAFE_FUNCTIONS	55
#endif

#if defined(__EXT_QNX) || defined(__EXT_POSIX1_200112)
#define _SC_2_CHAR_TERM			56
#define _SC_2_C_BIND			57
#define _SC_2_C_DEV				58
#define _SC_2_C_VERSION			59
#define _SC_2_FORT_DEV			60
#define _SC_2_FORT_RUN			61
#define _SC_2_LOCALEDEF			62
#define _SC_2_SW_DEV			63
#define _SC_2_UPE				64
#define _SC_2_VERSION			65
#define _SC_ATEXIT_MAX			66
#endif
#if defined(__EXT_QNX)
#define _SC_AVPHYS_PAGES		67
#endif
#if defined(__EXT_QNX) || defined(__EXT_POSIX1_200112)
#define _SC_BC_BASE_MAX			68
#define _SC_BC_DIM_MAX			69
#define _SC_BC_SCALE_MAX		70
#define _SC_BC_STRING_MAX		71
#endif
#if defined(__EXT_QNX)
#define _SC_CHARCLASS_NAME_MAX	72
#define _SC_CHAR_BIT			73
#define _SC_CHAR_MAX			74
#define _SC_CHAR_MIN			75
#endif
#if defined(__EXT_QNX) || defined(__EXT_POSIX1_200112)
#define _SC_COLL_WEIGHTS_MAX	76
#endif
#if defined(__EXT_QNX)
#define _SC_EQUIV_CLASS_MAX		77
#endif
#if defined(__EXT_QNX) || defined(__EXT_POSIX1_200112)
#define _SC_EXPR_NEST_MAX		78
#endif
#if defined(__EXT_QNX)
#define _SC_INT_MAX				79
#define _SC_INT_MIN				80
#endif
#if defined(__EXT_QNX) || defined(__EXT_POSIX1_200112)
#define _SC_LINE_MAX			81
#endif
#if defined(__EXT_QNX)
#define _SC_LONG_BIT			82
#define _SC_MB_LEN_MAX			83
#define _SC_NL_ARGMAX			84
#define _SC_NL_LANGMAX			85
#define _SC_NL_MSGMAX			86
#define _SC_NL_NMAX				87
#define _SC_NL_SETMAX			88
#define _SC_NL_TEXTMAX			89
#define _SC_NPROCESSORS_CONF	90
#define _SC_NPROCESSORS_ONLN	91
#define _SC_NZERO				92
#define _SC_PHYS_PAGES			93
#define _SC_PII					94
#define _SC_PII_INTERNET		95
#define _SC_PII_INTERNET_DGRAM	96
#define _SC_PII_INTERNET_STREAM	97
#define _SC_PII_OSI				98
#define _SC_PII_OSI_CLTS		99
#define _SC_PII_OSI_COTS		100
#define _SC_PII_OSI_M			101
#define _SC_PII_SOCKET			102
#define _SC_PII_XTI				103
#define _SC_POLL				104
#endif
#if defined(__EXT_QNX) || defined(__EXT_POSIX1_200112)
#define _SC_RE_DUP_MAX			105
#endif
#if defined(__EXT_QNX)
#define _SC_SCHAR_MAX			106
#define _SC_SCHAR_MIN			107
#define _SC_SELECT				108
#define _SC_SHRT_MAX			109
#define _SC_SHRT_MIN			110
#define _SC_SSIZE_MAX			111
#define _SC_T_IOV_MAX			112
#define _SC_UCHAR_MAX			113
#define _SC_UINT_MAX			114
#define _SC_UIO_MAXIOV			115
#define _SC_ULONG_MAX			116
#define _SC_USHRT_MAX			117
#define _SC_WORD_BIT			118
#endif
#if defined(__EXT_QNX) || defined(__EXT_POSIX1_200112)
#define _SC_XOPEN_CRYPT			119
#define _SC_XOPEN_ENH_I18N		120
#define _SC_XOPEN_SHM			121
#define _SC_XOPEN_UNIX			122
#endif
#if defined(__EXT_QNX)
#define _SC_XOPEN_XCU_VERSION	123
#define _SC_XOPEN_XPG2			124
#define _SC_XOPEN_XPG3			125
#define _SC_XOPEN_XPG4			126
#endif

#if defined(__EXT_XOPEN_EX)
#define _SC_XBS5_ILP32_OFF32	127
#define _SC_XBS5_ILP32_OFFBIG	128
#define _SC_XBS5_LP64_OFF64		129
#define _SC_XBS5_LPBIG_OFFBIG	130
#endif

#if defined (__EXT_POSIX1_200112)
/* POSIX 1003.1d-1999 */
#define _SC_ADVISORY_INFO			131
#define _SC_CPUTIME					132
#define _SC_SPAWN					133
#define _SC_SPORADIC_SERVER			134
#define _SC_THREAD_CPUTIME			135
#define _SC_THREAD_SPORADIC_SERVER	136
#define _SC_TIMEOUTS				137

/* POSIX 1003.1j-2000 */
#define _SC_BARRIERS				138
#define _SC_CLOCK_SELECTION			139
#define _SC_MONOTONIC_CLOCK			140
#define _SC_READER_WRITER_LOCKS		141
#define _SC_SPIN_LOCKS				142
#define _SC_TYPED_MEMORY_OBJECTS	143

/* POSIX 1003.1q-2000 */
#define _SC_TRACE_EVENT_FILTER		144
#define _SC_TRACE					145
#define _SC_TRACE_INHERIT			146
#define _SC_TRACE_LOG				147

/* POSIX 1003.1-2001 */
#define _SC_2_PBS							148
#define _SC_2_PBS_ACCOUNTING				149
#define _SC_2_PBS_CHECKPOINT				150
#define _SC_2_PBS_LOCATE					151
#define _SC_2_PBS_MESSAGE					152
#define _SC_2_PBS_TRACK						153
#define _SC_HOST_NAME_MAX					154
#define _SC_IOV_MAX							155
#define _SC_IPV6							156
#define _SC_RAW_SOCKETS						157
#define _SC_REGEXP							158
#define _SC_SHELL							159
#define _SC_SS_REPL_MAX						160
#define _SC_SYMLOOP_MAX						161
#define _SC_TRACE_EVENT_NAME_MAX				162
#define _SC_TRACE_NAME_MAX					163
#define _SC_TRACE_SYS_MAX					164
#define _SC_TRACE_USER_EVENT_MAX			165
#define _SC_V6_ILP32_OFF32					166
#define _SC_V6_ILP32_OFFBIG					167
#define _SC_V6_LP64_OFF64					168
#define _SC_V6_LPBIG_OFFBIG					169
#define _SC_XOPEN_REALTIME					170
#define _SC_XOPEN_REALTIME_THREADS			171
#define _SC_XOPEN_LEGACY					172
#define _SC_XOPEN_STREAMS					173
#endif

/* Symbolic constants for pathconf fpathconf */
#if defined(__EXT_POSIX1_198808)
#define _PC_LINK_MAX			1
#define _PC_MAX_CANON			2
#define _PC_MAX_INPUT			3
#define _PC_NAME_MAX			4
#define _PC_PATH_MAX			5
#define _PC_PIPE_BUF			6
#define _PC_NO_TRUNC			7
#define _PC_VDISABLE			8
#define _PC_CHOWN_RESTRICTED	9
#endif

#if defined(__EXT_QNX)
#define _PC_DOS_SHARE			10 /* test for dos share support */
#define _PC_IMAGE_VADDR			11
#endif

#if defined(__EXT_POSIX1_199309)
#define _PC_ASYNC_IO			12
#define _PC_PRIO_IO				13
#define _PC_SYNC_IO				14
#endif

#if defined(__EXT_QNX)
#define _PC_SOCK_MAXBUF			15
#endif

#if defined(__EXT_XOPEN_EX) || defined(__EXT_POSIX1_200112)
#define _PC_FILESIZEBITS		16
#endif

#if defined(__EXT_QNX) || defined(__EXT_POSIX1_200112)
#define _PC_SYMLINK_MAX			17
#endif
#if defined(__EXT_QNX)
#define _PC_SYMLOOP_MAX			18
#define _PC_LINK_DIR            19
#endif

#if defined(__EXT_POSIX1_200112)
#define _PC_2_SYMLINKS			20
#define _PC_ALLOC_SIZE_MIN		21
#define _PC_REC_INCR_XFER_SIZE	22
#define _PC_REC_MAX_XFER_SIZE		23
#define _PC_REC_MIN_XFER_SIZE	24
#define _PC_REC_XFER_ALIGN		25
#endif

#endif



/* __SRCVERSION("confname.h $Rev: 153052 $"); */
