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
 *  sys/procmsg.h
 *

 */
#ifndef __PROCMSG_H_INCLUDED
#define __PROCMSG_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
#pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __SYSMSG_H_INCLUDED
#include _NTO_HDR_(sys/sysmsg.h)
#endif

#ifndef _SPAWN_H_INCLUDED
#include _NTO_HDR_(spawn.h)
#endif

#ifndef __SIGINFO_H_INCLUDED
#include _NTO_HDR_(sys/siginfo.h)
#endif

#ifndef __RESOURCE_H_INCLUDED
#include _NTO_HDR_(sys/resource.h)
#endif

#if defined(__ID_T)
typedef __ID_T		id_t;
#undef __ID_T
#endif

#if defined(__PID_T)
typedef __PID_T		pid_t;
#undef __PID_T
#endif

#if defined(__UID_T)
typedef __UID_T		uid_t;
#undef __UID_T
#endif

#if defined(__GID_T)
typedef __GID_T		gid_t;
#undef __GID_T
#endif

#define PROCMGR_PID				SYSMGR_PID
#define PROCMGR_CHID			SYSMGR_CHID
#define PROCMGR_COID			SYSMGR_COID
#define PROCMGR_HANDLE			SYSMGR_HANDLE

enum {
	_PROC_SPAWN = _PROCMGR_BASE,
	_PROC_WAIT,
	_PROC_FORK,
	_PROC_GETSETID,
	_PROC_SETPGID,
	_PROC_UMASK,
	_PROC_GUARDIAN,
	_PROC_SESSION,
	_PROC_DAEMON,
	_PROC_EVENT,
	_PROC_RESOURCE,
	_PROC_POSIX_SPAWN
};

enum {
	_PROC_SPAWN_START,
	_PROC_SPAWN_FD,
	_PROC_SPAWN_ARGS,
	_PROC_SPAWN_DONE,
	_PROC_SPAWN_DEBUG,
	_PROC_SPAWN_EXEC,
	_PROC_SPAWN_REMOTE,
	_POSIX_SPAWN_START
};

enum {
	_PROC_ID_GETID,
	_PROC_ID_SETUID,
	_PROC_ID_SETGID,
	_PROC_ID_SETEUID,
	_PROC_ID_SETEGID,
	_PROC_ID_SETREUID,
	_PROC_ID_SETREGID,
	_PROC_ID_SETGROUPS
};

enum {
	_PROC_EVENT_NOTIFY,
	_PROC_EVENT_TRIGGER,
	_PROC_EVENT_NOTIFY_ADD,
	_PROC_EVENT_NOTIFY_DEL
};

enum {
	_PROC_UMASK_SET,
	_PROC_UMASK_GET
};

enum {
	_PROC_RESOURCE_USAGE,
	_PROC_RESOURCE_GETLIMIT,
	_PROC_RESOURCE_SETLIMIT
};

#define _FORK_ASPACE	0x00000001	/* Don't share address space */
#define _FORK_NOFDS		0x00000002	/* Don't dup any fd's */
#define _FORK_NOZOMBIE	0x00000004	/* Don't allow waiting */

#include _NTO_HDR_(_pack64.h)

/*
 * Message of _PROC_SPAWN/_PROC_SPAWN_START
 */
#ifdef __EXT_QNX		/* struct inheritance is only defined in __EXT_QNX */
struct _proc_spawn {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint16t						searchlen;
	_Uint16t						pathlen;
	struct inheritance				parms;
	_Uint16t						nfds;
	_Uint16t						nargv;
	_Uint16t						narge;
	_Uint16t						reserved;	
	_Uint32t						nbytes;		/* Number of bytes of argv + arge with nulls */
	/* _Int32t						fd_map[nfds] */
	/* char							search[searchlen] */
	/* char							path[pathlen] */
	/* char							argv[nargv][] */
	/* char							arge[narge][] */
};

typedef union {
	struct _proc_spawn				i;
} proc_spawn_t;
#endif

/*
 * Message of _PROC_SPAWN/_PROC_SPAWN_ARGS
 */
struct _proc_spawn_args {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint32t						nbytes;
	_Uint32t						offset;
	_Uint32t						zero;
};

typedef union {
	struct _proc_spawn_args			i;
} proc_spawn_args_t;


/*
 * Message of _PROC_SPAWN/_PROC_SPAWN_DEBUG
 */
struct _proc_spawn_debug {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint32t						text_addr;
	_Uint32t						text_size;
	_Int32t							text_reloc;
	_Uint32t						data_addr;
	_Uint32t						data_size;
	_Int32t							data_reloc;
	char							name[1];
};

typedef union {
	struct _proc_spawn_debug		i;
} proc_spawn_debug_t;


/*
 * Message of _PROC_SPAWN/_PROC_SPAWN_FD
 */
struct _proc_spawn_fd {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint16t						flags;
	_Uint16t						nfds;
	_Int32t							base;
	pid_t							ppid;
	/* int							fd[nfds] */
};
#define _PROC_SPAWN_FD_LIST		0x0001		/* only fds from passed in list (otherwise all fds) */
#define _PROC_SPAWN_FD_NOCLOEXC	0x0002		/* no fds with CLOEXC set (i.e. spawn) */
#if 0
#define _PROC_SPAWN_FD_BASE		0x0004		/* start finding fd's from base */
#endif

struct _proc_spawn_fd_info {
	int								fd;
	_Uint32t						nd;
	_Uint32t						srcnd;
	pid_t							pid;
	_Int32t							chid;
	_Int32t							scoid;
	_Int32t							coid;
};

struct _proc_spawn_fd_reply {
	_Uint16t						flags;
	_Uint16t						nfds;
	/* struct _proc_spawn_fd_info	info[nfds]; */
};

#define _PROC_SPAWN_FDREPLY_MORE	0x00000001	/* More fd's to return, send again */

typedef union {
	struct _proc_spawn_fd			i;
	struct _proc_spawn_fd_reply		o;
} proc_spawn_fd_t;


/*
 * Message of _PROC_SPAWN/_PROC_SPAWN_DONE
 */
struct _proc_spawn_done {
	_Uint16t						type;
	_Uint16t						subtype;
	_Int32t							rcvid;
};

typedef union {
	struct _proc_spawn_done			i;
} proc_spawn_done_t;

/*
 * additional msg parts for remote spawn
 */
typedef struct {
	_Uint32t						key;
	_Uint32t						umask;
	_Uint16t						nfds;
	_Uint16t						root_len;
	_Uint16t						cwd_len;
	_Uint16t						flags;
	/*	char* root[PATH_MAX]	*/
	/*	char* cwd[PATH_MAX]	*/
	/*	struct _proc_spawn_fd_info	fd_array[]	*/
}	proc_spawn_remote_t;

/* definition for flags field */
#define _PROC_SPAWN_REMOTE_FLAGS_FDALLIN		0x1

typedef struct {
	_Uint32t						nd;
	pid_t							pid;
	_Uint32t						chid;
	_Int32t							size;
} spawn_remote_t;

#define SPAWN_REMOTE_FDARRAY_SIZE	10
#define SPAWN_REMOTE_REMOTEBUF_SIZE	sizeof(struct _proc_spawn_fd_info) * SPAWN_REMOTE_FDARRAY_SIZE + PATH_MAX * 2 + sizeof(proc_spawn_remote_t)
#define SPAWN_REMOTE_MSGBUF_SIZE	sizeof(spawn_remote_t) + SPAWN_REMOTE_REMOTEBUF_SIZE

/*
 * Message of _PROC_SPAWN/_PROC_SPAWN_DIR
 */

enum {
	_PROC_SPAWN_DIR_ALL,
	_PROC_SPAWN_DIR_ROOT,
	_PROC_SPAWN_DIR_CWD
};	
 
struct _proc_spawn_dir {
	_Uint16t						type;
	_Uint16t						subtype;
	_Int32t							ppid;
	_Uint16t						request;
	_Uint16t						pathmax;
};

struct _proc_spawn_dir_reply {
	_Uint16t						result;
	_Uint16t						rootlen;
};

typedef union {
	struct _proc_spawn_dir			i;
	struct _proc_spawn_dir_reply	o;
} proc_spawn_dir_t;

/*
 * Message of _PROC_POSIX_SPAWN
 */
struct _proc_posix_spawn {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint16t						pathlen;
	_Uint16t						nargv;
	_Uint16t						narge;
	_Uint16t						reserved[3];
	_Uint32t						filact_bytes;	/* size of the _posix_spawn_file_actions_t */
	_Uint32t						filact_obytes;	/* size of the _posix_spawn_file_actions_t.open */
	_Uint32t						argenv_bytes;	/* Number of bytes of argv + arge with nulls */
	_Uint32t						attr_bytes;		/* size of the _posix_spawnattr_t */
	/* _posix_spawnattr_t			attr;		  */
	/* _posix_spawn_file_actions_t	filact		  */
	/* char							path[pathlen] */
	/* char							argv[nargv][] */
	/* char							arge[narge][] */
};

typedef union {
	struct _proc_posix_spawn		i;
} proc_posixspawn_t;


/*
 * Message of _PROC_GETSETID
 */
struct _proc_getsetid {
	_Uint16t						type;
	_Uint16t						subtype;
	pid_t							pid;
	id_t							eid;
	id_t							rid;
	_Int32t							ngroups;
	/* gid_t						groups[ngroups]; */
};

struct _proc_getsetid_reply {
	_Uint32t						zero;
	pid_t							pgrp;
	pid_t							ppid;
	pid_t							sid;
	_Uint32t						reserved[3];
	struct _cred_info				cred;
};

typedef union {
	struct _proc_getsetid			i;
	struct _proc_getsetid_reply		o;
} proc_getsetid_t;


/*
 * Message of _PROC_SETPGID
 */
struct _proc_setpgid {
	_Uint16t						type;
	_Uint16t						zero;
	pid_t							pid;
	pid_t							pgid;
};

typedef union {
	struct _proc_setpgid			i;
} proc_setpgid_t;


/*
 * Message of _PROC_WAIT
 */
struct _proc_wait {
	_Uint16t						type;
	_Int16t							idtype;
	_Int32t							options;
	id_t							id;
};
	
typedef union {
	struct _proc_wait				i;
	siginfo_t						o;
} proc_wait_t;


/*
 * Message of _PROC_FORK
 */
struct _proc_fork {
	_Uint16t						type;
	_Uint16t						zero;
	_Uint32t						flags;
	_Uint64t						frame;
};

typedef union {
	struct _proc_fork				i;
} proc_fork_t;


/*
 * Message of _PROC_UMASK
 */
struct _proc_umask {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint32t						umask;
	pid_t							pid;
};

struct _proc_umask_reply {
	_Uint32t						zero1;
	_Uint32t						umask;
};

typedef union {
	struct _proc_umask				i;
	struct _proc_umask_reply		o;
} proc_umask_t;

/*
 * Message of _PROC_GUARDIAN
 */
struct _proc_guardian {
	_Uint16t						type;
	_Uint16t						subtype;
	pid_t							pid;
	_Uint32t						reserved;
};

struct _proc_guardian_reply {
	_Uint32t						zero1;
	pid_t							pid;
};

typedef union {
	struct _proc_guardian			i;
	struct _proc_guardian_reply		o;
} proc_guardian_t;

/*
 * Message of _PROC_SESSION
 */
struct _proc_session {
	_Uint16t						type;
	_Uint16t						subtype;
	pid_t							sid;
	_Int32t							id;
	_Uint32t						event;
};

typedef union {
	struct _proc_session			i;
} proc_session_t;

/*
 * Message of _PROC_DAEMON
 */
struct _proc_daemon {
	_Uint16t						type;
	_Uint16t						subtype;
	_Int32t							status;
	_Uint32t						flags;
	_Uint32t						reserved;
};

typedef union {
	struct _proc_daemon				i;
} proc_daemon_t;

/*
 * Message of _PROC_EVENT
 */
struct _proc_event {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint32t						flags;
	struct sigevent					event;
};

typedef union {
	struct _proc_event				i;
} proc_event_t;

struct _proc_event_del {
	_Uint16t						type;
	_Uint16t						subtype;
	int								id;
};

typedef union {
	struct _proc_event_del			i;
} proc_event_del_t;

/*
 * Message header for all _PROC_RESOURCE
 */
struct _proc_resource_hdr {
	_Uint16t						type;
	_Uint16t						subtype;
	pid_t							pid;
};

/*
 * Message of _PROC_RESOURCE/_PROC_RESOURCE_USAGE
 */
struct _proc_resource_usage {
	_Uint16t						type;
	_Uint16t						subtype;
	pid_t							pid;
	_Uint32t						who;
};

typedef union {
	struct _proc_resource_usage		i;
	struct rusage					o;
} proc_resource_usage_t;

/*
 * Message of _PROC_RESOURCE/_PROC_RESOURCE_GETLIMIT
 */
struct _proc_resource_getlimit {
	_Uint16t						type;
	_Uint16t						subtype;
	pid_t							pid;
	_Uint32t						count;
	_Uint32t						reserved;
	_Uint32t						resource[1];
};

typedef union {
	struct _proc_resource_getlimit	i;
#if _LARGEFILE64_SOURCE - 0 > 0
	struct rlimit64					o[1];
#else
	struct rlimit					o[1];
#endif
} proc_resource_getlimit_t;

/*
 * Message of _PROC_RESOURCE/_PROC_RESOURCE_SETLIMIT
 */
struct _proc_resource_setlimit {
	_Uint16t						type;
	_Uint16t						subtype;
	pid_t							pid;
	_Uint32t						count;
	_Uint32t						reserved;
	struct _proc_resource_entry {
		_Uint32t						resource;
		_Uint32t						reserved;
#if _LARGEFILE64_SOURCE - 0 > 0
		struct rlimit64					limit;
#else
		struct rlimit					limit;
#endif
	}								entry[1];
};

typedef union {
	struct _proc_resource_setlimit	i;
} proc_resource_setlimit_t;

#include _NTO_HDR_(_packpop.h)

#endif

/* __SRCVERSION("procmsg.h $Rev: 170184 $"); */
