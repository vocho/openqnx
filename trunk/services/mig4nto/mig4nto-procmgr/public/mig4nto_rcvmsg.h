/*
 *  mig4nto_rcvmsg.h - QNX 4 to QNX Neutrino migration header
 *
 *	These are messages handled by the Receivemx() migration library
 *	function
 */
#ifndef __MIG4NTO_RCVMSG_H_INCLUDED
#define __MIG4NTO_RCVMSG_H_INCLUDED

#include <sys/iomsg.h>
#include <sys/iomgr.h>

enum receive_msg_type_e {
	_RCVMSG_FROM_SEND = 0x1000,	/* User called Send()    */
	_RCVMSG_FROM_PROXY,			/* User called Trigger() */
	_RCVMSG_MAX_MSG_TYPE
};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct receive_msg_hdr {
	enum receive_msg_type_e	type;
	pid_t					senders_pid;
} receive_msg_hdr_t;
				
typedef struct receive_from_proxy_msg {
	int           nbytes;
	/* the data follows here for a size of nbytes */
} receive_from_proxy_msg_t;

typedef struct receive_msg {
	receive_msg_hdr_t				hdr;
	union {
		receive_from_proxy_msg_t	proxy;
		struct _pulse				pulse;
	} un;
} receive_msg_t;

#ifdef __cplusplus
};
#endif

#endif
