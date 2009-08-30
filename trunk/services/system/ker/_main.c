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
#include <sys/image.h>

char									*envv[16];
char									*argv[16];
int										_Multi_threaded;

struct ifs_bootstrap_data	bootstrap = { 
	sizeof(bootstrap), 0, (uintptr_t)startup_stack
};

static void
syspage_init() {
	struct callin_entry		*callin;
	unsigned				i;
	struct cpupage_entry	*cpupage;
	struct kdebug_callback *kdcall;

	_syspage_ptr->num_cpu = num_processors = min(PROCESSORS_MAX, _syspage_ptr->num_cpu);

	__cpu_flags = SYSPAGE_ENTRY(cpuinfo)->flags;

	privateptr = SYSPAGE_ENTRY(system_private);

	if((kdcall = privateptr->kdebug_call) != NULL) {
		kdextra = kdcall->extra;
	}

	_cpupage_ptr = cpupage = privateptr->kern_cpupageptr;
	cpupage->tls = &intr_tls;
	for(i = 0; i < NUM_PROCESSORS; ++i) {
		cpupageptr[i] = cpupage;
		cpupage = (void *)((uint8_t *)cpupage + privateptr->cpupage_spacing);
	}

	kercallptr = &_SYSPAGE_ENTRY(privateptr->user_syspageptr, system_private)->kercall;

	qtimeptr = SYSPAGE_ENTRY(qtime);

	intrinfoptr = SYSPAGE_ENTRY(intrinfo);
	intrinfo_num = _syspage_ptr->intrinfo.entry_size / sizeof(*intrinfoptr);

	calloutptr = SYSPAGE_ENTRY(callout);
	callout_timer_value = calloutptr->timer_value;

	callin = SYSPAGE_ENTRY(callin);
	callin->trace_event = outside_trace_event;
	callin->interrupt_mask = outside_intr_mask;
	callin->interrupt_unmask = outside_intr_unmask;

	cpu_syspage_init();
}

void
_main(struct syspage_entry *sysp) {
	int							i, argc, envc;
	char						*args;
	struct bootargs_entry		*bap = (void *)startup_stack;

	_syspage_ptr = sysp;

	mdriver_init();

	syspage_init();

	set_inkernel(INKERNEL_NOW|INKERNEL_LOCK);

	argc = envc = 0;
	args = bap->args;
	for(i = 0; i < bap->argc; ++i) {
		if(i < sizeof argv / sizeof *argv) argv[argc++] = args;
		while(*args++) ;
	}
	argv[argc] = 0;

	for(i = 0; i < bap->envc; ++i) {
		if(i < sizeof envv / sizeof *envv) envv[envc++] = args;
		while(*args++) ;
	}
	envv[envc] = 0;

  // allow override of the tick multiplier.
  if (qtimeptr->rr_interval_mul != 0) {
  	rr_interval_mul = qtimeptr->rr_interval_mul;
  }

	kernel_main(argc, argv, envv);
}

void
__exit(int status) {
	(void)ThreadDestroy_r(0, -1, (void *)status);
#ifdef __GNUC__
	for(;;) {
		/* nothing to do */
	}
#endif
}

void
_exit(int status) {
	__exit(status);
#ifdef __GNUC__
	for(;;) {
		/* nothing to do */
	}
#endif
}

void
exit(int status) {
	_exit(status);
#ifdef __GNUC__
	for(;;) {
		/* nothing to do */
	}
#endif
}

__SRCVERSION("_main.c $Rev: 153052 $");
