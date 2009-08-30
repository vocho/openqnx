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




#ifndef _ASYNCMSG_PRIV_H_INCLUDED
#define _ASYNCMSG_PRIV_H_INCLUDED

#ifndef _ASYNCMSG_H_INCLUDED
#include <sys/asyncmsg.h>
#endif

#ifndef _STRING_H_INCLUDE
#include <string.h>
#endif

struct _asyncmsg_channel_context {
	pthread_mutex_t mutex;
	int (*recvbuf_cb)(size_t bufsize, unsigned num_bufs, void*bufs[], int flags);
	int buffer_size;
	int max_num_buffer;
	struct _asyncmsg_get_header *free;
	int num_free;
	iov_t *iovs;
};

struct _asyncmsg_connect_context {
	unsigned flags;
	struct _asyncmsg_connection_descriptor acd;
};
#define _ASYNCMSG_CONNECT_TIMEROFF 1

/* by default, every get will try to receive 5 message */
#define _ASYNCMSG_DEFAULT_GET    5

/* for _asyncmsg_handle */
#define _ASYNCMSG_HANDLE_ADD     0
#define _ASYNCMSG_HANDLE_LOOKUP  1
#define _ASYNCMSG_HANDLE_DELETE  2
#define _ASYNCMSG_HANDLE_CHANNEL 0x80000000

extern void * _asyncmsg_handle(int id, int cmd, void *handle);

#endif
