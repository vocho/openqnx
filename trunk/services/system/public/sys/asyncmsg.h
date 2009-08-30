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
 *  sys/asyncmsg.h
 *

 */
#ifndef __ASYNCMSG_H_INCLUDED
#define __ASYNCMSG_H_INCLUDED

#ifndef __NEUTRINO_H_INCLUDED
#include <sys/neutrino.h>
#endif

#ifndef __PTHREAD_H_INCLUDED
#include <pthread.h>
#endif

#ifndef __INTTYPES_H_INCLUDED
#include <inttypes.h>
#endif

__BEGIN_DECLS

union _channel_connect_attr {
	unsigned flags;				/* flags of the channel */
	mode_t mode;				/* access permissions */
	size_t bufsize;				/* size of kernel buffer */
	unsigned maxbuf;			/* maximum number of buffers allowed */
	struct {
		struct sigevent event;	/* the event to be set for notification */
		int	coid;				/* to identify the owner */
	} ev;
	unsigned num_curmsgs;		/* how many msgs in the channel send queue */
	struct _cred_info cred;		/* credential information */
};

/* flags for _channel_connect_attr */
#define _NTO_CHANCON_ATTR_CONFLAGS		0x00000001
#define _NTO_CHANCON_ATTR_CHANFLAGS		0x00000002
#define _NTO_CHANCON_ATTR_MODE			0x00000004
#define _NTO_CHANCON_ATTR_BUFSIZE		0x00000008
#define _NTO_CHANCON_ATTR_MAXBUF		0x00000010
#define _NTO_CHANCON_ATTR_EVENT			0x00000020
#define _NTO_CHANCON_ATTR_CURMSGS		0x00000040
#define _NTO_CHANCON_ATTR_CRED			0x00000080

struct _asyncmsg_get_header {
	struct _msg_info info;				/* same as synchronous message */
	int err;							/* error status of this message */
	iov_t *iov;							/* pointer to iov list for message body */
	int parts;							/* size of this iov array */
	struct _asyncmsg_get_header *next;	/* linked list pointer */
	unsigned reserve[2];				/* reserve */
};

struct _asyncmsg_put_header {
	int err;							/* error status of the message */
	iov_t *iov;							/* pointer to iov list for message body */
	int parts; 							/* size of this iov array */
	unsigned handle;					/* a handle passed in by user and used by user */
	int (*cb)(int err, void* buf, unsigned handle);
                                        /* user callback */
	unsigned reserve;					/* reserve */
};

struct _asyncmsg_connection_attr { 
	int (*call_back)(int err, void* buff, unsigned handle); /* callback function for notification */
	size_t buffer_size; 									/* message buffer size */
	unsigned max_num_buffer;								/* maximum number of buffer allowed for this connection */
	unsigned trigger_num_msg;								/* triggering criteria by number of pending message */
	struct _itimer trigger_time;							/* triggering criteria by time passed since last send kernel call*/
	unsigned reserve;										/* reserve */
};

struct _asyncmsg_connection_descriptor {
	unsigned flags;							/* flags for the async connection */
	struct _asyncmsg_put_header *sendq;		/* send queue */
	unsigned sendq_size;				    /* send queue size */
	unsigned sendq_head;		            /* head of the send queue */
	unsigned sendq_tail;		            /* tail of the send queue */
	unsigned sendq_free;		            /* start of the free list */
	int err;								/* error status of this connection */
	struct sigevent ev;						/* the event to be sent for notification */
	unsigned num_curmsg;					/* number of messages pending on this connection */
	timer_t ttimer;							/* triggering timer */
	pthread_cond_t block_con;				/* condvar for blocking if send header queue is full */
	pthread_mutex_t mu;						/* mutex to protect the data structure and for the condvar */
	unsigned reserve;						/* reserve */
	struct _asyncmsg_connection_attr attr;	/* attribute of this connection */
	unsigned reserves[3];					/* reserve */
};

#define ASYNCMSG_FLUSH_NONBLOCK 1

#define ASYNCMSG_RECVBUF_ALLOC  1
#define ASYNCMSG_RECVBUF_FREE   2

extern int asyncmsg_channel_create(unsigned flags,  mode_t mode, size_t buffer_size, unsigned max_num_buffer, const struct sigevent *ev, int (*recvbuf_callback)(size_t bufsize, unsigned num_bufs, void*bufs[], int flags));

extern int asyncmsg_channel_destroy(int chid);

extern int asyncmsg_connect_attach(uint32_t nd, pid_t pid, int chid, unsigned index, unsigned flags, const struct _asyncmsg_connection_attr *attr);

extern int asyncmsg_connect_detach(int coid);

extern int asyncmsg_flush(int coid, int mode);

extern int asyncmsg_connect_attr(int coid, struct _asyncmsg_connection_attr *old_attr, const struct _asyncmsg_connection_attr *new_attr);

extern int asyncmsg_put(int coid, const void *buff, size_t size, unsigned handle, int (*call_back)(int err, void* buf, unsigned handle));

extern int asyncmsg_putv(int coid, const iov_t* iov, int parts, unsigned handle, int (*call_back)(int err, void* buf, unsigned handle));

extern struct _asyncmsg_get_header* asyncmsg_get(int chid);

extern void *asyncmsg_malloc(size_t size);

extern void asyncmsg_free(void *buf);


__END_DECLS

#endif


/* __SRCVERSION("asyncmsg.h $Rev: 153052 $"); */
