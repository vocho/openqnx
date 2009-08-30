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




#define THREAD_POOL_HANDLE_T struct _aio_control_block
#define THREAD_POOL_PARAM_T  struct _aio_context
#include <sys/dispatch.h>
#include <pthread.h>

#define _AIO_FLAG_ENQUEUE     0x00000001
#define _AIO_FLAG_IN_PROGRESS 0x00000002
#define _AIO_FLAG_DONE        0x00000004
#define _AIO_FLAG_QMASK       0x00000007
#define _AIO_FLAG_SUSPEND     0x00000008

enum {
	  _AIO_OPCODE_READ = 1,
	  _AIO_OPCODE_WRITE,
	  _AIO_OPCODE_SYNC,
	  _AIO_OPCODE_DSYNC
};

struct _aio_prio_list {
	struct _aio_prio_list *next;
	int                   priority;
	struct aiocb          *head;
	struct aiocb          **tail;
};

#define _AIO_PRIO_LIST_LOW   (8)

struct _aio_context;
struct _aio_control_block {
	pthread_mutex_t       cb_mutex;
	pthread_cond_t        cb_cond;
	struct _aio_prio_list *cb_plist;
	struct _aio_prio_list *cb_plist_free;
	int                   cb_nfree;
	thread_pool_t         *tp;
	struct _aio_context   *ct_list;
	struct _aio_context   *ct_free;
};

struct _aio_waiter {
	pthread_cond_t        w_cond;
	unsigned              w_count;
};

extern struct _aio_control_block *_aio_cb;
extern int _aio_init(thread_pool_attr_t *pool_attr);
extern int _aio_insert(struct aiocb *aiocbp);
extern int _aio_insert_prio(struct aiocb *aiocbp);

/* __SRCVERSION("aio_priv.h $Rev: 153052 $"); */
