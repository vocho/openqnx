/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <sys/iomsg.h>
#include <sys/resmgr.h>
#include <pthread.h>
#include <atomic.h>
#include <semaphore.h>
#include <unistd.h>
#include <process.h>
#include <mig4nto.h>
#include <mig4nto_procmsg.h>
#include <mig4nto_table.h>

unsigned char   *progname;        		/* argv[0]                */
unsigned char   verbose;          		/* Handle -v option       */
										 
static char *msg_type_strs[] = {
	"QNX_NAME_ATTACH",
	"QNX_NAME_DETACH",
	"QNX_NAME_LOCATE",
	"QNX_NAME_QUERY",
	"QNX_VC_ATTACH", 
	"QNX_VC_DETACH",
	"QNX_VC_NAME_ATTACH",
	"GET_NID",
	"QNX_PROXY_ATTACH",
	"QNX_PROXY_DETACH",
	"QNX_PROXY_REM_ATTACH",
	"QNX_PROXY_REM_DETACH",
	"GET_TRIGGER_INFO"
};

static unsigned        qnx4_nid = 1;			/* Default node number.   */

extern int name_init(void);
extern void name_handle_close_ocb(pid_t pid);
extern void do_qnx_name_attach(procmgr_msg_t *msg, procmgr_reply_t *reply, pid_t from_pid);
extern void do_qnx_name_detach(procmgr_msg_t *msg, procmgr_reply_t *reply, pid_t from_pid);
extern void do_qnx_name_locate(procmgr_msg_t *msg, procmgr_reply_t *reply);
extern void do_qnx_name_query(procmgr_msg_t *msg, procmgr_reply_t *reply);
extern int proxy_init(int n);
extern void proxy_handle_close_ocb(int nd, pid_t pid);
extern void do_qnx_proxy_attach(procmgr_msg_t *msg, procmgr_reply_t *reply);
extern void do_qnx_proxy_detach(procmgr_msg_t *msg, 
		procmgr_reply_t *reply, struct _msg_info *info);
extern void do_qnx_proxy_rem_attach(procmgr_msg_t *msg, procmgr_reply_t *reply);
extern void do_qnx_proxy_rem_detach(procmgr_msg_t *msg, procmgr_reply_t *reply);
extern void do_get_trigger_info(procmgr_msg_t *msg, procmgr_reply_t *reply);

static int io_close_ocb(resmgr_context_t *ctp, void *none, RESMGR_OCB_T *ocb);
static int io_msg(resmgr_context_t *ctp, io_msg_t *msg, RESMGR_OCB_T *ocb);
static void do_get_nid(procmgr_msg_t *msg, procmgr_reply_t *reply);

int
main(int argc, char *argv[])
{
	iofunc_attr_t			ioattr;
	dispatch_t				*dpp;
	resmgr_attr_t			rattr;
	resmgr_connect_funcs_t	connect_funcs;
	resmgr_io_funcs_t		io_funcs;
	resmgr_context_t		*ctp;
	int						arg;
	int						numtrigthreads = -1;

	progname = argv[0];

	while ((arg = getopt(argc, argv, "n:t:v")) != -1)
		switch (arg) {
		case 'n':
			qnx4_nid = atoi(optarg);
			break;
		case 't':
			numtrigthreads = atoi(optarg);
			break;
		case 'v':
			verbose++;
			break;
		case '?':
			/* getopt already reported this error */
			exit(EXIT_FAILURE);
		}

	if (proxy_init(numtrigthreads) == -1)
		exit(EXIT_FAILURE);
	
	if (name_init() == -1)
		exit(EXIT_FAILURE);
	
	if ((dpp = dispatch_create()) == NULL) {
		fprintf(stderr, "%s: Unable to allocate dispatch context.\n", progname);
		exit(EXIT_FAILURE);
	}

	memset(&rattr, 0, sizeof(resmgr_attr_t));
	rattr.msg_max_size = sizeof(proxy_msg_t);
	rattr.nparts_max = 1;

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
					 _RESMGR_IO_NFUNCS, &io_funcs);
	io_funcs.msg = io_msg;
	io_funcs.close_ocb = io_close_ocb;

	memset(&ioattr, 0, sizeof(iofunc_attr_t));
	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);

	if (resmgr_attach(dpp, &rattr, _PROCMGR_PATH, _FTYPE_ANY,
					0, &connect_funcs, &io_funcs, &ioattr) == -1) {
		fprintf(stderr, "%s: Unable to attach name.\n", progname);
		exit(EXIT_FAILURE);
	}

	if ((ctp = resmgr_context_alloc(dpp)) == NULL) {
		fprintf(stderr, "%s: Unable to allocate context.\n", progname);
		exit(EXIT_FAILURE);
	}

	for (;;) {
		resmgr_context_t *new_ctp;
		if ((new_ctp = resmgr_block(ctp)) == NULL) {
			fprintf(stderr, "%s: Resource manager blocking failed.\n", progname);
		} else {
			ctp = new_ctp;
			resmgr_handler(ctp);
		}
	}
	return 0;
}

static int
io_close_ocb(resmgr_context_t *ctp, void *none, RESMGR_OCB_T *ocb)
{
	if (verbose)
		printf("%s: close from pid %d; cleaning up.\n", 
				progname, ctp->info.pid);

	name_handle_close_ocb(ctp->info.pid);
	proxy_handle_close_ocb(ctp->info.nd, ctp->info.pid);
	

	return EOK;
}

static int
io_msg(resmgr_context_t *ctp, io_msg_t *iomsg, RESMGR_OCB_T *ocb)
{
	procmgr_msg_t	*msg = (procmgr_msg_t *) iomsg;
	procmgr_reply_t	reply;

	if (verbose) {
		if (msg->hdr.subtype >= _PROCMGR_ENUM_BASE
		 && msg->hdr.subtype < _PROCMGR_MAX_MSG_TYPE) {
			printf("%s: Message type is %s\n", 
					progname, msg_type_strs[msg->hdr.subtype-_PROCMGR_ENUM_BASE]);
		} else {
			printf("%s: Message type is %d\n", progname, msg->hdr.subtype);
		}
	}

	switch(msg->hdr.subtype) {
	case _PROCMGR_QNX_NAME_ATTACH:    
		do_qnx_name_attach(msg, &reply, ctp->info.pid);
		break;
	case _PROCMGR_QNX_NAME_DETACH:    
		do_qnx_name_detach(msg, &reply, ctp->info.pid);
		break;
	case _PROCMGR_QNX_NAME_LOCATE:    
		do_qnx_name_locate(msg, &reply);
		break;
	case _PROCMGR_QNX_NAME_QUERY:     
		do_qnx_name_query(msg, &reply);
		break;
	case _PROCMGR_QNX_VC_ATTACH:     
		break; /* Don't get these yet */
	case _PROCMGR_QNX_VC_DETACH:      
		break; /* Don't get these yet */
	case _PROCMGR_QNX_VC_NAME_ATTACH: 
		break; /* Don't get these yet */
	case _PROCMGR_GET_NID:    
		do_get_nid(msg, &reply);
		break;
	case _PROCMGR_QNX_PROXY_ATTACH:     
		do_qnx_proxy_attach(msg, &reply);
		break;
	case _PROCMGR_QNX_PROXY_DETACH:     
		do_qnx_proxy_detach(msg, &reply, &ctp->info);
		break;
	case _PROCMGR_QNX_PROXY_REM_ATTACH: 
		do_qnx_proxy_rem_attach(msg, &reply);
		break;
	case _PROCMGR_QNX_PROXY_REM_DETACH: 
		do_qnx_proxy_rem_detach(msg, &reply);
		break;
	case _PROCMGR_GET_TRIGGER_INFO:
		do_get_trigger_info(msg, &reply);
		break;
	default: 
		return ENOSYS;
	}
	MsgReply(ctp->rcvid, 0, &reply, sizeof(procmgr_reply_t));
	return _RESMGR_NOREPLY;
}

static void
do_get_nid(procmgr_msg_t *msg, procmgr_reply_t *reply)
{
	reply->un.get_nid.nid = qnx4_nid;
}
