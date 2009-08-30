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
 * Internal definitions for the dispatch interface
 */

#define _DISPATCH_BLOCK_RECEIVE		1
#define _DISPATCH_BLOCK_SIGWAIT 	2
#define _DISPATCH_BLOCK_TIMEOUT		3

#define _DPP(dpp)					((dispatch_t *)dpp)
/*
 * Special-case flags
 */

#define _DISPATCH_ONLY_RESMGR		0x00000001
#define _DISPATCH_ONLY_SELECT		0x00000002
#define _DISPATCH_TIMEOUT			0x00000004

#define _VEC_VALID					0x80000000

/*
 * Select interface
 */
#define SEL_REARM_ALL (-1)

#define _SELECT_ARMED				0x00010000
#define _SELECT_INVALID				0x00020000
#define _SELECT_EVENT				0x00040000
#define _SELECT_FLAG_NOREARM			0x00080000
#define _SELECT_ARM_FIRST			0x00100000
#define _SELECT_READ_EVENT			(SELECT_FLAG_READ >> 4)
#define _SELECT_WRITE_EVENT			(SELECT_FLAG_WRITE >> 4)
#define _SELECT_EXCEPT_EVENT		(SELECT_FLAG_EXCEPT >> 4)
#define _SELECT_EVENT_MASK			(_NOTIFY_COND_MASK >> 4)
#define _SELECT_SN_MASK				0x00000fff
#define _SELECT_SRVEXCEPT			0x08000000

#define _SELECT_SIGEV(index,sn) 	(index | (sn & _SELECT_SN_MASK) << 16)
#define _SELECT_SIGEV_SN(sigev) 	(_SELECT_SN_MASK & (sigev >> 16))
#define _SELECT_SIGEV_INDEX(sigev) 	(sigev & 0xffff)

typedef struct {
	void						*select_vec;
	int							num_elements;
	int							num_entries;
	unsigned					context_size;
	unsigned					msg_max_size;
	pthread_mutex_t				mutex;
	int							coid;
	int							code;
	unsigned					sernum;
	unsigned					flags;
	dispatch_context_t 			*(*rearm_func)(dispatch_context_t *);
} _select_control;

typedef struct _select_vec {
	unsigned						flags;
	int								fd;
	int						(*func)(select_context_t *ctp, int fd, unsigned flags, void *handle);
	void 							*handle;
} select_vec_t;

void _select_disarm(dispatch_t *dpp, int fd);
int _select_msg_handler(message_context_t *ctp, int code, unsigned flags, void *handle);
dispatch_context_t *_select_rearm_all(dispatch_context_t *ctp);
dispatch_context_t *_select_rearm_how(dispatch_context_t *ctp, int fd);
int _select_query(select_context_t *ctp, int *fd, unsigned *flags,
		int (**func)(select_context_t *ctp, int fd, unsigned flags, void *handle),
		void **handle, unsigned clear_event);

/*
 * Resmgr interface
 */

#define _RESMGR_CROSS_ENDIAN		RESMGR_FLAG_CROSS_ENDIAN

typedef struct {
	int							(*other_func)(resmgr_context_t *ctp, void *msg);
	unsigned					nparts_max;
	unsigned					msg_max_size;
	unsigned					context_size;
	pthread_mutex_t				mutex;
	unsigned					flags;
} _resmgr_control;

int _resmgr_msg_handler(message_context_t *ctp, int code, unsigned flags, void *handle);
int _resmgr_default_handler(message_context_t *ctp, int code, unsigned flags, void *handle);

/*
 * Message interface
 */

#define _MESSAGE_RESMGR_ENTRY		MSG_FLAG_TYPE_RESMGR
#define _MESSAGE_SELECT_ENTRY		MSG_FLAG_TYPE_SELECT
#define _MESSAGE_PULSE_ENTRY		MSG_FLAG_TYPE_PULSE
#define _MESSAGE_DEFAULT_ENTRY		MSG_FLAG_DEFAULT_FUNC
#define _MESSAGE_CROSS_ENDIAN		MSG_FLAG_CROSS_ENDIAN

typedef struct {
	void 						*message_vec;
	int							num_elements;
	int							num_entries;
	unsigned					context_size;
	unsigned					msg_max_size;
	unsigned					nparts_max;
	pthread_mutex_t				mutex;
	unsigned					reserved; // flags;
} _message_control;

typedef struct _message_vec {
	unsigned						flags;
	short							lo;		/* Stay unsigned, see message.c */
	short							high;
	int						(*func)(message_context_t *ctp, int code, unsigned flags, void *handle);
	void 							*handle;
} message_vec_t;

int _message_handler(dispatch_context_t *ctp);
void _message_unblock(dispatch_context_t *ctp);

/*
 * Sigwait interface
 */

#define _SIGWAIT_SIGNAL_ENTRY		0x00000001
#define _SIGWAIT_SELECT_ENTRY		0x00000002

typedef struct {
	void						*sigwait_vec;
	int							num_elements;
	int							num_entries;
	unsigned					context_size;
	pthread_mutex_t				mutex;
} _sigwait_control;

typedef struct _sigwait_vec {
	unsigned						flags;
	unsigned						signo;
	int						(*func)(sigwait_context_t *ctp, int signo, int flags, void *handle);
	void 							*handle;
} sigwait_vec_t;

int _sigwait_handler(dispatch_context_t *ctp);

/*
 * Vector "helper" functions
 */

void *_dispatch_vector_find(void *vec, int num);
void *_dispatch_vector_grow(void *vec, int new_num);

/*
 * Dispatch internal structures
 */
int _dispatch_attach(dispatch_t *dpp, void *ctrl, unsigned type);
int _dispatch_set_contextsize(dispatch_t *dpp, unsigned type);

#define _DISPATCH_CHANNEL_COIDDEATH		0x40000000
#define _DISPATCH_CONTEXT_ALLOCED		0x80000000

struct _dispatch {
	//enum dispatch_type		type;
	int						(*other_func)(resmgr_context_t *ctp, void *msg);
	unsigned				nparts_max;
	unsigned				msg_max_size;
	int						block_type;
	struct timespec			timeout;	
	unsigned int			flags;
	int						chid;
	unsigned int			context_size;
	_resmgr_control			*resmgr_ctrl;
	_message_control		*message_ctrl;
	_select_control			*select_ctrl;
	_sigwait_control		*sigwait_ctrl;
};

struct _pool_properties {
	pthread_mutex_t inline_lock;
	pthread_cond_t pool_cond;
	unsigned newthreads;
	unsigned reserved_threads;
	unsigned control_threads;
};

/* __SRCVERSION("dispatch.h $Rev: 167031 $"); */
