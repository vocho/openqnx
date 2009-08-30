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




#include <stdlib.h>
#include <process.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <ucontext.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <sys/kdebug.h>
#include "gmon.h"
#include "init.h"

static unsigned short 	*counters;
static int				nelems;
static unsigned			low;
static unsigned			scale;

static pid_t			mypid;
static int				irq_id;

/*
 * This file includes the kernel structures for now; we need to change 
 * this to use only the public debug/kdebug headers.
 */
static const struct sigevent *handler(void *data, int id) {
	const struct kdebug_private	*kpp;
	unsigned					ip;
	void						*thread;
	void						*process;
	int							cpunum = 0;

	kpp = SYSPAGE_ENTRY(system_private)->kdebug_info->kdbg_private;
	thread = ((char **)(kpp->actives))[cpunum];
	process = *((void **)((char *)thread + kpp->th_process_off));
	
	ip = 0;
	if(mypid == *((pid_t *)((char *)process + kpp->pr_pid_off))) {
		ip = GET_REGIP(&((mcontext_t *)((char *)thread + kpp->th_reg_off))->cpu);
	}

	// See if IP in range that we are profiling
	if(ip >= low && ip < (low + nelems * scale)) {
		if(counters[(ip - low) / scale] < USHRT_MAX)
			counters[(ip - low) / scale]++;
	}

	return NULL;
}
		

// Turns profiling on or off
int profil(char *buffer, size_t size, unsigned lowpc, unsigned histfraction) {
	struct qtime_entry		*qtp;

	// Stuff _syspage, as this may get called from init before the libc
	// init has been done.
	_syspage_ptr = __SysCpupageGet(CPUPAGE_SYSPAGE);

	qtp = SYSPAGE_ENTRY(qtime);

	if(buffer) {
		// Start up the profiling; set params, attach irq
		counters = (unsigned short *)buffer;
		nelems = size / sizeof(unsigned short);
		low = lowpc;
		scale = histfraction * sizeof(unsigned short);
		mypid = getpid();

		// Touch buffer to make sure it is accessible
		memset(buffer, 0, nelems * sizeof(unsigned short));

		// Need I/O Priv
		(void)ThreadCtl(_NTO_TCTL_IO, 0);
		irq_id = InterruptAttach(qtp->intr, handler, NULL, 0, _NTO_INTR_FLAGS_PROCESS);
	} else {
		(void)InterruptDetach(irq_id);
	}
	return 0;
}

__SRCVERSION("sample.c $Rev: 153052 $");
