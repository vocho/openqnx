/*
 *  mig4nto_procmsg.h - QNX 4 to QNX Neutrino migration header
 *
 *  These are the messages that the mig4nto-procmgr handles.
 *
 *  Note: The QNX 4 struct _nameinfo has the process ID as type "mpid_t"
 *  which is 16-bits. A Neutrino PID will not fit, so this has been
 *  modified to be "pid_t". Code that depends on the 16-bit PID would need
 *  to be changed.
 */
#ifndef __MIG4NTO_PROCMSG_H_INCLUDED
#define __MIG4NTO_PROCMSG_H_INCLUDED

#include <sys/iomgr.h>
#include <sys/iomsg.h>
#include <sys/neutrino.h>
#include <sys/siginfo.h>

#define MGR_ID_PROCMGR		(_IOMGR_PRIVATE_BASE+0)

#define _PROCMGR_PATH		"/dev/mig4nto-procmgr"

#define _NAME_MAX_LEN		32

/* from QNX 4 sys/vc.h */

#define _VC_AT_SHARE        0x0001
#define _VC_AT_REM_ZOMBIE   0x0002

#define _QNX_PROXY_SIZE_MAX		100		/* maximum proxy message size */
#define _PROXY_TABLE_MAX   		2000	/* maximum number of proxies */
#define _PROXY_TRIGGER_CODE		_PULSE_CODE_MINAVAIL

#define _PROCMGR_ENUM_BASE	0x1000
enum proc_msg_type_e {
	_PROCMGR_QNX_NAME_ATTACH = _PROCMGR_ENUM_BASE,
	_PROCMGR_QNX_NAME_DETACH,
	_PROCMGR_QNX_NAME_LOCATE,
	_PROCMGR_QNX_NAME_QUERY,
	_PROCMGR_QNX_VC_ATTACH,
	_PROCMGR_QNX_VC_DETACH,
	_PROCMGR_QNX_VC_NAME_ATTACH,
	_PROCMGR_GET_NID,
	_PROCMGR_QNX_PROXY_ATTACH,
	_PROCMGR_QNX_PROXY_DETACH,
	_PROCMGR_QNX_PROXY_REM_ATTACH,
	_PROCMGR_QNX_PROXY_REM_DETACH,
	_PROCMGR_GET_TRIGGER_INFO,
	_PROCMGR_MAX_MSG_TYPE
};

/*
 * This one is sent to the trigger handling threads within the
 * migration process manager.  It still uses the structures below.
 */
#define _TRIGMGR_DO_TRIGGER		(_PROCMGR_MAX_MSG_TYPE+100)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qnx_name_attach_msg {
	nid_t         nid;
	unsigned char user_name_data[_NAME_MAX_LEN + 1];
} qnx_name_attach_msg_t;

typedef struct qnx_name_attach_reply {
	pid_t name_id;
} qnx_name_attach_reply_t;

typedef struct qnx_name_detach_msg {
	nid_t nid;
	int   name_id;
} qnx_name_detach_msg_t;

typedef struct qnx_name_detach_reply {
	int retval;
} qnx_name_detach_reply_t;

typedef struct qnx_name_locate_msg {
	nid_t    nid;
	unsigned vc_size;
	char     name[_NAME_MAX_LEN + 1];
} qnx_name_locate_msg_t;

typedef struct qnx_name_locate_reply {
	pid_t    pid;
	unsigned copies; /* copied back to client. */
} qnx_name_locate_reply_t;

typedef struct qnx_name_query_msg {
	pid_t proc_pid;
	int   name_id;
} qnx_name_query_msg_t;

typedef struct qnx_name_query_reply {
	int    name_id;
	struct _nameinfo buf;
} qnx_name_query_reply_t;

typedef struct qnx_vc_attach_msg {
	nid_t    nid;
	pid_t    pid;
	unsigned length;
	int      flags;
} qnx_vc_attach_msg_t;

typedef struct qnx_vc_attach_reply {
	pid_t pid;
} qnx_vc_attach_reply_t;

typedef struct qnx_vc_detach_msg {
	pid_t pid;
} qnx_vc_detach_msg_t;

typedef struct qnx_vc_name_attach_msg {
	nid_t    nid;
	unsigned length;
	char     name[_NAME_MAX_LEN + 1];
} qnx_vc_name_attach_msg_t;

typedef struct qnx_vc_name_atach_reply {
	pid_t pid;
} qnx_vc_name_attach_reply_t;

typedef struct get_nid_reply {
	nid_t	nid;
} get_nid_reply_t;

typedef struct proxy_msg {
	int				hit_this_nd;
	pid_t			hit_this_pid;
	int				priority;
	int				creator_nd;
	pid_t			creator_pid;
	int				nbytes;
	unsigned char	data[_QNX_PROXY_SIZE_MAX];
} proxy_msg_t;

typedef struct get_trigger_info_reply {
	pid_t			trigger_mgr_pid;
	int				trigger_chid;
} get_trigger_info_reply_t;

typedef struct procmgr_msg {
	struct _io_msg					hdr;
	union {
		qnx_name_attach_msg_t  		attach;
		qnx_name_detach_msg_t  		detach;
		qnx_name_locate_msg_t  		locate;
		qnx_name_query_msg_t   		name_query;
		qnx_vc_attach_msg_t    		vc_attach;
		qnx_vc_detach_msg_t   		vc_detach;
		qnx_vc_name_attach_msg_t	vc_name_attach;
		proxy_msg_t					proxy;
	} un;
} procmgr_msg_t;

typedef struct procmgr_reply {
	int 							set_errno_to_this;
	union {
		qnx_name_attach_reply_t     attach;
		qnx_name_detach_reply_t     detach;
		qnx_name_locate_reply_t     locate;
		qnx_name_query_reply_t      name_query;
		qnx_vc_attach_reply_t      	vc_attach;
		qnx_vc_name_attach_reply_t 	vc_name_attach;
		get_nid_reply_t            	get_nid;
		pid_t						proxy_pid;
		get_trigger_info_reply_t	trigger_info;
	} un;
} procmgr_reply_t;

#ifdef __cplusplus
};
#endif

#endif
