/*
 * $QNXLicenseC:
 * Copyright 2007,2008, QNX Software Systems. All Rights Reserved.
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

#include "common.h"

#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef _BT_LIGHT
#include <process.h>
#include <sys/neutrino.h>
#endif
#include <errno.h>
#include <ucontext.h>

#include "backtrace.h"
#include "get_backtrace.h"

#if defined(_NTO_TCTL_ONE_THREAD_HOLD) && defined(_NTO_TCTL_ONE_THREAD_CONT)
#define _BT_SINGLE_THREAD_CTRL
#endif

void (*BT(gather_hook))(bt_addr_t sp) = 0;

#ifndef _BT_LIGHT

static int
hold_thread (int fd, bt_accessor_t *acc)
{
	if (acc->flags & BTF_LIVE_BACKTRACE) return 0;

#ifdef _BT_SINGLE_THREAD_CTRL
	{
		procfs_threadctl tctl;
		tctl.cmd = _NTO_TCTL_ONE_THREAD_HOLD;
		tctl.tid = acc->tid;
		memcpy(tctl.data, &(acc->tid), sizeof(acc->tid));
		return devctl(fd, DCMD_PROC_THREADCTL, &tctl, sizeof(tctl), NULL);
	}
#else
	if (acc->type == BT_SELF || acc->pid == getpid()) {
		return ThreadCtl(_NTO_TCTL_THREADS_HOLD, 0);
	} else {
		return kill(acc->pid, SIGHOLD);
	}
#endif
}


static int
cont_thread (int fd, bt_accessor_t *acc)
{
	if (acc->flags & BTF_LIVE_BACKTRACE) return 0;

#ifdef _BT_SINGLE_THREAD_CTRL
	{
		procfs_threadctl tctl;
		tctl.cmd = _NTO_TCTL_ONE_THREAD_CONT;
		tctl.tid = acc->tid;
		memcpy(tctl.data, &(acc->tid), sizeof(acc->tid));
		return devctl(fd, DCMD_PROC_THREADCTL, &tctl, sizeof(tctl), NULL);
	}
#else
	if (acc->type == BT_SELF || acc->pid == getpid()) {
		return ThreadCtl(_NTO_TCTL_THREADS_CONT, 0);
	} else {
		return kill(acc->pid, SIGCONT);
	}       
#endif
}


int
_bt_get_greg (int fd, bt_accessor_t *acc, procfs_greg *greg, int *greg_size)
{
	int err;
	if ((err=devctl(fd, DCMD_PROC_CURTHREAD, &(acc->tid),
					sizeof(acc->tid), 0)) != EOK) {
		errno=err;
		return -1;
	}
	if ((err=devctl(fd, DCMD_PROC_GETGREG, greg,
					sizeof(*greg), greg_size)) != EOK) {
		errno=err;
		return -1;
	}
	return EOK;
}

#endif

int
BT(get_backtrace) (bt_accessor_t *acc, bt_addr_t *pc, int pc_ary_sz)
{
	int real_type;
	int count = 0;
	volatile int err=EOK;		/* volatile needed due to the sigsetjmp() */
	mem_reader_t rdr;

#ifdef BT_GATHER_MAKE_LABEL
	/* Label that's used on some processors as a way to get an address
	   inside of bt_get_backtrace.  Typically for BT_GATHER_SELF... */
	BT_GATHER_MAKE_LABEL();
#endif

	if (acc==0 || pc==0 || pc_ary_sz<0) {
		errno=EINVAL;
		return -1;
	}
	if (pc_ary_sz == 0)
		return 0;

	real_type=acc->type;
	switch (acc->type) {
		case BT_SELF:
			break;
#ifndef _BT_LIGHT            
		case BT_THREAD:
			if (acc->tid == gettid()) real_type=BT_SELF;
			break;
		case BT_PROCESS:
			if (acc->pid == getpid()) {
				if (acc->tid == gettid())
					real_type=BT_SELF;
				else
					real_type=BT_THREAD;
			}
			break;
#endif
		default:
			errno=EINVAL;
			return -1;
			break;
	}

	if (real_type == BT_SELF) {
		_BT(mem_reader_init)(&rdr,
							 0,
							 _BT(read_mem_direct),
							 -1, /*fd*/
							 0 /*cache*/
							 );
		if (sigsetjmp(rdr.env, 1) == 0) {
			if (_BT(mem_reader_install_sig_trap)(&rdr) == -1)
				return -1;

			BT_GATHER_SELF(err,&rdr,pc,pc_ary_sz,count,acc->stack_limit);
		} else {
			rdr.err=EFAULT;
		}
		_BT(mem_reader_uninstall_sig_trap)(&rdr);
	} else {
#ifndef _BT_LIGHT        
		// Prepare for memory access, and HOLD/CONT of thread...
		int fd=-1;
		int flags=O_CLOEXEC|
			((acc->flags&BTF_LIVE_BACKTRACE)?O_RDWR:O_RDONLY);
		char path[64];
		{
			/* Not using sprintf because posix doesn't garantee it is
			 * async-signal-safe.  Not using strcat/strcpy
			 * because. backtrace is used within libmalloc which
			 * itself implements its own strcat/strcpy. */ 
			char pidstring[sizeof(pid_t)*3+1];
			char *pout=path;
			char *pin;
			static char *pfx="/proc/";
			static char *sfx="/as";
			pin=pfx;
			while (*pin) { (*pout)=(*pin); pin++; pout++; }
			pin=ltoa((long)(acc->pid), pidstring, 10);
			while (*pin) { (*pout)=(*pin); pin++; pout++; }
			pin=sfx;
			while (*pin) { (*pout)=(*pin); pin++; pout++; }
			*pout = '\0';
		}
		if (acc->stack_limit) {
			err=EINVAL;			/* stack_limit only supported for
								 * BT_SELF */
		} else if ((fd = open(path, flags)) == -1) {
			err=errno;
		} else {
			if (hold_thread(fd, acc) == -1) {
				err=errno;
			} else {
				if (real_type == BT_THREAD) {
					// read directly, since it is faster...
					_bt_mem_reader_init(&rdr,
										0,
										_bt_read_mem_direct,
										fd,
										0 /*cache*/
										);
				} else {
					// process reads via procfs
					_bt_mem_reader_init(&rdr,
										0,
										_bt_read_mem_indirect,
										fd,
										alloca(MEM_RDR_CACHE_SZ)/*cache*/
										);
				}
				if (sigsetjmp(rdr.env, 1) == 0) {
					if (_bt_mem_reader_install_sig_trap(&rdr) == -1)
						return -1;

					if (_bt_gather_other(acc, &rdr,
										 pc, pc_ary_sz, &count) == -1) {
						err=errno;
					}
				} else {
					rdr.err=EFAULT;
				}
				_bt_mem_reader_uninstall_sig_trap(&rdr);
				cont_thread(fd, acc);
			}
			close(fd);
		}
#endif
	}

	/* If there's a memory access error, then don't treat it as an
	 * error, but instead as simply the end of the backtrace */
	if (err == EFAULT && rdr.err == EFAULT) {
		return count;
	}
	if (err != EOK) {
		errno=err;
		return -1;
	}

	return count;
}
