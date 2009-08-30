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

#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <malloc.h>
#include <malloc-control.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>


extern int __malloc_bt_depth;
extern int __malloc_error_btdepth;
extern int __malloc_print_nodetail;
extern int __malloc_trace_minsz;
extern int __malloc_trace_maxsz;
extern int malloc_verbose;

void muntrace();
void __dbgm_print_currently_alloced_blocks(int fd);
dispatch_t *_dispatch_create(int chid, unsigned flags);
void __dbgm_print_snapshots(int fd);

// TODO
// locking
// all commands
// 

static void
print_verbose(const char *fmt, ...) {
	va_list arglist;
	if (malloc_verbose) {
		va_start(arglist, fmt);
		vfprintf(stderr, fmt, arglist);
		va_end(arglist);
	}
}

static int
io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb) {
	int status=0;
	int fd;
	void *dptr;
	int nbytes;
	union {
		__dbgm_na_msg na_msg;
		__dbgm_one_int_msg one_int_msg;
		__dbgm_two_int_msg two_int_msg;
		__dbgm_str_msg str_msg;
		__dbgm_get_state_msg gs_msg;
	} dbgm_data;
	int fdset=0;
    
	memset(&dbgm_data, 0, sizeof(dbgm_data));
	if ((status = iofunc_devctl_default(ctp, msg, ocb)) != _RESMGR_DEFAULT)
		return (status);

	dptr = _DEVCTL_DATA (msg->i);
	ENTER();
	MALLOC_INIT();

	switch (msg -> i.dcmd) {
	case DBGM_DEVCTL_GET_STATE: {
		__dbgm_get_state_msg *gs_msg;
		print_verbose("received devctl get state\n");
		gs_msg = (__dbgm_get_state_msg *)dptr;
		gs_msg->tmin = __malloc_trace_minsz;
		gs_msg->tmax = __malloc_trace_maxsz;
		gs_msg->btdepth = __malloc_error_btdepth;
		gs_msg->tdepth = __malloc_bt_depth;
		//gs_msg->btdepth = __malloc_bt_depth;
		//gs_msg->tdepth = __malloc_error_btdepth;
		strcpy(gs_msg->tfile, __malloc_trace_filename);
		nbytes = sizeof(__dbgm_get_state_msg);
		memset(&msg->o, 0, sizeof(msg->o));
		msg->o.ret_val = status;
		msg->o.nbytes = nbytes;
		LEAVE();
		return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + nbytes));
	}
	case DBGM_DEVCTL_NOBT:
		resmgr_msgread(ctp, &dbgm_data.na_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl event_btdepth -1 \n");
		mallopt(MALLOC_EVENTBTDEPTH, -1);
		break;
	case DBGM_DEVCTL_TRACEMIN:
		resmgr_msgread(ctp, &dbgm_data.one_int_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl tracemin: %d\n", dbgm_data.one_int_msg.val);
		mallopt(MALLOC_TRACEMIN, dbgm_data.one_int_msg.val);
		break;
	case DBGM_DEVCTL_TRACEMAX:
		resmgr_msgread(ctp, &dbgm_data.one_int_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl tracemax: %d\n", dbgm_data.one_int_msg.val);
		mallopt(MALLOC_TRACEMAX, dbgm_data.one_int_msg.val);
		break;
	case DBGM_DEVCTL_TRACERANGE:
		resmgr_msgread(ctp, &dbgm_data.two_int_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl tracerange: %d:%d\n", dbgm_data.two_int_msg.val1, dbgm_data.two_int_msg.val2);
		mallopt(MALLOC_TRACEMIN, dbgm_data.two_int_msg.val1);
		mallopt(MALLOC_TRACEMAX, dbgm_data.two_int_msg.val2);
		break;
	case DBGM_DEVCTL_BTDEPTH: {
		int depth = 0;
		resmgr_msgread(ctp, &dbgm_data.one_int_msg, msg->i.nbytes, sizeof(msg->i));
		depth= dbgm_data.one_int_msg.val;
		print_verbose("received devctl event_btdepth: %d\n", depth);	
		mallopt(MALLOC_EVENTBTDEPTH, depth);
		break;
	}
	case DBGM_DEVCTL_TRACEDEPTH: {
		int depth = 0;
		resmgr_msgread(ctp, &dbgm_data.one_int_msg, msg->i.nbytes, sizeof(msg->i));
		depth = dbgm_data.one_int_msg.val;
		print_verbose("received devctl trace_btdepth: %d\n", depth);
		mallopt(MALLOC_TRACEBTDEPTH, depth);
		break;
	}
	case DBGM_DEVCTL_NOTRACEDEPTH:
		resmgr_msgread(ctp, &dbgm_data.na_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl trace_btdepth -1\n");
		mallopt(MALLOC_TRACEBTDEPTH, -1);
		break;
	case DBGM_DEVCTL_TRACEFILE: {
		char *name;
	    resmgr_msgread(ctp, &dbgm_data.str_msg, msg->i.nbytes, sizeof(msg->i));
		name = dbgm_data.str_msg.str;
		if (name != NULL) {
			print_verbose("received devctl trace_file: %s\n", name);
		} else {
			print_verbose("received devctl no trace_file\n");
		}
		mallopt(MALLOC_TRACEFILE, (int)name);
		break;
	}
	case DBGM_DEVCTL_NOTRACEFILE:
		resmgr_msgread(ctp, &dbgm_data.na_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl notrace_file\n");
		mallopt(MALLOC_TRACEFILE, 0);
		break;
	case DBGM_DEVCTL_DUMPUNREF:
		resmgr_msgread(ctp, &dbgm_data.str_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl dump unref\n");
		fd = -1;
		if (strcmp(dbgm_data.str_msg.str, "STDOUT") == 0) {
			fd = 1;
		} else if (strcmp(dbgm_data.str_msg.str, "STDERR") == 0) {
			fd = 2;
		} else {
			fd = open(dbgm_data.str_msg.str, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR );
			fdset = 1;
		}
		if (fd > 0) {
			malloc_dump_unreferenced(fd, NULL);
		}
		if (fdset) {
			close(fd);
		}
		break;
	case DBGM_DEVCTL_DUMPALLOCSTATE_NODETAIL:
		resmgr_msgread(ctp, &dbgm_data.na_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl dump_alloc_state_nodetail\n");
		__malloc_print_nodetail = 1;
		break;
	case DBGM_DEVCTL_DUMPALLOCSTATE:
		resmgr_msgread(ctp, &dbgm_data.str_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl dump_alloc_state\n");
		fd = -1;
		if (strcmp(dbgm_data.str_msg.str, "STDOUT") == 0) {
			fd = 1;
		} else if (strcmp(dbgm_data.str_msg.str, "STDERR") == 0) {
			fd = 2;
		} else {
			fd = open(dbgm_data.str_msg.str, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR );
			fdset = 1;
		}
		if (fd > 0) {
			//__dbgm_print_currently_alloced_blocks(fd);
			__dbgm_print_snapshots(fd);
		}
		if (fdset) {
			close(fd);
		}
		break;
	case DBGM_DEVCTL_CKALLOC: {
		int val;
		resmgr_msgread(ctp, &dbgm_data.one_int_msg, msg->i.nbytes, sizeof(msg->i));
		val = dbgm_data.one_int_msg.val;
		print_verbose("received devctl check alloc: %d\n", val);
		if (dbgm_data.one_int_msg.val > 0) {
			mallopt(MALLOC_CKALLOC, val);
		} else {
			mallopt(MALLOC_CKALLOC, 0);
		}
		break;
	}
	case DBGM_DEVCTL_CKACCESS: {
		int val;
		resmgr_msgread(ctp, &dbgm_data.one_int_msg, msg->i.nbytes, sizeof(msg->i));
		val = dbgm_data.one_int_msg.val;
		print_verbose("received devctl check access: %d\n", val);
		if (dbgm_data.one_int_msg.val > 0) {
			mallopt(MALLOC_CKACCESS, val);
		} else {
			mallopt(MALLOC_CKACCESS, 0);
		}
		break;
	}
	case DBGM_DEVCTL_CKCHAIN: {
		int val;
		resmgr_msgread(ctp, &dbgm_data.one_int_msg, msg->i.nbytes, sizeof(msg->i));
		val = dbgm_data.one_int_msg.val;
		print_verbose("received devctl check chain: %d\n", val);
		if (dbgm_data.one_int_msg.val > 0) {
			mallopt(MALLOC_CKCHAIN, val);
		} else {
			mallopt(MALLOC_CKCHAIN, 0);
		}
		break;
	}
	case DBGM_DEVCTL_CKBOUNDS: {
		int val;
		resmgr_msgread(ctp, &dbgm_data.one_int_msg, msg->i.nbytes, sizeof(msg->i));
		val = dbgm_data.one_int_msg.val;
		print_verbose("received devctl check bounds: %d\n", val);
		if (dbgm_data.one_int_msg.val > 0) {
			mallopt(MALLOC_FILLAREA, val);
		} else {
			mallopt(MALLOC_FILLAREA, 0);
		}
		break;
	}
	case DBGM_DEVCTL_EVENT_ACTION: {
		int val;
		resmgr_msgread(ctp, &dbgm_data.one_int_msg, msg->i.nbytes, sizeof(msg->i));
		val = dbgm_data.one_int_msg.val;
		print_verbose("received devctl set action: %d\n", val);
		if (dbgm_data.one_int_msg.val > 0) {
			mallopt(MALLOC_WARN, val);
			mallopt(MALLOC_FATAL, val);
		} else {
			mallopt(MALLOC_WARN, 0);
			mallopt(MALLOC_FATAL, 0);
		}
		break;
	}
	case DBGM_DEVCTL_VERIFY:
		resmgr_msgread(ctp, &dbgm_data.na_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl verify\n");
		mallopt(MALLOC_VERIFY, 1);
		break;
	case DBGM_DEVCTL_EVENTFILE: {
		char * name;
		resmgr_msgread(ctp, &dbgm_data.str_msg, msg->i.nbytes, sizeof(msg->i));
		name = dbgm_data.str_msg.str;
		if (name != NULL) {
			print_verbose("received devctl event_file: %s\n", name);
		} else {
			print_verbose("received devctl no event_file\n");
		}
		mallopt(MALLOC_EVENTFILE, (int)name);
		break;
	}
	case DBGM_DEVCTL_NOEVENTFILE:
		resmgr_msgread(ctp, &dbgm_data.na_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl noevent_file\n");
		mallopt(MALLOC_EVENTFILE, 0);
		break;
	case DBGM_DEVCTL_VERBOSE: {
		int val;
		resmgr_msgread(ctp, &dbgm_data.one_int_msg, msg->i.nbytes, sizeof(msg->i));
		val = dbgm_data.one_int_msg.val;
		mallopt(MALLOC_VERBOSE, val);
		print_verbose("received devctl verbose: %d\n", val);
		break;
	}
	case DBGM_DEVCTL_ERRFILE: {
		char * name;
		resmgr_msgread(ctp, &dbgm_data.str_msg, msg->i.nbytes, sizeof(msg->i));
		name = dbgm_data.str_msg.str;
		if (name != NULL) {
			print_verbose("received devctl error_file: %s\n", name);
		} else {
			print_verbose("received devctl no error_file\n");
		}
		mallopt(MALLOC_ERRFILE, (int)name);
		break;
	}
	case DBGM_DEVCTL_NOERRFILE:
		resmgr_msgread(ctp, &dbgm_data.na_msg, msg->i.nbytes, sizeof(msg->i));
		print_verbose("received devctl noerror_file\n");
		mallopt(MALLOC_ERRFILE, 0);
		break;
	case DBGM_DEVCTL_USE_DLADDR: {
		int val;
		resmgr_msgread(ctp, &dbgm_data.one_int_msg, msg->i.nbytes, sizeof(msg->i));
		val = dbgm_data.one_int_msg.val;
		mallopt(MALLOC_USE_DLADDR, val);
		print_verbose("received devctl use dladdr: %d\n", val);
		break;
	}
	default:
		print_verbose("unknown command\n");
		break;
	}

  LEAVE();
  return(EOK);

};

// TODO
// detach thread
void *__dbg_malloc_thread(void *arg)
{
	resmgr_connect_funcs_t    connect_funcs;
	resmgr_io_funcs_t         io_funcs;
	iofunc_attr_t             attr;
    /* declare variables we'll be using */
    resmgr_attr_t        resmgr_attr;
    dispatch_t           *dpp;
    dispatch_context_t   *ctp;
    int                  id;
	char buf[256];

	sigset_t              mask;
	int channelId = 0;
	

  
	sigfillset(&mask); /* Mask all allowed signals */
	pthread_sigmask(SIG_BLOCK, &mask, NULL); /* we should check the result */


    /* initialize dispatch interface */
	pthread_detach(pthread_self());
	
	channelId = ChannelCreate(0);
	// PR #44018: regular dispatch_create creates a channel which flag NTO_CHF_COID_DISCONNECT 
	// and it would conflict with appication channel flags because only one channel can be with this flag
    if((dpp = _dispatch_create(channelId, 0)) == NULL) {
        //fprintf(stderr, "DbgMalloc_Thread: Unable to allocate dispatch handle.\n");
		slogf(_SLOG_SETCODE(_SLOGC_TEST, 2), _SLOG_ERROR, "DBGMalloc_Thread: unable to dispatch error %d\n", errno);
        return(NULL); 
    }

    /* initialize resource manager attributes */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 2048;

    /* initialize functions for handling messages */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
                     _RESMGR_IO_NFUNCS, &io_funcs);
	io_funcs.devctl = io_devctl;

    /* initialize attribute structure used by the device */
    iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

		sprintf(buf, "%s_%d", MDBG_PREFIX, getpid());
    /* attach our device name */
    id = resmgr_attach(dpp,            /* dispatch handle */
                       &resmgr_attr,   /* resource manager attrs */
                       buf,            /* device name */
                       _FTYPE_ANY,     /* open type */
                       0,              /* flags */
                       &connect_funcs, /* connect routines */
                       &io_funcs,      /* I/O routines */
                       &attr);         /* handle */
    if(id == -1) {
        //fprintf(stderr, "DbgMalloc_Thread: Unable to attach name.\n");
		slogf(_SLOG_SETCODE(_SLOGC_TEST, 2), _SLOG_ERROR, "DBGMalloc_Thread: unable to attach error %d\n", errno);
        return(NULL);
    }

    /* allocate a context structure */
    ctp = dispatch_context_alloc(dpp);

    /* start the resource manager message loop */
    for(;;) {
        if((ctp = dispatch_block(ctp)) == NULL) {
            //fprintf(stderr, "DbgMalloc_Thread: block error\n");
			if (errno == EINTR) {
				continue;
			}
			slogf(_SLOG_SETCODE(_SLOGC_TEST, 2), _SLOG_ERROR, "DBGMalloc_thread: block error %d\n", errno);
			return(NULL);
        }
        dispatch_handler(ctp);
    }
}

// Generic function to start a thread
pthread_t __dbgm_StartThread(void *(*func)(void *), void *arg)
{
  pthread_t  threadID;
  pthread_attr_t attrib;
  struct sched_param param;
  struct sched_param our_param;

  pthread_attr_init (&attrib);
  pthread_attr_setinheritsched (&attrib, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy (&attrib, SCHED_RR);
  sched_getparam(0, &our_param);
  param.sched_priority = our_param.sched_priority;
  pthread_attr_setschedparam (&attrib, &param);

  pthread_create (&threadID, &attrib, func, arg);
  return(threadID);
}
