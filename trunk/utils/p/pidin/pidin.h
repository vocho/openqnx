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





#ifndef __PIDIN_HEADER__
#define __PIDIN_HEADER__

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/neutrino.h>
#include <sys/kercalls.h>
#include <sys/debug.h>
#include <sys/procfs.h>

/*
 * flags for dspinfo 
 */
#define DONT_RECURSE	0x1
#define DO_THREADS		0x2

struct shared_info;

extern struct format {
	int				width;
	char		   *title;
	int				(*print) (FILE * fp, int pid, int *tid, struct format * fmt, int fd, struct shared_info * info);
#define TITLE_LEFT_JUSTIFIED	0x1
#define DATA_CENTERED			0x2
#define MULTI_LINE				0x4	/* No title required for this field, it spans a line */
#define DATA_FILL				0x8
#define PRESERVE_RIGHT			0x10
#define DATA_LEFT_JUSTIFIED		0x20
#define PROCESS_ONLY			0x40
#define ZOMBIE_INVALID 			0x80 /* this field is invalid if its a zombie */
#define ZI						ZOMBIE_INVALID
#define NA						0x100 /* print (n/a) if not available */
#define MEMORY_INFO				0x200
#define THREAD_UNIQUE			0x400
#define TU						THREAD_UNIQUE
#define TITLE_RIGHT_JUSTIFIED	0x800
	unsigned int	flags;
	char			letter;
} formats[256];

#define NUM_KCALLS __KER_BAD
extern const char *const kc_names[NUM_KCALLS + 1];
#define NUM_THREAD_STATE 16
extern const int num_thread_states;
extern const char *const thread_state[];
extern const char *const spaces;
extern const char *const zeros;
extern const char *const na;
extern const char* regnames[];
extern const int nregnames;

typedef	struct {
	procfs_debuginfo   info;
	char               buff[_POSIX_PATH_MAX];
} meminfo_mapdebug_t;
typedef struct {
	meminfo_mapdebug_t mapdebug;
	ino_t              ino;
	int                ok;
} meminfo_mapdata_t;
typedef struct {
	int                tid;
	int                cache_len;
	procfs_mapinfo    *mapinfo;
	meminfo_mapdata_t *mapdata;
} meminfo_t;

typedef struct {
	int totalprocs;
	int totalthreads;
} proc_info_t;

struct process_entry_
{
	procfs_info         info;
	int                 n_threads;
};
struct thread_entry_
{
	procfs_status       status;
};

struct shared_info {
	/*
	 * as infomation is gathered it is added here 
	 */
	procfs_status	*status;
	procfs_debuginfo *name;
	procfs_mapinfo	*mem;
	meminfo_t       *meminfo;
	int				num_mem;
#define SEPARATE_MEMORY 0x1
#define NO_MEMINFO	    0x2
	int flags;
	procfs_info		*info;
	struct _thread_local_storage *tls;
	uint64_t	text;
	uint64_t	data;
	uint64_t	stack;
	uint64_t	vstack;
	struct memobjects {
		char *name;
		uint64_t text_size;
		uint64_t data_size;
		uint64_t vaddr;
		uint64_t offset;
		uint64_t flags;
	} *memobjects;
	unsigned int	num_memobjects;
	unsigned int	next_memobject;
	procfs_irq		*irqs; 
	int				num_irqs;
	procfs_timer	*timers;
	int				num_timers;
	procfs_channel	*channels;
	int				num_channels;
	size_t			gprs_size;
	procfs_greg		*gprs;
	struct coid_info {
		int		fd;
		int		pid;
		int 		ioflag;
		off_t		offset;
		size_t		size;
		char		*name;
	} *coids;
	int			num_coids;	
};
int				fill_status(int expectwarn, struct shared_info *i, int *tid, int fd);
int				fill_name(struct shared_info *i, int fd);
int				fill_mem(struct shared_info *i, int fd);
int				fill_info(struct shared_info *i, int fd);
int				fill_timers(struct shared_info *i, int fd);
int				fill_irqs(struct shared_info *i, int fd);
int				fill_channels(struct shared_info *i, int fd);
void			free_meminfo(meminfo_t **m);

int				fwoutput(FILE * fp, int len, const char *str);
int				format_data_string(FILE * fp, struct format *fmt, const char *str);
int				format_title_string(FILE * fp, struct format *fmt, const char *str);
int				format_data_int(FILE * fp, struct format *fmt, int d);

extern int		Channels(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Registers(FILE* fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Timers(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Coids(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Interrupt(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Arguments(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		WhereBlocked(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Environment(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		DebugFlags(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		ProcessFlags(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		ThreadFlags(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		PidTidField(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		State(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		KerCall(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Memory(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		MemoryPhys(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Name(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
#ifndef NO_BACKTRACE_LIB
extern int		ThreadBacktrace(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
#endif
extern int		ThreadName(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		pid(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		tid(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		codesize(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		datasize(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		stacksize(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		long_name(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Pgrp(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		parentpid(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Sibling(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Child(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Sid(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Uid(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Gid(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		EUid(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		EGid(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		SUid(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		SGid(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		SigIgnore(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		SigPending(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		ProcessUtime(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *i);
extern int		ProcessStime(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *i);
extern int		ProcessCutime(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *i);
extern int		ProcessCstime(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *i);
extern int		ThreadSUtime(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *i);
extern int		ProcessStartTime(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *i);
extern int		ThreadStartTime(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *i);
extern int		NumThreads(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		priority(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		MemObjectCode(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		MemObjectData(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		MemObjectMapAddr(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		MemObjectFlags(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		MemObjectOffset(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		MemObjectOffsetPhys(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		LastCPU(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		ExtSched(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);
extern int		Rmasks(FILE * fp, int pid, int *tid, struct format *fmt, int fd, struct shared_info *info);

extern uint64_t	normalize_data_size(uint64_t size, char **sym);

extern void		error_exit(int printmsg, const char *fmt,...);
extern void		warning_exit(int printmsg, int expectwarn, const char *fmt,...);

extern int					dspsyspage(char *enables);
extern struct syspage_entry	*load_syspage(int fd, int full);
extern uint64_t				get_total_mem(struct syspage_entry *ptr);

#endif
