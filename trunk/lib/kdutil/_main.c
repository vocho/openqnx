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

#include "kdintl.h"
#include <kernel/nto.h>
#include <sys/image.h>
#include <sys/startup.h>

extern int main(int, char**, char **);

#define STACK_SIZE	8192

struct syspage_entry					*_syspage_ptr;
struct cpupage_entry					*_cpupage_ptr;
struct system_private_entry				*private;
int										_argc;
char									**_argv;
char									**environ;
int										_multi_threaded;
unsigned 								__cpu_flags;
volatile unsigned long					*__shadow_imask; // for MIPS, SH

uint64_t 		_stack_base[STACK_SIZE/sizeof(uint64_t)] = { __STACK_SIG };
uint64_t		*_stack_top = &_stack_base[(STACK_SIZE-STACK_INITIAL_CALL_CONVENTION_USAGE)/sizeof(uint64_t)];


struct ifs_bootstrap_data	bootstrap = {
	sizeof(bootstrap), 0, (uintptr_t)_stack_base
};

void
_main(struct syspage_entry *sysp) {
	char					*argv [10], *envv [10];
	int						i, argc, envc;
	char					*args;
	struct bootargs_entry	*boot_args = (void *)_stack_base;

	_syspage_ptr = sysp;
	private = SYSPAGE_ENTRY(system_private);
	_cpupage_ptr = private->kern_cpupageptr;
	__cpu_flags = SYSPAGE_ENTRY(cpuinfo)->flags;

#if defined(__MIPS__)
		__shadow_imask = &_syspage_ptr->un.mips.shadow_imask;
#elif defined(__SH__)
		__shadow_imask = &_syspage_ptr->un.sh.imask;
#endif

	argc = envc = 0;
	args = boot_args->args;
	for(i = 0; i < boot_args->argc; ++i) {
		if(i < sizeof argv / sizeof *argv) argv [argc++] = args;
		while (*args++) ;
	}
	argv[argc] = 0;

	for(i = 0; i < boot_args->envc; ++i) {
		if (i < sizeof envv / sizeof *envv) envv [envc++] = args;
		while (*args++) ;
	}
	envv[envc] = 0;

	main(_argc = argc, _argv = argv, environ = envv);
}


uintptr_t
next_bootstrap(void) {
	uintptr_t	start_vaddr;

	start_vaddr = bootstrap.next_entry;
	if(start_vaddr == 0) {
		start_vaddr = private->boot_pgm[++(private->boot_idx)].entry;
	}
	return start_vaddr;
}


void
_exit(int status) {
	for(;;);
}
