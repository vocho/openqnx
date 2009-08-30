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




#include <sys/resmgr.h>
#include "resmgr.h"

struct link							*_resmgr_link_list;
struct _resmgr_handle_table			_resmgr_io_table = {
	NULL,                                       /* vector */
	NULL,                                       /* free_list */
	PTHREAD_MUTEX_INITIALIZER,                  /* mutex */
	PTHREAD_COND_INITIALIZER,                   /* cond */
	0,                                          /* nentries */
	0,                                          /* total */
	0,                                          /* free */
	_RESMGR_CLIENT_MIN * _RESMGR_CLIENT_FD_MIN, /* min */
	NULL,                                       /* free_buckets */
	0,                                          /* total_buckets */
	0,                                          /* nfree_buckets */
	_RESMGR_CLIENT_MIN,                         /* min_buckets */
	_RESMGR_CLIENT_FD_MAX / 4                   /* nlists_max (size of each bucket) */
};
struct pulse_func					*_resmgr_pulse_list;

pthread_key_t						_resmgr_thread_key;	



__SRCVERSION("resmgr_data.c $Rev: 153052 $");
