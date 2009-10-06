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
#include "mt_kertrace.h"

int kdecl
ker_mt_ctl(THREAD *act, struct kerargs_mt_ctl *kap)
{
	switch (kap->type)
	{

	/* Gets the useful data in order to fire the flush pulse */
	case _MT_CTL_INIT_FLUSH_PULSE:
	{
		mtctl_initflush_data_t* data = (mtctl_initflush_data_t*) kap->data;
		mt_controller_thread = act;
		mt_flush_evt_channel = data->channel;
		break;
	}

	/* Initializes and start the tracelogger */
	case _MT_CTL_INIT_TRACELOGGER:
	{
		int status;
		unsigned i;
		part_id_t mpid;
		mt_data_ctrl_t *ptbuf;
		mtctl_inittracelogger_data_t* data = (mtctl_inittracelogger_data_t*) kap->data;

		/* Initializes the shared memory */
		status = memmgr.mmap(NULL, 0, _MT_ALLOC_SIZE, PROT_WRITE | PROT_READ,
				MAP_PHYS | MAP_PRIVATE | MAP_ANON, NULL, 0, 0, 0, NOFD,
				(void *) &ptbuf, &i, mpid = mempart_getid(NULL, sys_memclass_id));
		if (status != EOK || MAP_FAILED == (void *) ptbuf)
			return (ENOMEM);

		mt_buffers_init(ptbuf);
		mt_tracebuf_addr = (uintptr_t) ptbuf;

		/* Traces the currently existing tasks information */
		mt_list_task_info();

		/* returning address to userspace */
		(void) memmgr.vaddrinfo(aspaces_prp[KERNCPU], mt_tracebuf_addr,
				data->shared_memory, NULL, VI_NORMAL);

		/* toggle debug tracing filter */
		mt_filter_tracing_debug((char) data->filter);
		break;
	}

	/* Stops the logger and free the allocated memory */
	case _MT_CTL_TERMINATE_TRACELOGGER:
	{
		if (mt_tracebuf_addr)
		{
			if (!kerisroot(act))
				return (EPERM);

			lock_kernel();

			/////////////////////////////////////////////////////
			////////////////////  FIX_ME!!  /////////////////////
			// There is a danger that deallocated buffers might
			// be in use.
			/////////////////////////////////////////////////////

			(void) memmgr.munmap(NULL, mt_tracebuf_addr, (_MT_ALLOC_SIZE), 0, mempart_getid(NULL, sys_memclass_id));
			mt_tracebuf_addr = (uintptr_t) NULL;

			unlock_kernel();
		}
		break;
	}

	/* For tests only */
	case _MT_CTL_DUMMY:
	{
		PROCESS* pProcess;
		THREAD* pThread;
		int iProc, iThread;

		for (iProc=1; iProc<process_vector.nentries; iProc++)
			if (VECP(pProcess, &process_vector, iProc))
			{
				kprintf("Process %d : %s\n", iProc, pProcess->debug_name);
/*
				for (iThread=1; iThread<pProcess->threads.nentries; iThread++)
					if (VECP(pThread, &pProcess->threads, iThread))
					{
						kprintf("   Task %d : %s\n", iProc, pThread);
					}
*/			}
	}
	}

	return EOK;
}

__SRCVERSION("ker_mt.c $Rev: 153052 $");
