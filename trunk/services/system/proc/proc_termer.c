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
#include <libgen.h>
#include <fcntl.h>
#include <share.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/procfs.h>

extern unsigned fd_close_timeout;

static void trace(PROCESS *prp, siginfo_t *info) {
	char						buff[512];
	void 						*vaddr;
	int							fd;
	char						*p;
	int							sig;
	static const char			sigstrs[][5] = {
		"HUP",		"INT",		"QUIT",		"ILL",		
		"TRAP",		"ABRT",		"EMT",		"FPE",		
		"KILL",		"BUS",		"SEGV",		"SYS",		
		"PIPE",		"ALRM",		"TERM",		"USR1",		
		"USR2",		"CHLD",		"PWR ",		"WNCH",		
		"URG",		"POLL",		"STOP",		"TSTP",		
		"CONT",		"TTIN",		"TTOU",		"VTAL",
		"PROF",		"XCPU",		"XFSZ"
	};
	
	vaddr = info->si_fltip;
	sig = info->si_signo;
	if(info->si_code > SI_USER) {
		procfs_debuginfo	*debuginfo = (procfs_debuginfo *)buff;

		switch(sig) {
		case SIGSEGV:
		case SIGILL:
		case SIGFPE:
		case SIGTRAP:
		case SIGBUS:
			fd = open("/proc/self/as", O_RDONLY);
			if(fd != -1) {
				debuginfo->vaddr = (uintptr_t)vaddr;
				if(devctl(fd, DCMD_PROC_MAPDEBUG, debuginfo, sizeof(*debuginfo), 0) == EOK) {
					vaddr = (void *)(uintptr_t)debuginfo->vaddr;
				}
				close(fd);
			}
			break;
		default:
			if(ker_verbose <= 1) {
				return;
			}
			break;
		}
	} else if(ker_verbose <= 1) {
		// Don't bother displaying user delivered signals
		return;
	}
	
	fd = STDERR_FILENO;
	if(ConnectServerInfo(0, fd, 0) != (fd)) {
		fd = sopen("/dev/text", O_WRONLY, SH_DENYNO);
	}

	p = buff + ksnprintf(buff, sizeof buff - 2, "\nProcess %d (%s) ", prp->pid, basename(prp->debug_name));
	if(sig == SIGCHLD) {
		p += ksnprintf(p, (&buff[sizeof buff] - p) - 2, "exited status=%d.", info->si_status);
	} else {
		p += ksnprintf(p, (&buff[sizeof buff] - p) - 2, "terminated ");
		if((sig < 1) || (sig > NUM_ELTS(sigstrs))) {
			p += ksnprintf(p, (&buff[sizeof buff] - p) - 2, "signo=%d", sig);
		} else {
			p += ksnprintf(p, (&buff[sizeof buff] - p) - 2, "SIG%s", sigstrs[sig - 1]);
		}
		p += ksnprintf(p, (&buff[sizeof buff] - p) - 2, " code=%d", info->si_code);
		if(info->si_code > SI_USER) {
			Dl_info					sym;

			switch(sig) {
			case SIGSEGV:
			case SIGILL:
			case SIGFPE:
			case SIGTRAP:
			case SIGBUS:
				p += ksnprintf(p, (&buff[sizeof buff] - p) - 2, " fltno=%d ip=%p", info->si_fltno, info->si_fltip);
				if(_dladdr(info->si_fltip, &sym) && sym.dli_sname) {
					if(!sym.dli_fname) {
						sym.dli_fname = basename(prp->debug_name);
					}
					p += ksnprintf(p, (&buff[sizeof buff] - p) - 2, "(%s@%s+0x%x)", sym.dli_fname, sym.dli_sname, (uintptr_t)info->si_fltip - (uintptr_t)sym.dli_saddr);
				}
				if(vaddr != info->si_fltip) {
					p += ksnprintf(p, (&buff[sizeof buff] - p) - 2, " mapaddr=%p.", vaddr);
				}
				if(info->si_addr != info->si_fltip) {
					p += ksnprintf(p, (&buff[sizeof buff] - p) - 2, " ref=%p", info->si_addr);
				}
				if(info->si_bdslot) {
					p += ksnprintf(p, (&buff[sizeof buff] - p) - 2, " bdslot=1");
				}
				break;
			default:
				break;
			}
		} else {
			p += ksnprintf(p, (&buff[sizeof buff] - p) - 2, " by process %d value=%x.", info->si_pid, info->si_value.sival_int);
		}
	}
	*p++ = '\n';
	*p = '\0';
	write(fd, buff, p - buff);
}

static void cleanup(PROCESS *prp, siginfo_t *info) {
	unsigned		i;
	CHANNEL			*chp;
	int				exec;
	int				r;
	unsigned		todo;
	unsigned		last_todo;

	sigfillset(&prp->sig_ignore);
	exec = (prp->lcp->state == LC_EXEC_SWAP);

	if(info->si_signo != SIGCHLD) {
		if(prp->flags & _NTO_PF_COREDUMP) {
			info->si_code = CLD_DUMPED;
		} else {
			info->si_code = CLD_KILLED;
		}
		info->si_status = info->si_signo;
		info->si_pid = prp->pid;
		info->si_signo = SIGCHLD;
	}
	info->si_utime = info->si_stime = 0;

	// Remove any requests for events
	procmgr_event_destroy(prp);

	// destroy all the channels
	i = -1U;
	while((chp = QueryObject(_QUERY_PROCESS, prp->pid, _QUERY_PROCESS_CHANCONS, ++i, &i, 0, 0))) {
		if(chp->type == TYPE_CHANNEL) {
			r = ChannelDestroy_r(i);
			CRASHCHECK(r != EOK);
		}
	}

	// remove any pending alarm
	if(!exec && prp->alarm) {
		struct _itimer	cancel;

		memset(&cancel, 0, sizeof(cancel));
		r = TimerAlarm_r(CLOCK_REALTIME, &cancel, NULL);
		CRASHCHECK(r != EOK);
	}

	// remove any timers and timer vector
	i = -1U;
	while(QueryObject(_QUERY_PROCESS, prp->pid, _QUERY_PROCESS_TIMERS, ++i, &i, 0, 0)) {
		r = TimerDestroy_r(i);
		CRASHCHECK(r != EOK);
	}

	// close all the FD's
	i = -1U;
	while(QueryObject(_QUERY_PROCESS, prp->pid, _QUERY_PROCESS_FDCONS, ++i, &i, 0, 0)) {
		if(!exec || (ConnectFlags(0, i, 0, 0) & FD_CLOEXEC)) {
			uint64_t	timeout = fd_close_timeout * (uint64_t)1000000000;
			struct _server_info infolocal;

			(void) ConnectServerInfo(0, i, &infolocal);

			// Only wait fd_close_timeout seconds for the close to happen, then just detach
			// the connection. If the server manages to receive the _IO_CLOSE
			// but is too messed up to respond to an unblock pulse, this
			// termer thread is screwed.
			if(!(infolocal.chid & _NTO_GLOBAL_CHANNEL)) {
				TimerTimeout(CLOCK_REALTIME,
					_NTO_TIMEOUT_SEND|_NTO_TIMEOUT_REPLY, NULL, &timeout, NULL);
				if(close(i) != 0) {
					ConnectDetach(i);
				}
			} else {
				ConnectDetach(i);
			}
		}
	}

	// Remove any conf variables
	sysmgr_conf_destroy(prp);

	// Release any resources
	rsrcdbmgr_destroy_process(prp);

	(void) proc_lock_pid(prp->pid);
	// The first time remove the address space
	if(prp->memory) {
		// Clean up address space before we kill the coid to proc
		if(!(prp->flags & _NTO_PF_VFORKED)) {
			// Remove all memory associated with process
			prp->pls = NULL;
			munmap(0, ~0);
		}

		proc_wlock_adp(sysmgr_prp);
		while(ProcessShutdown(0, exec) == -1 && errno == EAGAIN) {
			// nothing to do
		}
		proc_unlock_adp(sysmgr_prp);
	}
	proc_unlock(prp);

	// Clear up side chanels and server connection ids
	//
	// Might have to run the loop multiple times because you could have a 
	// server connection id that is lower than the connection id and you 
	// need to close both.
	//
	// E.g.:
	//	  ConnectAttach(0, 0, ChannelCreate(0), _NTO_SIDE_CHANNEL, 0);
	//
	// The scoid will be lower than the coid so the first time will try to
	// close the scoid and fail because it is still in use. It will then 
	// close the coid allowing this loop to close the scoid.
	//
	last_todo = 0;
	for( ;; ) {
		todo = 0;
		i = -1U;
		while((chp = QueryObject(_QUERY_PROCESS, prp->pid, _QUERY_PROCESS_CHANCONS, ++i, &i, 0, 0))) {
			if(chp->type == TYPE_CONNECTION) {
				r = ConnectDetach_r(_NTO_SIDE_CHANNEL | i);
				switch(r) {
				case EINVAL:	
					++todo;
					break;
				case EOK:
					break;
				default:
					crash();
				}
			}
		}
		if(todo == 0) break;
		CRASHCHECK(todo == last_todo);
		last_todo = todo;
	}

	// Do the more sensitive removal at kernel time
	prp->siginfo = *info;

	if(!exec && !(prp->flags & _NTO_PF_NOZOMBIE)) {
		PROCESS						*parent;

		parent = proc_lock_parent(prp);
		if(!sigismember(&prp->parent->sig_ignore, SIGCHLD)) {
			prp->flags |= _NTO_PF_WAITINFO;
		}
		proc_unlock(parent);
	}

	prp->lcp->state |= LC_TERMER_FINISHED;

	// Now get rid of everything else
	(void) ProcessShutdown(0, exec);

	// We should never get here
	ThreadDestroy(-1, -1, 0);
}

static void handler(int signo) {
	PROCESS						*prp;

	// get the process entry
	prp = proc_lookup_pid(getpid());

	if(signo != SIGINT && signo != SIGQUIT && ker_verbose > 0) {
		kprintf("Termer thread crashed with signo %d\n", signo);
	}

	cleanup(prp, &prp->lcp->info);
}

void *terminator(void *parm) {
	struct loader_context		*lcp = parm;
	PROCESS						*prp;

	// get the process entry
	prp = proc_lookup_pid(getpid());

	if((ker_verbose > 0) && (lcp->state != LC_EXEC_SWAP) && prp->debug_name) {
		struct sigaction			act;

		// If trace routine crashed, catch it and still clean up process.
		act.sa_handler = handler;
		sigfillset(&act.sa_mask);
		act.sa_flags = 0;

		sigaction(SIGSEGV, &act, 0);
		sigaction(SIGILL, &act, 0);
		sigaction(SIGFPE, &act, 0);
		sigaction(SIGBUS, &act, 0);
		sigaction(SIGINT, &act, 0);
		sigaction(SIGQUIT, &act, 0);

		sigemptyset(&act.sa_mask);
		sigaddset(&act.sa_mask, SIGSEGV);
		sigaddset(&act.sa_mask, SIGILL);
		sigaddset(&act.sa_mask, SIGFPE);
		sigaddset(&act.sa_mask, SIGBUS);
		sigaddset(&act.sa_mask, SIGINT);
		sigaddset(&act.sa_mask, SIGQUIT);
		sigprocmask(SIG_UNBLOCK, &act.sa_mask, 0);
		trace(prp, &lcp->info);
		sigprocmask(SIG_BLOCK, &act.sa_mask, 0);
	}

	cleanup(prp, &lcp->info);

	return 0;
}

__SRCVERSION("proc_termer.c $Rev: 199085 $");
