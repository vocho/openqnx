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

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <stdlib.h>
#include "utils.h"
#include "backtrace.h"
#include "mem_reader.h"


static
int
mem_reader_unsafe_wrapper (mem_reader_t *rdr,
						   void *mem, bt_addr_t position, size_t sz)
{
	rdr->fn(rdr,mem,position,sz);
	return 0;
}

void
_BT(mem_reader_init) (mem_reader_t *rdr,
					  void *fn_safe, void *fn,
					  int fd, void *cache)
{
	rdr->tid = pthread_self();
	rdr->fn_safe = fn_safe;
	rdr->fn = fn;
	rdr->fd = fd;
	rdr->sig_handler_installed = 0;
	if (rdr->tid < 1 || rdr->tid >= _POSIX_THREAD_THREADS_MAX) {
		/* Since the tid cannot be accomodated in env_g, just fall
		 * back to the unsafe memreader and hope for the best */
		rdr->fn_safe = mem_reader_unsafe_wrapper;
	}
	rdr->cache = cache;
	rdr->cache_offset=BT_ADDR_INVALID;
	rdr->err = EOK;
}




/* Really really ugly ack.  We'll correctly trap sigsegv if the tid
 * has a slot in check_fail.  Otherwise, we let the chips fall where
 * they may... */
static sigjmp_buf* env_g[_POSIX_THREAD_THREADS_MAX+1] = {0};

#define ENV_INDEX(t) ((t>_POSIX_THREAD_THREADS_MAX)?(_POSIX_THREAD_THREADS_MAX):((t)-1))
static void
sig_trap (int sig)
{
	signal(SIGSEGV, SIG_IGN);
	signal(SIGBUS, SIG_IGN);
	{
	int t = pthread_self();
	sigjmp_buf * res = env_g[ENV_INDEX(t)];
	siglongjmp(*res,1);
	}
}


int
_BT(mem_reader_install_sig_trap) (mem_reader_t *rdr)
{
	struct sigaction newact;
	sigset_t newset;
	int e;
	int t= rdr->tid;

	if (rdr->sig_handler_installed)
		abort();
	
	// Heavy handed way to avoid SIGSEGV while backtracing...
	if (sigaction(SIGSEGV, 0, &(rdr->oldact.sigsegv)) == -1)
		return -1;
	if (sigaction(SIGBUS, 0, &(rdr->oldact.sigbus)) == -1) {
		sigaction(SIGSEGV, &(rdr->oldact.sigsegv), 0);
		return -1;
	}
	env_g[ENV_INDEX(t)] = 0;
	newact.sa_flags = 0; /* i.e. ~SA_SIGINFO */
	sigfillset(&(newact.sa_mask)); /* block all signals while in a
									* the sig handler */
	newact.sa_handler = sig_trap;
	sigfillset(&newset);
	sigdelset(&newset, SIGSEGV); /* only allow SIGSEGV while
								  * memory is read */
	sigdelset(&newset, SIGBUS);	/* ... and SIGBUS */
	if ((e=pthread_sigmask(SIG_SETMASK, &newset, &(rdr->oldset))) != 0) {
		errno = e;
		return -1;
	}
	if (sigaction(SIGSEGV, &newact, 0) == -1) {
		// Try to revert to old sigset...
		pthread_sigmask(SIG_SETMASK, &(rdr->oldset), 0);
		return -1;
	}
	if (sigaction(SIGBUS, &newact, 0) == -1) {
		// Try to revert to old sigset...
		pthread_sigmask(SIG_SETMASK, &(rdr->oldset), 0);
		return -1;
	}
	env_g[ENV_INDEX(t)] = &(rdr->env);
	rdr->sig_handler_installed = 1;
	return 0;
}

int
_BT(mem_reader_uninstall_sig_trap) (mem_reader_t *rdr)
{
	int errcnt=0;
	int t= rdr->tid;
	env_g[ENV_INDEX(t)] = 0;
	rdr->sig_handler_installed = 0;
	if (sigaction(SIGSEGV, &(rdr->oldact.sigsegv), 0) == -1)
		errcnt ++;
	if (sigaction(SIGBUS, &(rdr->oldact.sigbus), 0) == -1)
		errcnt ++;
	if (errcnt)
		return -1;
	pthread_sigmask(SIG_SETMASK, &(rdr->oldset), 0);
	return 0;
}

#ifndef _BT_LIGHT

int bt_mem_reader_no_cache = 0;

static int
read_mem_procfs (mem_reader_t *rdr,
				 void *mem, bt_addr_t position, size_t size)
{
	char *p=mem;
	size_t to_read=size;
	int cnt=0, ret;
	if (lseek(rdr->fd, (off_t)position, SEEK_SET) == -1)
		return -1;
	while (to_read > 0) {
		if ((ret=read(rdr->fd, p+cnt, to_read)) == -1) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			else
				return -1;
		}
		if (ret == 0) {
			/*
			 * Should always read exactly 'size' bytes.  Failure to do
			 * so is assumed to be due to a non-existent memory location.
			 */
			errno=EFAULT;
			return -1;
		}
		cnt+=ret;
		to_read-=cnt;
	}
	return size;
}

#endif



void
_BT(read_mem_direct) (mem_reader_t *rdr,
					  void *mem, bt_addr_t position, size_t size)
{
	(void)_BT(memcpy)(mem, (void*)position, size);
}


#ifndef _BT_LIGHT
int
_bt_read_mem_direct_safe (mem_reader_t *rdr,
						  void *mem, bt_addr_t position, size_t size)
{
	/* If the chunk of memory is entirely in a page that was
	 * previously accessed, or if the tid isn't one for which we have
	 * room in env_g, then read the memory without trapping SIGSEGV */
	if (((position/MEM_RDR_CACHE_SZ)==(rdr->cache_offset/MEM_RDR_CACHE_SZ) &&
		 ((position+size)/MEM_RDR_CACHE_SZ)==(rdr->cache_offset/MEM_RDR_CACHE_SZ) &&
		 rdr->cache_offset != BT_ADDR_INVALID)) {
		(void)_bt_memcpy(mem, (void*)position, size);
		return size;
	}

	if (sigsetjmp(rdr->env, 1) == 0) {
		if (_bt_mem_reader_install_sig_trap(rdr) == -1)
			return -1;
		
		(void)_bt_memcpy(mem, (void*)position, size);
		/* Record that the page containing position is safe to read... */
		rdr->cache_offset=position-position%MEM_RDR_CACHE_SZ;
	} else {
		rdr->err=EFAULT;
	}
	if (_bt_mem_reader_uninstall_sig_trap(rdr) == -1)
		return -1;

	if (rdr->err == EFAULT) {
		errno = rdr->err;
		return -1;
	}

	return size;
}


/*
 * Note: the cache is the smallest page size on nto, and the cache
 * offset is always chosen to be on a page boundary.
 * This method of handling the cache avoids accidentally accessing
 * an invalid address.
 */ 
int
_bt_read_mem_indirect_safe (mem_reader_t *rdr,
							void *mem, bt_addr_t position, size_t size)
{
	bt_addr_t read_addr;
	void *p;
	int ret;

	if (bt_mem_reader_no_cache ||
		rdr->cache == 0 ||
		size > MEM_RDR_CACHE_SZ ||
		(position/MEM_RDR_CACHE_SZ) != ((position+size)/MEM_RDR_CACHE_SZ)) {
		// If no cache, or if size is greater than cache size, or if
		// the start/end of the region to read do not fall in the same
		// page, then just don't bother with the cache
		return read_mem_procfs(rdr, mem, position, size);
	}

	if (rdr->cache_offset == BT_ADDR_INVALID ||
		position < rdr->cache_offset ||
		(position+size) > (rdr->cache_offset+MEM_RDR_CACHE_SZ)) {
		read_addr=position-position%MEM_RDR_CACHE_SZ;
		rdr->cache_offset=BT_ADDR_INVALID;
		ret = read_mem_procfs(rdr, rdr->cache, read_addr, MEM_RDR_CACHE_SZ);
		if (ret == -1)
			return -1;
		if (ret != MEM_RDR_CACHE_SZ) {
			errno=EINVAL;
			return -1;
		}
		rdr->cache_offset=read_addr;
	}

	p=&(rdr->cache[position-(rdr->cache_offset)]);
	(void)_bt_memcpy(mem, p, size);

	return size;
}

void
_bt_read_mem_indirect (mem_reader_t *rdr,
					   void *mem, bt_addr_t position, size_t size)
{
	if (_bt_read_mem_indirect_safe(rdr, mem, position, size) == -1)
		MEM_FAULT;
}


#endif
