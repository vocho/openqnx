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



 
#include "externs.h"



int main(int argc, char *argv[]) {
	dispatch_t				*dpp;
	resmgr_context_t		*ctp;
	resmgr_attr_t			res_attr;
	resmgr_connect_funcs_t	connect_funcs1;
	resmgr_connect_funcs_t	connect_funcs2;
	resmgr_io_funcs_t		io_funcs1;
	resmgr_io_funcs_t		io_funcs2;
	struct slogdev			*trp;
	int				daemon_flag = 0;

	// Parse any options.
	options(argc, argv);

	//	
	// Buffer allocation.
	// The design puts all the data structures in one big contiguious
	// chunk of memory. Makes it easy to analyze if it was in a memory
	// mapped SRAM card which survives a crash or power failure.
	//
	trp = &SlogDev;
	trp->beg = malloc(NumInts*sizeof(int));
	trp->end = trp->beg + NumInts;
	if(trp->beg == NULL) {
		fprintf(stderr, "%s: Insufficient memory to allocate buffers.\n", __progname);
		exit(EXIT_FAILURE);
	}
	slogger_init(trp);

	// Create a dispatch context to receive messages.
	if((dpp = dispatch_create()) == NULL) {
		fprintf(stderr, "%s: Unable to allocate dispatch context\n", __progname);
		exit(EXIT_FAILURE);
	}

	// Init resmgr attributes
	memset(&res_attr, 0, sizeof res_attr);
	res_attr.nparts_max = 2;
	// All slogf/slogb calls should fit in the intial receive
	res_attr.msg_max_size = _SLOG_MAXSIZE + _SLOG_HDRINTS + sizeof(io_msg_t);

	// Init funcs for handling /dev/slog messages
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs1, _RESMGR_IO_NFUNCS, &io_funcs1);
	connect_funcs1.unlink = io_unlink;
	io_funcs1.read        = io_read;
	io_funcs1.write       = io_write;
	io_funcs1.unblock     = io_unblock;

	// Create /dev/slog in the pathname space. At this point we can get opens.
	iofunc_attr_init(&trp->attr, S_IFCHR | 0666, 0, 0);
	if((trp->id = resmgr_attach(dpp, &res_attr, "/dev/slog", _FTYPE_ANY, _RESMGR_FLAG_SELF,
		&connect_funcs1, &io_funcs1, &trp->attr)) == -1) {
		fprintf(stderr, "%s: Unable to allocate device %s (%s)\n", __progname, "/dev/slog", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// Init funcs for handling /dev/slog messages
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs2, _RESMGR_IO_NFUNCS, &io_funcs2);
	io_funcs2.write       = io_console_write;

	// Create /dev/console in the pathname space. At this point we can get opens.
	if(resmgr_attach(dpp, &res_attr, "/dev/console", _FTYPE_ANY, 0,
		&connect_funcs2, &io_funcs2, &trp->attr) == -1) {
		fprintf(stderr, "%s: Unable to allocate device %s (%s)\n", __progname, "/dev/console", strerror(errno));
		exit(EXIT_FAILURE);
	}

	// If a logfile was specified then create a thread to write it
	if(LogFname) {
		sigset_t	signalset;

		if(pthread_create(0, NULL, &logger, NULL) != 0) {
			fprintf(stderr, "%s: Unable to create logger thread.\n", __progname);
			exit(EXIT_FAILURE);
		}

	// We want the logger thread to catch signals. Not us!
	sigemptyset(&signalset);
	sigfillset(&signalset);
	pthread_sigmask(SIG_BLOCK, &signalset, NULL);
	}

	// Run in backgound
	if(Verbose) {
		daemon_flag = PROCMGR_DAEMON_NODEVNULL;
	}
	if (procmgr_daemon(EXIT_SUCCESS, daemon_flag) == -1) {
		fprintf(stderr, "%s: Couldn't become daemon.\n", argv[0]);
	}

	// Slogger is single-threaded
	ctp = resmgr_context_alloc(dpp);

	for(;;) {
		if((ctp = resmgr_block(ctp)) == NULL)
			exit(EXIT_FAILURE);

		resmgr_handler(ctp);
	}

	exit(EXIT_SUCCESS);
}


void
slogger_init(struct slogdev *trp) {

	memset(trp->beg, 0, NumInts * sizeof(trp->beg[0]));
	trp->get = trp->put = trp->beg;
	trp->cnt = 0;
}


//
// We override the default functions for allocating the OCB so we can get
// control and build a list of open for reads on /dev/slog. We could
// have also hooked the io_open/io_close function but this was easier.
//
IOFUNC_OCB_T *iofunc_ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *attr) {
	struct slogdev		*trp = (struct slogdev *) attr;
	struct ocbs			*ocbl;
	IOFUNC_OCB_T		*ocb;

	if((ocb = calloc(1, sizeof(*ocb)))) {

		// Only keep a list of open for reads on /dev/slog.
		// The trp->id contains the id for /dev/slog.
		if(trp->id != ctp->id || (((io_open_t *)ctp->msg)->connect.ioflag & _IO_FLAG_RD) == 0)
			return(ocb);

		// We keep a list of readers which is need to check for overflow..
		if((ocbl = calloc(1, sizeof(*ocbl)))) {
			ocbl->ocb = ocb;
			ocbl->next = trp->ocbs;
			trp->ocbs = ocbl;
			return(ocb);
		}

		free(ocb);
	}

	return(NULL);
}


//
// We also hook the free function which is called on a close.
//
void iofunc_ocb_free(IOFUNC_OCB_T *ocb) {
	struct slogdev		*trp = (struct slogdev *) ocb->attr;
	struct ocbs			*cur, *prev;

	prev = (struct ocbs *) &trp->ocbs;
	for(; (cur = prev->next) ; prev = cur)
		if(cur->ocb == ocb) {
			prev->next = cur->next;
			free(cur);
			break;
		}
	free(ocb);
}

__SRCVERSION("main.c $Rev: 200110 $");
