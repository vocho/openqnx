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
#include "apm.h"
#include <unistd.h>

struct module_entry {
	void		(*init)(unsigned, unsigned);
	unsigned	prio;
};

void
module_init(unsigned pass) {
	extern struct module_entry	module_start[];
	extern struct module_entry	module_end[];
	struct module_entry			*mod;

	for(mod = module_start; mod < module_end; ++mod) {
		if(mod->init != NULL) {
			mod->init(LIBMOD_VERSION_CHECK, pass);
		}
	}
}


static unsigned
getmem(char *arg) {
	unsigned	size;

	size = strtoul(arg, &arg, 0);
	switch(*arg) {
	case 'k':	
	case 'K':	
		size *= KILO(1);
		break;
	case 'm':	
	case 'M':	
		size *= MEG(1);
		break;
	default:
		break;
	}
	return size;
}

void
kernel_main(int argc, char *argv[], char *env[]) {
	int					i;
	int					n;
	char				*cp;
	char				*mem_config;
	size_t				pregrow = KILO(64);

	module_init(0);
	for(i = 0; i < NUM_PROCESSORS; ++i) {
		if((SYSPAGE_ENTRY(cpuinfo)[i].flags & CPU_FLAG_FPU) == 0) {
			fpuemul |= 2;
		}
	}
	mem_config = "";
	while ((i = getopt(argc, argv, "a:cf:T:F:m:pP:hl:R:M:ve:u:H:")) != -1) {
		switch(i) {
		case 'u':
			procfs_umask = strtoul( optarg, NULL, 0 );
			break;
		case 'a':
		    switch( optarg[0] ) {
			case 'd':
			    align_fault = 1;
			    break;
			case 'e':
			    align_fault = -1;
			    break;
			case 's':
			    align_fault = 0;
			    break;
			default:
			    break;
			}
			break;
		
		case 'c':
			intrs_aps_critical = 0;
			break;

		case 'p':
			nopreempt = 1;
			break;

		case 'h':
			nohalt = 1;
			break;

		case 'f':
			cp = optarg;
			if(*cp == 'e')
				fpuemul |= 1;
			break;
		case 'm':
			mem_config = optarg;
			break;

		case 'l':
			// -l PROCESSES,THREADS,TIMERS,PULSES,SYNCS,CONNECTS,CHANNELS,INTERRUPTS
			for(cp = optarg, i = 0 ; *cp ; ++i) {
				unsigned	lim;

				lim = strtoul(cp, &cp, 0);
				if(lim != 0) limits_max[i] = lim;
				if(*cp == ',') ++cp;
			}
			break;

		case 'T':
			fd_close_timeout = strtoul(optarg, NULL, 0);
			if(fd_close_timeout == 0) fd_close_timeout = 30;
			break;

		case 'F':
			max_fds = strtoul(optarg, NULL, 0);
			if(max_fds < 100) max_fds = 100;
			break;

		case 'P':
			priv_prio = strtoul(optarg, NULL, 0);
			if(priv_prio < 10) priv_prio = 10;
			if(priv_prio > NUM_PRI) priv_prio = NUM_PRI;
			break;

		case 'R':
		/*
		 * reserved memory size (guarantee) for the sysram memory class in the
		 * system partition. It would have bee nice to make this a memory ('m')
		 * option, but currently that mechanism was designed mainly for flags
		 * so using the '-R' option since this is consistent with startup.
		 * The value specified here will be sanitized and take effect when
		 * MEMPART_INIT() is called.
		 * This option is only effective if the memory partition module is
		 * included in the build otherwise any optional value will be ignored
		*/
		{
			sys_mempart_sysram_min_size = getmem(optarg);

			if (ker_verbose) {
				kprintf("system partition reserved = 0x%x bytes\n", sys_mempart_sysram_min_size);
			}
			break;
		}
		case 'M':
		{
			/*
			 * mempart_flags_t setting for default_mempart_flags.
			 * This option is independent or the partitioning module
			*/
			mempart_dcmd_flags_t  tmp = strtoul(optarg, NULL, 0);
				
			if ((tmp & (mempart_flags_HEAP_CREATE | mempart_flags_HEAP_SHARE)) == 0) {
				kprintf("Illegal -M option to procnto\n"
						"Must contain mempart_flags_HEAP_CREATE and/or mempart_flags_HEAP_SHARE\n");
			} else {
				default_mempart_flags = tmp;
			}

			if (ker_verbose) {
				kprintf("MEMPART options flags = 0x%x\n", default_mempart_flags);
			}
			break;
		}

		case 'v':
			ker_verbose++;
			break;

		case 'e':
			switch(optarg[0]) {
			case 'n':
				SYSPAGE_ENTRY(system_private)->private_flags |= SYSTEM_PRIVATE_FLAG_EALREADY_NEW;
				break;

			case 'o':
			default:
				SYSPAGE_ENTRY(system_private)->private_flags &= ~SYSTEM_PRIVATE_FLAG_EALREADY_NEW;
				break;
			}
			break;

		case 'H':
			pregrow = getmem(optarg);
			if(pregrow < KILO(1)) pregrow *= KILO(1);
			break;

		default:
			break;
		}
	}

	if(fpuemul) {
		for(i = 0; i < NUM_PROCESSORS; ++i) {
			SYSPAGE_ENTRY(cpuinfo)[i].flags &= ~CPU_FLAG_FPU;
		}
	}

	if (SYSPAGE_ENTRY(system_private)->private_flags & SYSTEM_PRIVATE_FLAG_EALREADY_NEW) {
		__ealready_value = EALREADY_NEW;
	} else {
		__ealready_value = EALREADY_OLD;
	}

	init_memmgr();
	memmgr.configure(mem_config);
	mdriver_check();
	init_traps();
	mdriver_check();
	memmgr.init_mem(0);
	mdriver_check();
	heap_init(pregrow);
	mdriver_check();
	thread_init();
	mdriver_check();
	module_init(1);
	mdriver_check();
	init_objects();
	mdriver_check();
	intrevent_init(0);
	mdriver_check();
	timer_init();
	mdriver_check();
	interrupt_init();
	mdriver_check();
	init_cpu();

	// Remaining arguments are for Proc, save them in malloc'd storage
	argc -= (optind - 1);
	argv += (optind - 1);
	_argv = _smalloc((argc+1) * sizeof(*argv));
	for(i = 1; i < argc; ++i) {
		_argv[i] = strdup(argv[i]);
	}
	_argv[0] = strdup(argv[1-optind]);
	_argv[i] = NULL;
	_argc = argc;
	for(n = 0; env[n] != NULL; ++n) {
		//nothing to do
	}
	environ = _smalloc((n+1) * sizeof(*environ));
	for(i = 0; i < n; ++i) {
		environ[i] = strdup(env[i]);
	}
	environ[i] = NULL;
	optind = -1;

	// This will return through the kernel and idle will be running!

	mdriver_check();
	set_inkernel(0);
	ker_start();
}

__SRCVERSION("kmain.c $Rev: 168289 $");
