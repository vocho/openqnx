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

#ifndef STRUCT_H
#define STRUCT_H

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE		1
#endif

#include <signal.h>
#include <sys/auxv.h>
#include <sys/iomsg.h>
#include <sys/procmsg.h>
#include <sys/pathmsg.h>
#include <sys/memmsg.h>
#include <sys/sysmsg.h>
#include "smpswitch.h"


#define PROC_INIT_PRIORITY		10	// priority to start init thread at

union proc_msg_union {
	uint16_t						type;
	proc_spawn_t					spawn;
	proc_spawn_args_t				spawn_args;
	proc_spawn_debug_t				spawn_debug;
	proc_spawn_fd_t					spawn_fd;
	proc_spawn_done_t				spawn_done;
	proc_spawn_dir_t				spawn_dir;
	proc_getsetid_t					getsetid;
	proc_setpgid_t					setpgid;
	proc_wait_t						wait;
	proc_fork_t						fork;
	proc_umask_t					umask;
	proc_guardian_t					guardian;
	proc_resource_usage_t			resource_usage;
	proc_resource_getlimit_t		resource_getlimit;
	proc_resource_setlimit_t		resource_setlimit;
	mem_map_t						mmap;
	mem_ctrl_t						mctrl;
	mem_info_t						minfo;
	mem_offset_t					moffset;
	sys_conf_t						conf;
	sys_cmd_t						syscmd;
	io_open_t						open;
	io_dup_t						dup;
	io_read_t						read;
	io_write_t						write;
	io_lseek_t						lseek;
	io_stat_t						stat;
	struct _pulse					pulse;
	char							filler[sizeof(struct _io_connect_link_reply) + sizeof(struct _io_connect_entry) * SYMLOOP_MAX + PATH_MAX + 1];
};

#define PROC_LF_LAZYSTACK			0x80000000

struct loader_startup {
	uintptr_t						eip;
	uintptr_t						esp;
	uintptr_t						stackaddr;		// prefered address of stack
	uintptr_t						stackalloc;		// minimum number of bytes needed in stack
	uintptr_t						stacksize;		// maximum stack size
	unsigned						flags;
	auxv_t							aux[AT_NUM + 1];
};

enum lc_state {
	LC_FREE,
	LC_SPAWN,
	LC_FORK,
	LC_EXEC_SWAP,
	LC_TERM,
	LC_STATE_MASK = 0x3f,
	LC_CROSS_ENDIAN = 0x40,
	LC_TERMER_FINISHED = 0x80
};

// Done as a define since C doesn't have inheritance. Maybe C++ does
// have some uses.... Naaaah!
#define LOADER_CONTEXT_FIELDS					\
	struct loader_context			*next;		\
	size_t							size;		\
	iov_t							iov[10];	\
	struct process_entry			*process;	\
	int								rcvid;		\
	sigset_t						mask;		\
	siginfo_t						info;		\
	pid_t							pid;		\
	pid_t							ppid;		\
	int								pnode;		\
	int								tid;		\
	unsigned						flags;		\
	unsigned						msgsize;	\
	int								state;		\
	struct loader_startup			start;		\
	unsigned						remote_off; \
	int								fault_errno;

struct loader_context_prefix {
	LOADER_CONTEXT_FIELDS
};

#define LOADER_STACK_SIZE 	(3*__PAGESIZE 						\
						- sizeof(struct loader_context_prefix)	\
						- sizeof(union proc_msg_union))

struct loader_context {
	LOADER_CONTEXT_FIELDS
	uint32_t					stack[LOADER_STACK_SIZE/sizeof(uint32_t)];

	// The 'msg' field has to go on the end so that we can expand
	// it out for long spawns.
	union proc_msg_union			msg;
};

// Make a unique inode number
#define PID_TO_INO(pid, subtype)	((pid) ^ (subtype ? UINT_MAX : INT_MAX))

#endif

/* __SRCVERSION("struct.h $Rev: 153052 $"); */
