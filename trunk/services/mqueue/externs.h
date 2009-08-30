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



struct mqdev_entry;
#define IOFUNC_ATTR_T			struct mqdev_entry
struct ocb;
#define IOFUNC_OCB_T			struct ocb
struct MemchunkCtrl;

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <devctl.h>
#include <sys/dcmd_all.h>
#include <sys/dcmd_misc.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/ftype.h>
#include <gulliver.h>
#include "mq.h"
#include "proto.h"

// These are only guidelines, see comments in main()
#define MAX_NUM_SEM	max(_POSIX_SEM_NSEMS_MAX*2, 4096)
#define MAX_NUM_MQ	max(_POSIX_MQ_OPEN_MAX*2, 1024)

#ifdef MQHDR
	#define EXT
	#define INIT1(a)				= { a }
#else
	#define EXT extern
	#define INIT1(a)
#endif

EXT int							 chid;
EXT void						*dpp;
EXT iofunc_mount_t				 mq_mount;
EXT iofunc_mount_t				 sem_mount;
EXT struct mqdev_entry			 mq_dir_attr;
EXT struct mqdev_entry			 sem_dir_attr;
EXT unsigned					 num_sem, max_num_sem;
EXT unsigned					 num_mq, max_num_mq;
EXT struct MemchunkCtrl			*memchunk;
EXT iofunc_funcs_t				 ocb_funcs;
EXT resmgr_connect_funcs_t		 mq_connect_funcs;
EXT resmgr_io_funcs_t		 	 mq_io_funcs;
EXT resmgr_io_funcs_t			 mq_io_dir_funcs;
EXT int							 nodaemon;
