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
 *  sys/dispatch.h
 *

 */
#ifndef __DISPATCH_H_INCLUDED
#define __DISPATCH_H_INCLUDED

#ifndef _PTHREAD_H_INCLUDED
 #include <pthread.h>
#endif

#ifndef _SIGNAL_H_INCLUDED
 #include <signal.h>
#endif

#ifndef _TIME_H_INCLUDED
 #include <time.h>
#endif

#ifndef __RESMGR_H_INCLUDED
 #include <sys/resmgr.h>
#endif

#define SIGDISPATCH SIGPHOTON

/*
 * Select dispatch functions
 */
#define SELECT_FLAG_READ		_NOTIFY_COND_INPUT
#define SELECT_FLAG_WRITE		_NOTIFY_COND_OUTPUT
#define SELECT_FLAG_EXCEPT		_NOTIFY_COND_OBAND
#define SELECT_FLAG_REARM		0x00000001	/* Default (redundant) */
#define SELECT_FLAG_SRVEXCEPT	0x00000002
#define SELECT_FLAG_NOREARM		0x00000004	/* Higher precedence than SELECT_FLAG_REARM */

__BEGIN_DECLS

typedef struct _select_context	select_context_t;

typedef struct _select_attr {
	unsigned					flags;
	unsigned					reserved[3];
} select_attr_t;

struct _select_context {
	int							rcvid;
	union {
		struct _msg_info		msginfo;
		siginfo_t				siginfo;
	} info;
	resmgr_iomsgs_t				*msg;
	void						*dpp;
	int							fd;
	unsigned					tid;	
	unsigned					reserved;
	int							flags;
	int							reserved2[2];
	iov_t						iov[1];
};

int select_attach(void *dpp, select_attr_t *attr, int fd, unsigned flags,
		int (*func)(select_context_t *ctp, int fd, unsigned flags, void *handle),
		void *handle);
int select_query(select_context_t *ctp, int *fd, unsigned *flags,
		int (**func)(select_context_t *ctp, int fd, unsigned flags, void *handle),
		void **handle);
int select_detach(void *dpp, int fd);

/*
 * Resmgr dispatch functions
 */
#define RESMGR_FLAG_NO_DEFAULT_FUNC		0x00000001
#define RESMGR_FLAG_ATTACH_LOCAL		0x00000002
#define RESMGR_FLAG_ATTACH_OTHERFUNC	0x00000004
#define RESMGR_FLAG_CROSS_ENDIAN		0x00000008

typedef struct _resmgr_attr {
	unsigned					flags;
	unsigned					nparts_max;
	unsigned					msg_max_size;
	int							(*other_func)(resmgr_context_t *, void *msg);
	unsigned					reserved[4];	
} resmgr_attr_t;

int resmgr_attach(dispatch_t *dpp, resmgr_attr_t *attr, const char *path, 
		enum _file_type file_type, unsigned flags, 
		const resmgr_connect_funcs_t *connect_funcs, 
		const resmgr_io_funcs_t *io_funcs, RESMGR_HANDLE_T *handle);
resmgr_context_t * resmgr_context_alloc(dispatch_t *dpp);
resmgr_context_t * resmgr_block(resmgr_context_t *ctp);
void resmgr_unblock(resmgr_context_t *ctp);
int resmgr_handler(resmgr_context_t *ctp);
void resmgr_context_free(resmgr_context_t *ctp);
int resmgr_detach(dispatch_t *dpp, int id, unsigned flags);

/*
 * Name dispatch functions
 */
#define NAME_FLAG_ATTACH_GLOBAL		0x00000002		/* Attach a global name */
#define NAME_FLAG_CROSS_ENDIAN		RESMGR_FLAG_CROSS_ENDIAN

#define NAME_FLAG_DETACH_SAVEDPP	0x00000001		/* Dispatch ptr saved */

typedef struct _name_attach {
    dispatch_t *dpp;
    int         chid;
    int         mntid;
	int			zero[2];
} name_attach_t;

name_attach_t *name_attach(dispatch_t *dpp, const char *path, unsigned flags);
int name_detach(name_attach_t *attach, unsigned flags);

int name_open(const char *name, int flags);
int name_close(int fd);

/*
 * Message dispatch functions
 */
#define MSG_FLAG_TYPE_PULSE		0x00000001
#define MSG_FLAG_ALLOC_PULSE	0x00000002
#define MSG_FLAG_TYPE_SELECT	0x00000004
#define MSG_FLAG_TYPE_RESMGR	0x00000008
#define MSG_FLAG_DEFAULT_FUNC	0x00000100
#define MSG_FLAG_SIDE_CHANNEL	0x00000200
#define MSG_FLAG_CROSS_ENDIAN	0x00000400

typedef struct _resmgr_context	message_context_t;

typedef struct _message_attr {
	unsigned					flags;
	unsigned					nparts_max;
	unsigned					msg_max_size;
	unsigned					reserved[5];	
} message_attr_t;

int message_attach(dispatch_t *dpp, message_attr_t *attr, int low, int high,
		int (*func)(message_context_t *ctp, int code, unsigned flags, void *handle),
		void *handle);
int pulse_attach(dispatch_t *dpp, int flags, int code,
		int (*func)(message_context_t *ctp, int code, unsigned flags, void *handle),
		void *handle);
int message_detach(dispatch_t *dpp, int low, int high, int flags);
int pulse_detach(dispatch_t *dpp, int code, int flags);
int message_connect(dispatch_t *dpp, int flags);

/*
 * Sigwait dispatch functions
 */
#define SIGWAIT_FLAG_ALLOC_SIG		0x00000001
#define SIGWAIT_FLAG_TYPE_SELECT	0x00000002

typedef struct _sigwait_context	sigwait_context_t;

typedef struct _sigwait_attr {
	unsigned					flags;
	unsigned					reserved[3];	
} sigwait_attr_t;

struct _sigwait_context {
	int							signo;
	union {
		struct _msg_info		msginfo;
		siginfo_t				siginfo;
	} info;
	resmgr_iomsgs_t				*msg;
	void 						*dpp;
	int							status;
	unsigned					tid;	
	sigset_t					set;
	int							reserved2[2];
	iov_t						iov[1];
};

int	sigwait_attach(dispatch_t *dpp, sigwait_attr_t *attr, int signo,
		int (*func)(sigwait_context_t *ctp, int signo, unsigned flags, void *handle),
		void *handle);
int sigwait_detach(dispatch_t *dpp, int signo, int flags);

/*
 * General dispatch functions
 */

#define DISPATCH_FLAG_NOLOCK		0x00000001

enum dispatch_type {
	DISPATCH_ERROR = -1,
	DISPATCH_NONE = 0,
	DISPATCH_TIMEOUT,
	DISPATCH_RESMGR,
	DISPATCH_MESSAGE,
	DISPATCH_SELECT,
	DISPATCH_SIGWAIT,
	DISPATCH_MAX
};

typedef union _dispatch_context {
	resmgr_context_t		resmgr_context;					
	message_context_t		message_context;					
	select_context_t		select_context;
	sigwait_context_t		sigwait_context;
} dispatch_context_t;

dispatch_t				*dispatch_create(void);
dispatch_t				*dispatch_create_channel(int chid, unsigned reserved);
dispatch_context_t 		*dispatch_block(dispatch_context_t *ctp);
void					dispatch_unblock(dispatch_context_t *ctp);
int 					dispatch_handler(dispatch_context_t *ctp);
int 					dispatch_timeout(dispatch_t *dpp, struct timespec *reltime);
dispatch_context_t 		*dispatch_context_alloc(dispatch_t *dpp);
void					dispatch_context_free(dispatch_context_t *ctp);
int 					dispatch_destroy(dispatch_t *dpp);

dispatch_context_t *select_rearm(dispatch_context_t *ctp, int fd);


/*
 * Thread pool functions
 */

#define	POOL_FLAG_EXIT_SELF		0x00000001
#define	POOL_FLAG_USE_SELF		0x00000002
#define	POOL_FLAG_EXITING		0x00000004
#define	POOL_FLAG_CHANGING		0x00000008
#define POOL_FLAG_RESERVE     		0x00000010
#define POOL_FLAG_CONTROL     	0x00000020

typedef struct _thread_pool		thread_pool_t;

#ifndef THREAD_POOL_PARAM_T
 #define THREAD_POOL_PARAM_T	void
#endif

#ifndef THREAD_POOL_HANDLE_T
 #define THREAD_POOL_HANDLE_T	dispatch_t
#endif

typedef struct _thread_pool_attr {
	THREAD_POOL_HANDLE_T	*handle;
	THREAD_POOL_PARAM_T		*(*block_func)(THREAD_POOL_PARAM_T *ctp);
	void					(*unblock_func)(THREAD_POOL_PARAM_T *ctp);
	int						(*handler_func)(THREAD_POOL_PARAM_T *ctp);
	THREAD_POOL_PARAM_T		*(*context_alloc)(THREAD_POOL_HANDLE_T *handle);
	void					(*context_free)(THREAD_POOL_PARAM_T *ctp);
	pthread_attr_t			*attr;
	unsigned short			lo_water;
	unsigned short			increment;
	unsigned short			hi_water;
	unsigned short			maximum;
	unsigned				reserved[8];
} thread_pool_attr_t;

struct _thread_pool	{
	thread_pool_attr_t	pool_attr;
	unsigned			created;
	unsigned			waiting;
	unsigned			flags;
	void          *props;
	unsigned			reserved[2];
};

thread_pool_t 	*thread_pool_create(thread_pool_attr_t *attr, unsigned flags);
int 			thread_pool_start(void *pool);
int 			thread_pool_destroy(thread_pool_t *pool);

#define THREAD_POOL_CONTROL_HIWATER		0x0001
#define THREAD_POOL_CONTROL_LOWATER		0x0002
#define THREAD_POOL_CONTROL_MAXIMUM		0x0004
#define THREAD_POOL_CONTROL_INCREMENT	0x0008
#define THREAD_POOL_CONTROL_NONBLOCK	0x4000
int				thread_pool_control(thread_pool_t *__pool, thread_pool_attr_t *__attr, 
									_Uint16t __lower, _Uint16t __upper, unsigned __flags);
int				thread_pool_limits(thread_pool_t *__pool, int __lowater, int __hiwater, 
							         int __maximum, int __increment, unsigned __flags);

extern thread_pool_attr_t *thread_pool_attr_default;

__END_DECLS

#endif

/* __SRCVERSION("dispatch.h $Rev: 171666 $"); */
