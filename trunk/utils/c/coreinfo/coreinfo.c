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





#ifdef __QNXNTO__
#define ELF_TARGET_ALL
#define SYSPAGE_TARGET_ALL
#define _DEBUG_TARGET_ALL
#endif

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>
#ifdef __QNXNTO__
#include <sys/procfs.h>
#include <sys/neutrino.h>
#endif

#include _NTO_HDR_(sys/elf.h)
#include _NTO_HDR_(sys/elf_notes.h)

#if _NTO_VERSION >= 640
#define BITS __bits
#else
#define BITS bits
#endif

#ifdef __USAGE
%C - display information on a neutrino core file
#endif

static int verbose;

static void swap_16(void *ptr) {
	unsigned char tmp, *p = ptr;

	tmp = p[0];
	p[0] = p[1];
	p[1] = tmp;
}

static void swap_32(void *ptr) {
	unsigned char tmp, *p = ptr;

	tmp = p[0];
	p[0] = p[3];
	p[3] = tmp;
	tmp = p[1];
	p[1] = p[2];
	p[2] = tmp;
}

static void swap_64(void *ptr) {
	unsigned char tmp, *p = ptr;

	tmp = p[0];
	p[0] = p[7];
	p[7] = tmp;
	tmp = p[1];
	p[1] = p[6];
	p[6] = tmp;
	tmp = p[2];
	p[2] = p[5];
	p[5] = tmp;
	tmp = p[3];
	p[3] = p[4];
	p[4] = tmp;
}

static void swap_Elf32_Ehdr(Elf32_Ehdr *ehdr) {
	swap_16(&ehdr->e_type);
	swap_16(&ehdr->e_machine);
	swap_32(&ehdr->e_version);
	swap_32(&ehdr->e_entry);
	swap_32(&ehdr->e_phoff);
	swap_32(&ehdr->e_shoff);
	swap_32(&ehdr->e_flags);
	swap_16(&ehdr->e_ehsize);
	swap_16(&ehdr->e_phentsize);
	swap_16(&ehdr->e_phnum);
	swap_16(&ehdr->e_shentsize);
	swap_16(&ehdr->e_shnum);
	swap_16(&ehdr->e_shstrndx);
}

static void swap_Elf32_Phdr(Elf32_Phdr *phdr) {
	swap_32(&phdr->p_type);
	swap_32(&phdr->p_offset);
	swap_32(&phdr->p_vaddr);
	swap_32(&phdr->p_paddr);
	swap_32(&phdr->p_filesz);
	swap_32(&phdr->p_memsz);
	swap_32(&phdr->p_flags);
	swap_32(&phdr->p_align);
}

static void dsp_index(const char *strv[], unsigned max, int index) {
	if(index < 0 || index >= max || !strv[index]) {
		printf("Unknown(%d)", index);
	} else {
		printf("%s", strv[index]);
	}
}

#ifdef __QNXNTO__
static void swap_Elf32_Nhdr(Elf32_Nhdr *nhdr) {
	swap_32(&nhdr->n_namesz);
	swap_32(&nhdr->n_descsz);
	swap_32(&nhdr->n_type);
}

static void swap_procfs_sysinfo(procfs_sysinfo *sysinfo) {
	swap_16(&sysinfo->size);
	swap_16(&sysinfo->total_size);
	swap_16(&sysinfo->type);
	swap_16(&sysinfo->num_cpu);
	swap_16(&sysinfo->system_private.entry_off);
	swap_16(&sysinfo->system_private.entry_size);
	swap_16(&sysinfo->asinfo.entry_off);
	swap_16(&sysinfo->asinfo.entry_size);
	swap_16(&sysinfo->meminfo.entry_off);
	swap_16(&sysinfo->meminfo.entry_size);
	swap_16(&sysinfo->hwinfo.entry_off);
	swap_16(&sysinfo->hwinfo.entry_size);
	swap_16(&sysinfo->cpuinfo.entry_off);
	swap_16(&sysinfo->cpuinfo.entry_size);
	swap_16(&sysinfo->cacheattr.entry_off);
	swap_16(&sysinfo->cacheattr.entry_size);
	swap_16(&sysinfo->qtime.entry_off);
	swap_16(&sysinfo->qtime.entry_size);
	swap_16(&sysinfo->callout.entry_off);
	swap_16(&sysinfo->callout.entry_size);
	swap_16(&sysinfo->callin.entry_off);
	swap_16(&sysinfo->callin.entry_size);
	swap_16(&sysinfo->typed_strings.entry_off);
	swap_16(&sysinfo->typed_strings.entry_size);
	swap_16(&sysinfo->strings.entry_off);
	swap_16(&sysinfo->strings.entry_size);
	swap_16(&sysinfo->intrinfo.entry_off);
	swap_16(&sysinfo->intrinfo.entry_size);
	swap_16(&sysinfo->smp.entry_off);
	swap_16(&sysinfo->smp.entry_size);
	swap_16(&sysinfo->pminfo.entry_off);
	swap_16(&sysinfo->pminfo.entry_size);
	swap_16(&sysinfo->mdriver.entry_off);
	swap_16(&sysinfo->mdriver.entry_size);
}

static void swap_cpuinfo_entry(struct cpuinfo_entry *cpuinfo) {
	swap_32(&cpuinfo->cpu);
	swap_32(&cpuinfo->speed);
	swap_32(&cpuinfo->flags);
	swap_16(&cpuinfo->name);
}

static void swap_qtime_entry(struct qtime_entry *qtime) {
	swap_64(&qtime->cycles_per_sec);
	swap_64((_Uint64t *)&qtime->nsec_tod_adjust);
	swap_64((_Uint64t *)&qtime->nsec);
	swap_32(&qtime->nsec_inc);
	swap_32(&qtime->boot_time);
	swap_32(&qtime->adjust.tick_count);
	swap_32(&qtime->adjust.tick_nsec_inc);
	swap_32(&qtime->timer_rate);
	swap_32(&qtime->timer_scale);
	swap_32(&qtime->timer_load);
	swap_32(&qtime->intr);
	swap_32(&qtime->epoch);
	swap_32(&qtime->flags);
	swap_32(&qtime->rr_interval_mul);
}

static void swap_procfs_info(procfs_info *info) {
	swap_32(&info->pid);
	swap_32(&info->parent);
	swap_32(&info->flags);
	swap_32(&info->umask);
	swap_32(&info->child);
	swap_32(&info->sibling);
	swap_32(&info->pgrp);
	swap_32(&info->sid);
	swap_64(&info->base_address);
	swap_64(&info->initial_stack);
	swap_32(&info->uid);
	swap_32(&info->gid);
	swap_32(&info->euid);
	swap_32(&info->egid);
	swap_32(&info->suid);
	swap_32(&info->sgid);
	swap_32(&info->sig_ignore);
	swap_32(&info->sig_queue);
	swap_32(&info->sig_pending);
	swap_32(&info->num_chancons);
	swap_32(&info->num_fdcons);
	swap_32(&info->num_threads);
	swap_32(&info->num_timers);
	swap_64(&info->start_time);		 
	swap_64(&info->utime);			 
	swap_64(&info->stime);			 
	swap_64(&info->cutime);			 
	swap_64(&info->cstime);			 
	swap_64(&info->canstub);			 
	swap_64(&info->sigstub);			 
}

static void swap_procfs_status(procfs_status *status) {
	size_t off = offsetof(procfs_status, sig_blocked); /* changed to include
		full structures within .sig_blocked, .sig_pending and .info */
	int *start = (int *)( (char *)status + off), *finish = (int *)&status->start_time;
	while(start < finish){
			swap_32(start);
			start++;
	}
	swap_32(&status->pid);
	swap_32(&status->tid);
	swap_32(&status->flags);
	swap_16(&status->why);
	swap_16(&status->what);
	swap_64(&status->ip);
	swap_64(&status->sp);
	swap_64(&status->stkbase);
	swap_64(&status->tls);
	swap_32(&status->stksize);
	swap_32(&status->tid_flags);
	swap_32(&status->priority);
	swap_32(&status->real_priority);
	swap_32(&status->policy);
	swap_32(&status->state);
	swap_16(&status->syscall);
	swap_16(&status->last_cpu);
	swap_32(&status->timeout);
	swap_32(&status->last_chid);
	swap_64(&status->start_time);		 
	swap_64(&status->sutime);			 
}

static void swap_procfs_greg(procfs_greg *greg, int cpu) {
	fprintf(stderr, "byteswapping of gpregs not supported yet\n");
}

static void swap_procfs_fpreg(procfs_fpreg *fpreg, int cpu) {
	fprintf(stderr, "byteswapping of fpregs not supported yet\n");
}

static void print_sysinfo(procfs_sysinfo *p, int xlat) {
	static const char			*cpu[] = {
		"X86", "PPC", "MIPS", "unknown(3)", "ARM", "SH"
	};

	if(verbose) {
		printf(" size=%d total_size=%d", p->size, p->total_size);
		printf(" system_private:off/size=%d,%d", p->system_private.entry_off, p->system_private.entry_size);
		printf("\n");
		printf(" asinfo:off/size=%d,%d", p->asinfo.entry_off, p->asinfo.entry_size);
		printf(" meminfo:off/size=%d,%d", p->meminfo.entry_off, p->meminfo.entry_size);
		printf(" hwinfo:off/size=%d,%d", p->hwinfo.entry_off, p->hwinfo.entry_size);
		printf("\n");
		printf(" cpuinfo:off/size=%d,%d", p->cpuinfo.entry_off, p->cpuinfo.entry_size);
		printf(" cacheattr:off/size=%d,%d", p->cacheattr.entry_off, p->cacheattr.entry_size);
		printf(" qtime:off/size=%d,%d", p->qtime.entry_off, p->qtime.entry_size);
		printf("\n");
		printf(" callout:off/size=%d,%d", p->callout.entry_off, p->callout.entry_size);
		printf(" callin:off/size=%d,%d", p->callin.entry_off, p->callin.entry_size);
		printf(" intrinfo:off/size=%d,%d", p->intrinfo.entry_off, p->intrinfo.entry_size);
		printf("\n");
		printf(" typed_strings:off/size=%d,%d", p->typed_strings.entry_off, p->typed_strings.entry_size);
		printf(" strings:off/size=%d,%d", p->strings.entry_off, p->strings.entry_size);
		printf("\n");
	}
	printf(" processor=");
	dsp_index(cpu, sizeof cpu / sizeof *cpu, p->type);
	printf(" num_cpus=%d", p->num_cpu);
	printf("\n");
	if(p->cpuinfo.entry_off + p->cpuinfo.entry_size <= p->total_size) {
		int							i;
		char						*strings;
		p->strings.entry_off = p->strings.entry_off; 
		p->strings.entry_size = p->strings.entry_size;
		strings = (p->strings.entry_off + p->strings.entry_size <= p->total_size) ?
					_SYSPAGE_ENTRY(p, strings)->data : 0;

		for(i = 0; strings && i < p->num_cpu; i++) {
			struct cpuinfo_entry	*c = _SYSPAGE_ENTRY(p, cpuinfo) + i;
			
			if (xlat) {
				swap_cpuinfo_entry(c);
			}
			
			printf("  cpu %d cpu=%d name=%s speed=%d\n", i + 1, c->cpu, strings ? strings + c->name : "", c->speed);
			printf("   flags=%#08x", c->flags);
			if(c->flags & CPU_FLAG_FPU) {
				printf(" FPU");
			}
			if(c->flags & CPU_FLAG_MMU) {
				printf(" MMU");
			}
			switch(p->type) {
			case SYSPAGE_X86:
				if(c->flags & X86_CPU_CPUID) {
					printf(" CPUID");
				}
				if(c->flags & X86_CPU_RDTSC) {
					printf(" RDTSC");
				}
				if(c->flags & X86_CPU_INVLPG) {
					printf(" INVLPG");
				}
				if(c->flags & X86_CPU_WP) {
					printf(" WP");
				}
				if(c->flags & X86_CPU_BSWAP) {
					printf(" BSWAP");
				}
				if(c->flags & X86_CPU_MMX) {
					printf(" MMX");
				}
				if(c->flags & X86_CPU_CMOV) {
					printf(" CMOV");
				}
				if(c->flags & X86_CPU_PSE) {
					printf(" PSE");
				}
				if(c->flags & X86_CPU_PGE) {
					printf(" PGE");
				}
				if(c->flags & X86_CPU_MTRR) {
					printf(" MTRR");
				}
				if(c->flags & X86_CPU_SEP) {
					printf(" SEP");
				}
				if(c->flags & X86_CPU_SIMD) {
					printf(" SIMD");
				}
				if(c->flags & X86_CPU_FXSR) {
					printf(" FXSR");
				}
				break;
			case SYSPAGE_PPC:
				if(c->flags & PPC_CPU_EAR) {
					printf(" EAR");
				}
				if(c->flags & PPC_CPU_HW_HT) {
					printf(" HW_HT");
				}
				if(c->flags & PPC_CPU_HW_POW) {
					printf(" HW_POW");
				}
				if(c->flags & PPC_CPU_FPREGS) {
					printf(" FPREGS");
				}
				if(c->flags & PPC_CPU_SW_HT) {
					printf(" SW_HT");
				}
				if(c->flags & PPC_CPU_ALTIVEC) {
					printf(" ALTIVEC");
				}
				break;
			case SYSPAGE_MIPS:
				break;
			case SYSPAGE_SH:
				break;
			case SYSPAGE_ARM:
				break;
			default:
				break;
			}
			printf("\n");
		}
	}
	if(p->qtime.entry_off + p->qtime.entry_size <= p->total_size) {
		struct qtime_entry	*t = _SYSPAGE_ENTRY(p, qtime);
		
		if (xlat) {
			swap_qtime_entry(t);
		}
		
		printf(" cyc/sec=%llu tod_adj=%llu nsec=%llu inc=%lu\n",
			t->cycles_per_sec, t->nsec_tod_adjust, t->nsec, t->nsec_inc);
		printf(" boot=%lu epoch=%lu intr=%ld\n", t->boot_time, t->epoch, t->intr);
		printf(" rate=%lu scale=%ld load=%lu\n", t->timer_rate, t->timer_scale, t->timer_load);
	}
	if(p->typed_strings.entry_off + p->typed_strings.entry_size <= p->total_size) {
		struct entry {
			uint32_t		type;
			char			info[1];
		}				*ent;
		int				pos;

		pos = 0;
		
		ent = (struct entry *)_SYSPAGE_ENTRY(p, typed_strings)->data;
		if (xlat) {
			swap_32(&ent->type);
		}
			
		while(ent->type != _CS_NONE) {

			char			*type, buff[20];
			int				len;

			switch(ent->type & ~_CS_SET) {
			case _CS_HOSTNAME:
				type = "HOSTNAME";
				break;
			case _CS_RELEASE:
				type = "RELEASE";
				break;
			case _CS_VERSION:
				type = "VERSION";
				break;
			case _CS_MACHINE:
				type = "MACHINE";
				break;
			case _CS_ARCHITECTURE:
				type = "ARCHITECTURE";
				break;
			case _CS_HW_SERIAL:
				type = "HW_SERIAL";
				break;
			case _CS_HW_PROVIDER:
				type = "HW_PROVIDER";
				break;
			case _CS_SRPC_DOMAIN:
				type = "SRPC_DOMAIN";
				break;
			case _CS_SYSNAME:
				type = "SYSNAME";
				break;
			case _CS_LIBPATH:
				type = "LIBPATH";
				break;
			case _CS_DOMAIN:
				type = "DOMAIN";
				break;
			case _CS_RESOLVE:
				type = "RESOLVE";
				break;
			case _CS_TIMEZONE:
				type = "TIMEZONE";
				break;
			default:
				sprintf(type = buff, "type(%d)", ent->type & ~_CS_SET);
				break;
			}

			len = strlen(type) + strlen(ent->info) + 4;		/* space = " " */
			if(pos && pos + len > 75) {
				printf("\n");
				pos = 0;
			}
			if(pos == 0) {
				printf("  ");
			}
			printf(" %s=\"%s\"", type, ent->info);
			pos += len;
			
			ent = (struct entry *)((char *)ent + ((offsetof(struct entry, info) +
						strlen(ent->info) + 1 + 3) & ~3));
			if (xlat) {
				swap_32(&ent->type);
			}
		}
		if(pos) {
			printf("\n");
		}
	}
	switch(p->type) {
	case SYSPAGE_PPC:
		if(p->un.ppc.kerinfo.entry_off + p->un.ppc.kerinfo.entry_size <= p->total_size) {
			struct ppc_kerinfo_entry	*k = _SYSPAGE_CPU_ENTRY(p, ppc, kerinfo);

			if(xlat) {
				swap_32(&k->pretend_cpu);
				swap_32(&k->init_msr);
			}
			printf(" pretend_cpu=%lu init_msr=%lu\n", k->pretend_cpu, k->init_msr);
		}
		break;
	case SYSPAGE_X86:
	case SYSPAGE_MIPS:
	case SYSPAGE_SH:
	case SYSPAGE_ARM:
	default:
		break;
	}
}

static void print_info(procfs_info *p) {
	printf(" pid=%d parent=%d child=%d pgrp=%d sid=%d\n",
		p->pid, p->parent, p->child, p->pgrp, p->sid);
	printf(" flags=%#08x umask=%#o base_addr=%#llx init_stack=%#llx\n",
		p->flags, p->umask, p->base_address, p->initial_stack);
	printf(" ruid=%d euid=%d suid=%d  rgid=%d egid=%d sgid=%d\n",
		p->uid, p->euid, p->suid, p->gid, p->egid, p->sgid);
	printf(" ign=%08lx%08lx queue=%08lx%08lx pending=%08lx%08lx\n",
		p->sig_ignore.BITS[1], p->sig_ignore.BITS[0],
		p->sig_queue.BITS[1], p->sig_queue.BITS[0],
		p->sig_pending.BITS[1], p->sig_pending.BITS[0]);
	printf(" fds=%d threads=%d timers=%d chans=%d\n",
		p->num_fdcons, p->num_threads, p->num_timers, p->num_chancons);
	printf(" canstub=%#llx sigstub=%#llx\n",
		p->canstub, p->sigstub );
}

static void print_status(procfs_status *p) {
	static const char				*sigstrs[] = {
		"unknown(0)",
		"SIGHUP",		"SIGINT",		"SIGQUIT",		"SIGILL",		
		"SIGTRAP",		"SIGABRT",		"SIGEMT",		"SIGFPE",		
		"SIGKILL",		"SIGBUS",		"SIGSEGV",		"SIGSYS",		
		"SIGPIPE",		"SIGALRM",		"SIGTERM",		"SIGUSR1",		
		"SIGUSR2",		"SIGCHLD",		"SIGPWR ",		"SIGWNCH",		
		"SIGURG",		"SIGPOLL",		"SIGSTOP",		"SIGTSTP",		
		"SIGCONT",		"SIGTTIN",		"SIGTTOU",		"SIGVTAL",
		"SIGPROF",		"SIGXCPU",		"SIGXFSZ"
	};
	static const char				*sigsegv_code[] = {
		"unknown(0)", "MAPERR", "ACCERR", "STKERR", "SPERR", "IRQERR"
	};
	static const char				*sigbus_code[] = {
		"unknown(0)", "ADRALN", "ADRERR", "OBJERR"
	};
	static const char				*sigill_code[] = {
		"unknown(0)", "ILLOPC", "ILLOPN", "ILLADR", "ILLTRP",
		"PRVOPC", "PRVREG", "COPROC", "BADSTK"
	};
	static const char				*sigcld_code[] = {
		"unknown(0)", "EXITED", "KILLED", "DUMPED",
		"TRAPPED", "STOPPED", "CONTINUED"
	};
	static const char				*sigfpe_code[] = {
		"unknown(0)", "INTDIV", "INTOVF", "FLTDIV", "FLTOVF", "FLTUND",
		"FLTRES", "FLTINV", "FLTSUB", "NOFPU", "NOMEM"
	};
	static const char				*state[] = {
		"DEAD", "RUNNING", "READY", "STOPPED", "SEND", "RECEIVE", "REPLY",
		"STACK", "WAITTHREAD", "WAITPAGE", "SIGSUSPEND", "SIGWAITINFO",
		"NANOSLEEP", "MUTEX", "CONDVAR", "JOIN", "INTR", "SEM", "WAITCTX"
	};
	static const char				*policy[] = {
		"NOCHANGE", "FIFO", "RR", "OTHER", "SPORATIC"
	};

	printf(" thread %d", p->tid);
	if((p->flags & _DEBUG_FLAG_CURTID) || p->why != _DEBUG_WHY_REQUESTED) {
		switch(p->why) {
		case _DEBUG_WHY_REQUESTED:
			printf(" REQUESTED");
			break;
		case _DEBUG_WHY_SIGNALLED:
			printf(" SIGNALLED-");
			dsp_index(sigstrs, sizeof sigstrs / sizeof *sigstrs, p->info.si_signo);
			goto dsp_siginfo;
		case _DEBUG_WHY_FAULTED:
			printf(" FAULTED-");
			dsp_index(sigstrs, sizeof sigstrs / sizeof *sigstrs, p->info.si_signo);
			goto dsp_siginfo;
		case _DEBUG_WHY_JOBCONTROL:
			printf(" JOBCONTROL-");
			dsp_index(sigstrs, sizeof sigstrs / sizeof *sigstrs, p->info.si_signo);
			goto dsp_siginfo;
		case _DEBUG_WHY_TERMINATED:
			printf(" TERMINATED");
dsp_siginfo:
			printf(" code=%d ", p->info.si_code);
			if(SI_FROMKERNEL(&p->info)) {
				switch(p->info.si_signo) {
				case SIGCHLD:
					dsp_index(sigcld_code, sizeof sigcld_code / sizeof *sigcld_code, p->info.si_code);
					switch(p->info.si_code) {
					case CLD_EXITED:
						printf(" status=%d", p->info.si_status);
						break;
					case CLD_KILLED:
					case CLD_DUMPED:
					case CLD_TRAPPED:
					case CLD_STOPPED:
					case CLD_CONTINUED:
					default:
						break;
					}
					break;
				case SIGSEGV:
					dsp_index(sigsegv_code, sizeof sigsegv_code / sizeof *sigsegv_code, p->info.si_code);
					goto dsp_ref;
				case SIGBUS:
					dsp_index(sigbus_code, sizeof sigbus_code / sizeof *sigbus_code, p->info.si_code);
					goto dsp_ref;
				case SIGTRAP:
dsp_ref:
					printf(" refaddr=%p fltno=%d", p->info.si_addr, p->info.si_fltno);
					break;
				case SIGILL:
					dsp_index(sigill_code, sizeof sigill_code / sizeof *sigill_code, p->info.si_code);
					goto dsp_adr;
				case SIGFPE:
					dsp_index(sigfpe_code, sizeof sigfpe_code / sizeof *sigfpe_code, p->info.si_code);
dsp_adr:
					printf(" addr=%p fltno=%d", p->info.si_addr, p->info.si_fltno);
					break;
				default:
					break;
				}
			} else if(p->info.si_code != SI_NOINFO) {
				printf(" from pid=%d uid=%d value=%d(%p)",
					p->info.si_pid, p->info.si_uid, p->info.si_value.sival_int, p->info.si_value.sival_ptr);
			}
			break;
		case _DEBUG_WHY_CHILD:
			printf(" CHILD");
			break;
		case _DEBUG_WHY_EXEC:
			printf(" EXEC");
			break;
		default:
			printf(" Reason=%d", p->why);
			break;
		}
	}
	printf("\n ");
	printf(" ip=%#llx", p->ip);
	printf(" sp=%#llx stkbase=%#llx stksize=%d\n", p->sp, p->stkbase, p->stksize);
	printf("  state=");
	dsp_index(state, sizeof state / sizeof *state, p->state);
	printf(" flags=%x", p->tid_flags);
	printf(" last_cpu=%d timeout=%#08x\n", p->last_cpu + 1, p->timeout);
	printf("  pri=%d realpri=%d policy=", p->priority, p->real_priority);
	dsp_index(policy, sizeof policy / sizeof *policy, p->policy);
	printf("\n");
	switch(p->state) {
	case STATE_RECEIVE:
		printf("  blocked_chid=%d\n", p->blocked.channel.chid);
		break;
	case STATE_JOIN:		// p->blocked.join.tid
	case STATE_MUTEX:		// p->blocked.sync.id,sync
	case STATE_CONDVAR:		// p->blocked.sync.id,sync
	case STATE_SEM:			// p->blocked.sync.id,sync
	case STATE_SEND:		// p->blocked.connect.nd,pid,coid,chid,scoid
	case STATE_REPLY:		// p->blocked.connect.nd,pid,coid,chid,scoid
	case STATE_WAITPAGE:	// p->blocked.waitpage.pid,vaddr,flags
	case STATE_STACK:		// p->blocked.stack.size
	default:
		break;
	}
}

#endif

static void print_pt_load(FILE *fp, Elf32_Phdr *phdr) {
	static const char *phdr_flags[] = {
		"---",
		"--X",
		"-W-",
		"-WX",
		"R--",
		"R-X",
		"RW-",
		"RWX"
	};

	printf(" Mapping %#.8x-%#.8x ", phdr->p_vaddr, phdr->p_vaddr + phdr->p_memsz);
	dsp_index(phdr_flags, sizeof phdr_flags / sizeof *phdr_flags, phdr->p_flags);
	printf("\n");
}

static void dsp_file(FILE *fp, char *name) {
	int						i;
	int						xlat;
	Elf32_Ehdr				ehdr;

	if(fread(&ehdr, sizeof ehdr, 1, fp) != 1) {
		fprintf(stderr, "Unable to read ehdr\n");
		return;
	}

	if(memcmp(ehdr.e_ident, ELFMAG, SELFMAG)) {
		fprintf(stderr, "Not in ELF format\n");
		return;
	}

	switch(ehdr.e_ident[EI_DATA]) {
#if defined(__X86__) || defined(__LITTLEENDIAN__)
	case ELFDATA2LSB:
		xlat = 0;
		break;
	case ELFDATA2MSB:
		xlat = 1;
		break;
#elif defined(__BIGENDIAN__)
	case ELFDATA2LSB:
		xlat = 1;
		break;
	case ELFDATA2MSB:
		xlat = 0;
		break;
#else
	#error Host system endianness not configured
#endif
	default:
		fprintf(stderr, "Unknown endian format\n");
		return;
	}

	if(xlat) {
		swap_Elf32_Ehdr(&ehdr);
	}
	if(ehdr.e_ehsize < sizeof ehdr) {
		fprintf(stderr, "Ehdr is too small\n");
		return;
	}

	if(ehdr.e_type != ET_CORE) {
		fprintf(stderr, "%s is not a core file!\n", name);
	}

	if(ehdr.e_phnum) {
		Elf32_Phdr				phdr;
		
		if(ehdr.e_phentsize < sizeof phdr) {
			fprintf(stderr, "Phdr is too small\n");
			return;
		}
		for(i = 0; i < ehdr.e_phnum; i++) {
			if(fseek(fp, ehdr.e_phoff + i * ehdr.e_phentsize, SEEK_SET) == -1) {
				fprintf(stderr, "Unable to seek to Phdr %d\n", i);
				return;
			}
			if(fread(&phdr, sizeof phdr, 1, fp) != 1) {
				fprintf(stderr, "Unable to read Phdr %d\n", i);
				return;
			}
			if(xlat) {
				swap_Elf32_Phdr(&phdr);
			}
			switch(phdr.p_type) {
#ifdef __QNXNTO__
			case PT_NOTE:
				if(fseek(fp, phdr.p_offset, SEEK_SET) != -1) {
					static Elf32_Nhdr			*note;
					static unsigned				note_size;
					Elf32_Nhdr					*nhdr;

					if(phdr.p_filesz > note_size) {
						if(!(nhdr = realloc(note, phdr.p_filesz))) {
							break;
						}
						note_size = phdr.p_filesz;
						note = nhdr;
					}
					if(fread(nhdr = note, phdr.p_filesz, 1, fp) == 1) {
						unsigned					size = phdr.p_filesz;

						while(size) {
							int							len;
							char						*name;

							if(xlat) {
								swap_Elf32_Nhdr(nhdr);
							}

							len = sizeof *nhdr + ((nhdr->n_namesz + 3) & ~3) + ((nhdr->n_descsz + 3) & ~3);
							if(len < 0 || len > size) {
								break;
							}
							name = (char *)(nhdr + 1);
							if(!strcmp(name, QNX_NOTE_NAME)) {
								union {
									procfs_sysinfo			sysinfo;
									procfs_info				info;
									procfs_status			status;
									procfs_greg				greg;
									procfs_fpreg			fpreg;
								}						*p = (void *)(nhdr + 1) + nhdr->n_namesz;

								switch(nhdr->n_type) {
								case QNT_CORE_SYSINFO:
									if(xlat) {
										swap_procfs_sysinfo(&p->sysinfo);
									}
									if(p->sysinfo.total_size > nhdr->n_descsz) {
										// This is for old dump files
										p->sysinfo.total_size = nhdr->n_descsz;
									}
									print_sysinfo(&p->sysinfo, xlat);
									break;

								case QNT_CORE_INFO:
									if(xlat) {
										swap_procfs_info(&p->info);
									}
									print_info(&p->info);
									break;

								case QNT_CORE_STATUS:
									if(xlat) {
										swap_procfs_status(&p->status);
									}
									print_status(&p->status);
									break;

								case QNT_CORE_GREG:
									if(xlat) {
										swap_procfs_greg(&p->greg, ehdr.e_machine);
									}
									break;

								case QNT_CORE_FPREG:
									if(xlat) {
										swap_procfs_fpreg(&p->fpreg, ehdr.e_machine);
									}
									break;

								default:
									break;
								}
							}
							nhdr = (Elf32_Nhdr *)((char *)nhdr + len);
							size -= len;
						}
					}
				}
				break;
#endif

			case PT_LOAD:
				if(verbose) {
					print_pt_load(fp, &phdr);
				}
				break;

			default:
				break;
			}
		}
	}
}

int main(int argc, char *argv[]) {
	int						opt;

	while((opt = getopt(argc, argv, "v")) != -1) {
		switch(opt) {
		case 'v':
			verbose++;
			break;
		}
	}
	while(optind < argc) {
		FILE					*fp;
		char					*name;

		if((fp = fopen(name = argv[optind++], "r"))) {
			printf("%s:\n", name);
			dsp_file(fp, name);
		} else {
			fprintf(stderr, "Unable to open %s\n", name);
		}
	}
	return EXIT_SUCCESS;
}
