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
 *  sys/resmgr.h
 *

 */
#ifndef __RESMGR_COMPAT_H_INCLUDED
#define __RESMGR_COMPAT_H_INCLUDED

#ifndef _PTHREAD_H_INCLUDED
 #include <pthread.h>
#endif

#ifndef __IOMSG_H_INCLUDED
 #include <sys/iomsg.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if defined(__IOVEC_T)
typedef __IOVEC_T	iov_t;
#undef __IOVEC_T
#endif

#include <_pack64.h>

typedef struct _pulse io_pulse_t;

struct _dispatch;
typedef struct _dispatch dispatch_t;

typedef union _resmgr_iomsgs {
	unsigned short				type;

	io_pulse_t					pulse;

	struct _io_connect			connect;
	io_open_t					open;
	io_unlink_t					unlink;
	io_rename_t					rename;
	io_mknod_t					mknod;
	io_readlink_t				readlink;
	io_link_t					link;
	io_mount_t					mount;

	struct _io_combine			combine;
	io_close_t					close;
	io_write_t					write;
	io_read_t					read;
	io_stat_t					stat;
	io_notify_t					notify;
	io_devctl_t					devctl;
	io_pathconf_t				pathconf;
	io_lseek_t					lseek;
	io_chmod_t					chmod;
	io_chown_t					chown;
	io_utime_t					utime;
	io_openfd_t					openfd;
	io_fdinfo_t					fdinfo;
	io_lock_t					lock;
	io_space_t					space;
	io_shutdown_t				shutdown;
	io_msg_t					msg;
	io_mmap_t					mmap;
	io_dup_t					dup;
	io_sync_t					sync;
} resmgr_iomsgs_t;

typedef struct _resmgr_context {
	int							rcvid;
	struct _msg_info			info;
	resmgr_iomsgs_t				*msg;
	struct _resmgr_ctrl			*ctrl;
	int							id;
	unsigned					tid;
	int							reserved;
	int							status;
	int							offset;
	int							size;
	iov_t						iov[1];
} resmgr_context_t;

typedef struct _resmgr_ctrl {
	int							(*other_func)(resmgr_context_t *ctp, void *msg);
/*	int							(*other_pulse)(resmgr_context_t *ctp, io_pulse_t *pulse); */
/*	int							(*other_connect)(resmgr_context_t *ctp, void *msg, void *handle, void *extra); */
/*	int							(*other_io)(resmgr_context_t *ctp, void *msg, void *ocb); */
	unsigned					nparts_max;
	unsigned					msg_max_size;
	unsigned					thread_stack_size;
	unsigned short				lo_water;
	unsigned short				increment;
	unsigned short				hi_water;
	unsigned short				maximum;
	unsigned					reserved[10];
	int							chid;
	pthread_mutex_t				mutex;
	unsigned					created;
	unsigned					waiting;
} resmgr_ctrl_t;

#ifndef RESMGR_HANDLE_T
#define RESMGR_HANDLE_T			void
#endif

typedef int(*_resmgr_func_t)(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, void *ocb);

typedef struct _resmgr_connect_funcs {
	unsigned	nfuncs;
	int			(*open)(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra);
	int			(*unlink)(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
	int			(*rename)(resmgr_context_t *ctp, io_rename_t *msg, RESMGR_HANDLE_T *handle, io_rename_extra_t *extra);
	int			(*mknod)(resmgr_context_t *ctp, io_mknod_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
	int			(*readlink)(resmgr_context_t *ctp, io_readlink_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
	int			(*link)(resmgr_context_t *ctp, io_link_t *msg, RESMGR_HANDLE_T *handle, io_link_extra_t *extra);
	int			(*unblock)(resmgr_context_t *ctp, io_pulse_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
	int			(*mount)(resmgr_context_t *ctp, io_mount_t *msg, RESMGR_HANDLE_T *handle, io_mount_extra_t *extra);
} resmgr_connect_funcs_t;
#define _RESMGR_CONNECT_NFUNCS	((sizeof(resmgr_connect_funcs_t)-sizeof(unsigned))/sizeof(void *))
	
#ifndef RESMGR_OCB_T
#define RESMGR_OCB_T			void
#endif

struct _io_umount {
        _Uint16t                                        type;
        _Uint16t                                        combine_len;
		_Uint32t										flags;
		_Uint32t										zero[2];
};

typedef union {
        struct _io_umount                        i;
} io_umount_t;

typedef struct _resmgr_io_funcs {
	unsigned	nfuncs;
	int			(*read)(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
	int			(*write)(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
	int			(*close_ocb)(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb);
	int			(*stat)(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb);
	int			(*notify)(resmgr_context_t *ctp, io_notify_t *msg, RESMGR_OCB_T *ocb);
	int			(*devctl)(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
	int			(*unblock)(resmgr_context_t *ctp, io_pulse_t *msg, RESMGR_OCB_T *ocb);
	int			(*pathconf)(resmgr_context_t *ctp, io_pathconf_t *msg, RESMGR_OCB_T *ocb);
	int			(*lseek)(resmgr_context_t *ctp, io_lseek_t *msg, RESMGR_OCB_T *ocb);
	int			(*chmod)(resmgr_context_t *ctp, io_chmod_t *msg, RESMGR_OCB_T *ocb);
	int			(*chown)(resmgr_context_t *ctp, io_chown_t *msg, RESMGR_OCB_T *ocb);
	int			(*utime)(resmgr_context_t *ctp, io_utime_t *msg, RESMGR_OCB_T *ocb);
	int			(*openfd)(resmgr_context_t *ctp, io_openfd_t *msg, RESMGR_OCB_T *ocb);
	int			(*fdinfo)(resmgr_context_t *ctp, io_fdinfo_t *msg, RESMGR_OCB_T *ocb);
	int			(*lock)(resmgr_context_t *ctp, io_lock_t *msg, RESMGR_OCB_T *ocb);
	int			(*space)(resmgr_context_t *ctp, io_space_t *msg, RESMGR_OCB_T *ocb);
	int			(*shutdown)(resmgr_context_t *ctp, io_shutdown_t *msg, RESMGR_OCB_T *ocb);
	int			(*mmap)(resmgr_context_t *ctp, io_mmap_t *msg, RESMGR_OCB_T *ocb);
	int			(*msg)(resmgr_context_t *ctp, io_msg_t *msg, RESMGR_OCB_T *ocb);
	int			(*umount)(resmgr_context_t *ctp, io_umount_t *msg, RESMGR_OCB_T *ocb);
	int			(*dup)(resmgr_context_t *ctp, io_dup_t *msg, RESMGR_OCB_T *ocb);
	int			(*close_dup)(resmgr_context_t *ctp, io_close_t *msg, RESMGR_OCB_T *ocb);
	int			(*lock_ocb)(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb);
	int			(*unlock_ocb)(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb);
	int			(*sync)(resmgr_context_t *ctp, io_sync_t *msg, RESMGR_OCB_T *ocb);
} resmgr_io_funcs_t;
#define _RESMGR_IO_NFUNCS		((sizeof(resmgr_io_funcs_t)-sizeof(unsigned))/sizeof(void *))

#define _RESMGR_NOREPLY			((~0u ^ (~0u >> 1)) | 0)	/* Do not reply to sender (could use _RESMGR_AGAIN later) */
#define _RESMGR_DEFAULT			((~0u ^ (~0u >> 1)) | 1)	/* Default action (usually return ENOSYS) */
#define _RESMGR_NPARTS(_num)	(-(_num))				/* Reply with this many parts from ctp->iov */
#define _RESMGR_ERRNO(_err)		(_err)					/* Reply with a short status containing an errno (can be EOK) */
#define _RESMGR_PTR(_h,_p,_n)	(SETIOV((_h)->iov+0,(_p),(_n)),_RESMGR_NPARTS(1))
#define _RESMGR_STATUS(_c,_s)	((_c)->status = (_s))

enum _resmgr_handle_type {
	_RESMGR_HANDLE_FIND,
	_RESMGR_HANDLE_SET,
	_RESMGR_HANDLE_REMOVE,
	_RESMGR_HANDLE_DISCONNECT,
	_RESMGR_HANDLE_UNLOCK,
	_RESMGR_HANDLE_LOCK	= (int)(~0u ^ (~0u >> 1)),
	_RESMGR_HANDLE_FIND_LOCK = _RESMGR_HANDLE_FIND | _RESMGR_HANDLE_LOCK,
	_RESMGR_HANDLE_DISCONNECT_LOCK = _RESMGR_HANDLE_DISCONNECT | _RESMGR_HANDLE_LOCK
};

#define _RESMGR_PULSE_ALLOC			~0			/* passed as code to qnx_resmgr_pulse() */

struct _resmgr_handle_entry {
	struct _resmgr_handle_entry		*next;
	void							*handle;
	int								coid;
};

struct _resmgr_handle_list {
	struct _resmgr_handle_entry		*list;
	int								waiting;
	pid_t							pid;
	_Uint32t						nd;
};

struct _resmgr_handle_table {
	struct _resmgr_handle_list		*vector;	/* pointer to an array of lists */
	struct _resmgr_handle_entry		*free_list;
	pthread_mutex_t					mutex;
	pthread_cond_t					cond;
	unsigned						nentries;
	unsigned						total;
	unsigned						free;
	unsigned						min;
	unsigned						reserved[6];
};

extern struct _resmgr_handle_table	_resmgr_io_table;

/*
 * flags for resmgr_pathname_attach()
 */
#define _RESMGR_FLAG_BEFORE		0x0001	/* Force path to be resolved before others at the same mountpoint. */
#define _RESMGR_FLAG_AFTER		0x0002	/* Force path to be resolved after others at the same mountpoint. */
#define _RESMGR_FLAG_OPAQUE		0x0004	/* Don't resolve to mountpoints with shorter pathname matches. */
#define _RESMGR_FLAG_FTYPEONLY  0x0008  /* Only attach at "/", and only match ftype */
#define _RESMGR_FLAG_FTYPEALL   0x0010  /* Matching all ftypes (for redirecting servers) */
#define _RESMGR_FLAG_DIR		0x0100	/* Allow resolving of longer pathnames. */
#define _RESMGR_FLAG_SELF		0x0200	/* Allow resolving names to itself. */
#define _RESMGR_FLAG_MASK		0x031f	/* Flags allowed by resmgr_pathname_attach() */

/*
 * flags for resmgr_pathname_detach()
 */
#define _RESMGR_DETACH_ALL		0x0000	/* Detach the name from the namespace and invalidate all open bindings. */
#define _RESMGR_DETACH_PATHNAME	0x0001	/* Only detach the name from the namespace */

/*
 * flags for resmgr_pathname(). Must match _FDINFO_FLAG_*.
 */
#define _RESMGR_PATHNAME_LOCALPATH	0x0001	/* Used to return smaller path for displaying */

/*
 * If using thread pool functions, default to passing resmgr_context_t
 */
#ifndef THREAD_POOL_PARAM_T
 #define THREAD_POOL_PARAM_T	resmgr_context_t
#endif

#ifndef THREAD_POOL_HANDLE_T
 #define THREAD_POOL_HANDLE_T	dispatch_t
#endif

#define resmgr_pathname_attach 	_resmgr_pathname_attach
#define resmgr_pathname_detach 	_resmgr_pathname_detach
#define resmgr_start			_resmgr_start
#define resmgr_pulse_attach		_resmgr_pulse_attach
#define resmgr_pulse_detach		_resmgr_pulse_detach

__BEGIN_DECLS

struct binding;

extern int _resmgr_pathname_attach(const char *path, int chid, enum _file_type file_type, unsigned flags,
		const resmgr_connect_funcs_t *connect_funcs, const resmgr_io_funcs_t *io_funcs, RESMGR_HANDLE_T *handle);
extern int _resmgr_pathname_detach(int id, unsigned flags);
extern int resmgr_pathname(int id, unsigned flags, char *path, int maxbuf);
extern int resmgr_devino(int id, dev_t *pdevno, ino64_t *pino);
extern int resmgr_open_bind(resmgr_context_t *ctp, RESMGR_OCB_T *ocb, const resmgr_io_funcs_t *io_funcs);
extern int _resmgr_start(resmgr_ctrl_t *ctrl);
extern int resmgr_msg_again(resmgr_context_t *ctp, int rcvid);
extern int _resmgr_pulse_attach(int code, void (*func)(resmgr_context_t *ctp, int code, union sigval value, void *handle), void *handle);
extern int _resmgr_pulse_detach(int code);

extern int resmgr_msgwrite(resmgr_context_t *ctp, const void *msg, int size, int offset);
extern int resmgr_msgwritev(resmgr_context_t *ctp, const struct iovec *smsg, int sparts, int offset);
extern int resmgr_msgread(resmgr_context_t *ctp, void *msg, int size, int offset);
extern int resmgr_msgreadv(resmgr_context_t *ctp, struct iovec *rmsg, int rparts, int offset);

extern void *_resmgr_ocb(resmgr_context_t *ctp, struct _msg_info *info);
extern const resmgr_io_funcs_t *_resmgr_iofuncs(resmgr_context_t *ctp, struct _msg_info *info);
extern int _resmgr_handle_grow(unsigned min);
extern void *_resmgr_handle(struct _msg_info *rep, void *handle, enum _resmgr_handle_type type);

__END_DECLS

#include <_packpop.h>

#define _RESMGR_FLAG_EXCL 0		/* Temporary, this flag does not exist any more */

#endif

/* __SRCVERSION("resmgr_compat.h $Rev: 153052 $"); */
