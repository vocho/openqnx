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



#define MQHDR
#include "externs.h"
#undef  MQHDR
#include <confname.h>
#include <limits.h>
#include <stdio.h>
#include <sys/procmgr.h>
#include <sys/resource.h>
#include <sys/sysmgr.h>


// Bucket/Chunk sizes for memchunk memory allocator
static const size_t	memchunks[] = {
						sizeof(struct ocb), sizeof(MQWAIT),
						sizeof(MQDEV) + 32, sizeof(MQDEV) + 68,
						MQ_DATAOFF +   564, MQ_DATAOFF +  1344,
						MQ_DATAOFF +  2024, MQ_DATAOFF +  4064,
						MQ_DATAOFF +  8158, MQ_DATAOFF + 12256,
						MQ_DATAOFF + 16352, MQ_DATAOFF + 20448,
						MQ_DATAOFF + 24544, MQ_DATAOFF + 28638,
					};

int
main(int argc, char *argv[]) {
	struct rlimit					fdlimit;
	resmgr_attr_t					res_attr;
	resmgr_context_t				*ctp;
	extern char						*__progname;
	int								semaphores;

	/*
	 * Attempt to co-exist with 6.3 procnto which provides named semaphores
	 * itself.  If _SC_SEMAPHORES set already, then either duplicate mqueue
	 * or new procnto; either way don't put up _FTYPE_SEM services ourself.
	 */
	semaphores = !((semaphores = sysconf(_SC_SEMAPHORES)) != -1 && semaphores != 0);

	func_init();

	if(MemchunkInit(&memchunk, sizeof(memchunks) / sizeof(memchunks[0]), memchunks) != EOK) {
		fprintf(stderr, "%s: Unable to initialise memory allocator\n", __progname);
		return EXIT_FAILURE;
	}

	if((dpp = dispatch_create()) == NULL) {
		fprintf(stderr, "%s: Unable to allocate dispatch context\n", __progname);
		return EXIT_FAILURE;
	}

	options(argc, argv);

	// Init resmgr attributes
	memset(&res_attr, 0, sizeof res_attr);
	res_attr.flags = RESMGR_FLAG_CROSS_ENDIAN;
	res_attr.nparts_max = 2;
	res_attr.msg_max_size = sizeof(io_write_t) + 2560;

	// Initialize mountpoint attributes
	iofunc_attr_init(&mq_dir_attr.attr, S_IFDIR | S_ISVTX | S_IPERMS, 0, 0);
	mq_dir_attr.attr.mount = &mq_mount;
	mq_mount.conf = IOFUNC_PC_CHOWN_RESTRICTED | IOFUNC_PC_NO_TRUNC;
	mq_mount.funcs = &ocb_funcs;
	mq_dir_attr.link = NULL;

	// Initialize mountpoint attributes
	if (semaphores) {
	iofunc_attr_init(&sem_dir_attr.attr, S_IFDIR | S_ISVTX | S_IPERMS, 0, 0);
	sem_dir_attr.attr.mount = &sem_mount;
	sem_mount.conf = IOFUNC_PC_CHOWN_RESTRICTED | IOFUNC_PC_NO_TRUNC;
	sem_mount.funcs = &ocb_funcs;
	sem_dir_attr.link = NULL;
	}

	// Create the mount points for all message queues and named semaphores
	if(resmgr_attach(dpp, &res_attr, NULL, _FTYPE_MQUEUE, _RESMGR_FLAG_DIR | _RESMGR_FLAG_FTYPEONLY,
		&mq_connect_funcs, &mq_io_dir_funcs, &mq_dir_attr) == -1) {
		fprintf(stderr, "%s: Unable to allocate mqueue (%s)\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}
	if (semaphores) {
	if(resmgr_attach(dpp, &res_attr, NULL, _FTYPE_SEM, _RESMGR_FLAG_DIR | _RESMGR_FLAG_FTYPEONLY,
		&mq_connect_funcs, &mq_io_dir_funcs, &sem_dir_attr) == -1) {
		fprintf(stderr, "%s: Unable to allocate sem (%s)\n", __progname, strerror(errno));
		return EXIT_FAILURE;
	}
	}

	/*
	 The maximum number of semaphores/mqueues that the system can support
	 is actually limited by the number of fd connections it can establish
	 to proc (each resmgr_attach() is equivalent to an open()).  At some
	 future point we will be able to boost our limit to match if it is 
	 lower.  Note that it must be the _sum_ of the MAX_NUM_SEM and MAX_NUM_MQ
	 that we boost to since a user might want to use all of those together.

	 For now we ignore this fact and let the limits float.
	*/
	if (getrlimit(RLIMIT_NOFILE, &fdlimit) == -1) {
		fdlimit.rlim_cur = semaphores ? __max(MAX_NUM_SEM, MAX_NUM_MQ) : MAX_NUM_MQ;
	}
	max_num_mq = __min(fdlimit.rlim_cur, MAX_NUM_MQ);
	max_num_sem = semaphores ? __min(fdlimit.rlim_cur, MAX_NUM_SEM) : 0;

	//Set the system configuration parameters
	sysmgr_sysconf_set(0, _SC_MESSAGE_PASSING, _POSIX_MESSAGE_PASSING);
	sysmgr_sysconf_set(0, _SC_MQ_OPEN_MAX, max_num_mq);
	sysmgr_sysconf_set(0, _SC_MQ_PRIO_MAX, MQ_PRIO_MAX);
	if (semaphores) {
	sysmgr_sysconf_set(0, _SC_SEMAPHORES, _POSIX_SEMAPHORES);
	sysmgr_sysconf_set(0, _SC_SEM_NSEMS_MAX, max_num_sem);
	sysmgr_sysconf_set(0, _SC_SEM_VALUE_MAX, SEM_VALUE_MAX);
	}

	if(!nodaemon) {
		procmgr_daemon(EXIT_SUCCESS, PROCMGR_DAEMON_NODEVNULL);
	}

	// mqueue is single-threaded (define MQUEUE_1_THREAD for optimisation)
	ctp = resmgr_context_alloc(dpp);

	for(;;) {
		if((ctp = resmgr_block(ctp)) == NULL) {
			return EXIT_FAILURE;
		}
		resmgr_handler(ctp);
	}

	return EXIT_SUCCESS;
}

/*
 *  Linked-list manipulation routines for pending r/w clients and msgs.
 *  The msgs have both head/tail for optimised append to the end of the
 *  list (common situation as most often msgs all have the same priority).
 */
void LINK_PRI_CLIENT(MQWAIT **head, MQWAIT *client)
{
MQWAIT	*m;

	while ((m = *head) != NULL && m->priority >= client->priority)
		head = &m->next;
	client->next = m, *head = client;
}
void LINK_PRI_MSG(MQMSG *head[2], MQMSG *msg)
{
MQMSG	*m;

	if ((m = head[0]) == NULL) {
		msg->next = NULL, head[0] = head[1] = msg;
	}
	else if (msg->priority > m->priority) {
		msg->next = m, head[0] = msg;
	}
	else if (msg->priority <= head[1]->priority) {
		msg->next = NULL, head[1]->next = msg, head[1] = msg;
	}
	else {
		while (m->next->priority >= msg->priority)
			m = m->next;
		msg->next = m->next, m->next = msg;
	}
}

__SRCVERSION("main.c $Rev: 169544 $");
