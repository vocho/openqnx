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


void
kerext_soul_compact(unsigned *n) {
	SOUL		*soulp;
	THREAD		*act = actives[KERNCPU];
	unsigned	index = *n;
	
	if((soulp = vector_search(&soul_vector, index, &index)) == NULL) {
		lock_kernel();
		SETKSTATUS(act,-1);
		return;
	}

	lock_kernel();
	object_compact(soulp);
	SETKSTATUS(act,index);
}


void *
start_kernel_threads(void *dummy) {
	extern void main(int argc, char *argv[], char *env[]);

#ifdef VARIANT_smp
	int i;
	uint64_t min_cycle = ULLONG_MAX;

	for ( i = 0; i < NUM_PROCESSORS; i++ ) {
		if ( clockcycles_offset[i] < min_cycle ) {
			min_cycle = clockcycles_offset[i];
		}
	}
	for ( i = 0; i < NUM_PROCESSORS; i++ ) {
		clockcycles_offset[i]  -= min_cycle;
	}
#endif
	clock_start(SYSPAGE_ENTRY(cpuinfo)->speed <= _NTO_SLOW_MHZ ?
			_NTO_TICKSIZE_SLOW : _NTO_TICKSIZE_FAST);

	module_init(5);		// currently the 2nd last init pass
	main(_argc, _argv, environ);
	(void)ThreadDestroy_r(0, -1, 0);
	return NULL;
}

#if !defined(SHOW_IDLE) && defined(VARIANT_smp) && !defined(NDEBUG)
#define SHOW_IDLE
#endif

#if defined(__WATCOMC__)
#pragma aux loop aborts;
#endif

static void
loop(void) {
#if defined(SHOW_IDLE) && defined(__X86__)
	unsigned i;
	unsigned col = 0;
	char *cp = _syspage_ptr->un.x86.real_addr;
#endif

	for(;;) {

#if defined(SHOW_IDLE) && defined(__X86__)
		col = (col + 1) % 75;
		if(ker_verbose > 2) {
			for(i = 100000; i ; --i) {
				++cp[0xb8000 + 2*(col+5) + RUNCPU*160];
				if(i > 100000) DebugKDBreak();
			}
		}
#endif
	
		if(!nohalt) {
			__Ring0(kerext_idle, interrupt_level[HOOK_TO_LEVEL(_NTO_HOOK_IDLE)].queue);
		}
	}
}

struct idle_stack {
	uint64_t stack[(IDLE_STACK_NBYTES+sizeof(struct _thread_local_storage)) / sizeof(uint64_t)];
};

void idle_release_stack(void* data) {
	THREAD 							*act = (THREAD*)data;
	struct _thread_local_storage	*tls;
	struct idle_stack				*isp;
	uintptr_t						stack_top;
	uintptr_t						new_sp;
	void							*dummy;

	lock_kernel();
	
	isp = _scalloc(sizeof(*isp));

	/* load new stack pointer */
	stack_top = STACK_INIT((uintptr_t)isp, sizeof(*isp));
	STACK_ALLOC(tls, new_sp, stack_top, sizeof(struct _thread_local_storage));
	STACK_ALLOC(dummy, new_sp, new_sp, STACK_INITIAL_CALL_CONVENTION_USAGE);
	SETKSP(act, new_sp);
	SETKIP_FUNC(act, loop);
	tls->__flags = PTHREAD_CANCEL_DISABLE;
	tls->__errptr = &tls->__errval;

	_sfree(act->un.lcl.stackaddr, act->un.lcl.stacksize);
	act->un.lcl.tls = tls;
	act->un.lcl.stackaddr = (void *)isp;
	act->un.lcl.stacksize = sizeof(*isp);
}

void
idle(void) {
#if defined(VARIANT_smp)
	static volatile unsigned cpus_started;
#endif
	THREAD				*act;
	struct sched_param	param;
	unsigned			cpu;

	// Can't use KERNCPU here because we're not in the kernel
	cpu = RUNCPU;

	alives[cpu] = 1;
	act = actives[cpu];

	if(cpu == 0) {
		// Initialize process manager local address space
		(void)memmgr.mcreate(act->process);
	}

	/*
	 * Give memory manager a chance to adjust the address space (e.g. SMP)
	 */
	ProcessBind(SYSMGR_PID);
	ProcessBind(0);

#if defined(VARIANT_smp)
	if(++cpus_started < NUM_PROCESSORS) {
		cpu_start_ap((uintptr_t)_smpstart);
		do {
			// Don't let anything happen until we've got everybody up
		} while(cpus_started < NUM_PROCESSORS);
		clockcycles_offset[cpu] = ClockCycles();
	} else
#endif
	{
#if defined(VARIANT_smp)
		clockcycles_offset[cpu] = ClockCycles();
#endif
		mdriver_process_time();
		cpu_start_ap(~0L);	/* Tell all remaining AP's to shut down */

		_sfree(startup_stack, sizeof(startup_stack));

		(void)ThreadCreate_r(0, start_kernel_threads, NULL, main_attrp);
	}

	// Reduce priority to lowest in the system.
	memset(&param, 0, sizeof(param));
	(void)SchedSet_r(0, 0, SCHED_FIFO, &param);

	/* this function will not return */
	__Ring0(idle_release_stack, act);

	crash();
}

__SRCVERSION("idle.c $Rev: 165879 $");
