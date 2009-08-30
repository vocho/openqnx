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

#include <fcntl.h>
#include <sys/elf.h>
#include <sys/elf_nto.h>
#include "externs.h"
#include <sys/resmgr.h>
#include <time.h>
#include "mm_internal.h"


/*
 Functions to handle mmap'ing of generic files.
*/

#define STALE_CLEAN_INTERVAL	1

static VECTOR			memmgr_fd_vector;
static int				memmgr_fd_code;
static int 				timerid;

// We want to use obp->hdr.next == NULL as an indicator that we're
// not on the zombie list.
#define	ZOMBIE_END		((OBJECT *)1)

/* the mutex is used to protect condvar, fd_vector, fdmem_open_lock and fdmem_close_lock */
static pthread_mutex_t	fdmem_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t	fdmem_condvar = PTHREAD_COND_INITIALIZER;
static unsigned			fdmem_open_lock;
static unsigned			fdmem_close_lock;
static OBJECT			*fdmem_zombies = ZOMBIE_END;
static unsigned			fdmem_zombie_gen;
static unsigned			fdmem_cleanup;
static OBJECT			*fdmem_cleaner_object;


#define MMAPFD_CLOSE_WAIT		0x80000000
#define MMAPFD_CLOSELOCK_MASK	0x7fffffff

#define MMAPFD_STALES_HIGHWATER		50

#define MMAPFD_CLEANUP_QUEUED		0x00000001
#define MMAPFD_CLEANUP_INPROGRESS	0x00000002

#define MMAPFD_WAKEUP_CLOSER \
if(fdmem_open_lock == 0) { \
	pthread_cond_broadcast(&fdmem_condvar); \
}

#define	MMAPFD_WAKEUP_OPENER \
if(fdmem_close_lock == 0) { \
	pthread_cond_broadcast(&fdmem_condvar); \
}

// Timeout in seconds after which mappings get dumped
#define STICKY_TIMEOUT			300

// Minimum free mem below which stale mappings always get dumped
#define MIN_MEM_FREE_SIZE		65536


int
fdmem_create(OBJECT *obp, void *extra) {
	int			mmap_fd = (uintptr_t)extra;
	int			fd;

	obp->fdmem.fd = mmap_fd;
	//RUSH2: Need a way to inherit/modify the restrict list.
	obp->mem.mm.restriction = restrict_user;
	obp->mem.mm.pmem_cache = &obp->mem.mm.pmem;
	obp->mem.mm.size = -1;
	fd = vector_add(&memmgr_fd_vector, obp, mmap_fd);
	if(fd != mmap_fd) {
		if(fd != -1) crash();
		return ENOMEM;
	}
	return EOK;
}


int 
fdmem_done(OBJECT *obp) {
	if(obp == fdmem_cleaner_object) {
		atomic_add(&fdmem_close_lock, 1);
		// RUSH1: We should release the fdmem_mutex over the close,
		// RUSH1: but then we get into race conditions with the open
		if(memmgr.resize(obp, 0) != EOK) crash();
		vector_rem(&memmgr_fd_vector, obp->fdmem.fd);
		close(obp->fdmem.fd);
		free(obp->fdmem.name);
		atomic_sub(&fdmem_close_lock, 1);
		return 1;
	}

	pthread_mutex_lock(&fdmem_mutex);

	//RUSH3: Should we crash if obp is ever a zombie in this function,
	//RUSH3: rather just if refs is != NULL?

	// We have to check for hdr.refs != 0 as well since object_done() could
	// have checked for hdr.refs == 0 after mem_map_fd() has incremented
	// it but before it did the memobj_lock().

	if((obp->mem.mm.refs == NULL) && ((obp->hdr.refs & ~0x80000000) == 0)) {
		obp->mem.pm.mtime = time(0);
		if(obp->mem.mm.flags & MM_FDMEM_STICKY) {
			obp->mem.pm.mtime += STICKY_TIMEOUT;
		}
		if(obp->hdr.next == NULL) {
			// Put it on the zombie list
			obp->hdr.next = fdmem_zombies;
			fdmem_zombies = obp;
			++fdmem_zombie_gen;
			(void)memmgr_fd_compact();
		}
	} else if(obp->hdr.next != NULL) {
		// Somebody was using an object on the zombie list. Not good.
		crash();
	}

	pthread_mutex_unlock(&fdmem_mutex);

	return 0;
}


size_t
fdmem_name(OBJECT *obp, size_t max, char *dest) {
	char	*name;

	name = obp->fdmem.name;
	//RUSH1: query the name from the file system?
	if(name == NULL) return 0;
	
	STRLCPY(dest, name, max);
	return strlen(dest);
}

/*
 * This sets the name of the fdmem object. Called from 
 * vmm_debuginfo. We need to do this here and properly protect
 * access to the name field.
 */
int
memmgr_fd_setname(OBJECT *obp, char *name) {
	
	pthread_mutex_lock(&fdmem_mutex);
	memobj_lock(obp);

	free(obp->fdmem.name);
	obp->fdmem.name = strdup(name);

	memobj_unlock(obp);
	pthread_mutex_unlock(&fdmem_mutex);

	return 0;
}

static int
io_mmap(struct _msg_info *info, int fd, resmgr_context_t *ctp, mem_map_t *mmapp, unsigned prot) {
	io_mmap_t	msg;

	msg.i.type = _IO_MMAP;
	msg.i.combine_len = sizeof msg.i;
	msg.i.offset = mmapp->i.offset;
	msg.i.prot = prot;
	msg.i.info = *info;
	msg.i.info.nd =	netmgr_remote_nd(info->nd, ctp->info.nd);	// error check below
	msg.i.info.pid = ctp->info.pid;
	msg.i.info.coid = mmapp->i.fd;

	if((msg.i.info.nd == ~0U) ||
		(MsgSendnc(fd, &msg.i, sizeof msg.i, &msg.o, sizeof msg.o) == -1)) {
		return -1;
	}
	return msg.o.coid;
}


static int 
mem_map_fd(resmgr_context_t *ctp, PROCESS *prp, mem_map_t *mmapp, OBJECT **object) {
	int					fd, error;
	int					mmap_fd;
	OBJECT				*obp;
	unsigned			prot;
	unsigned			orig_prot;			
	struct _msg_info	info;
	int status;

	// Get information on passed fd
	if(ConnectServerInfo(ctp->info.pid, mmapp->i.fd, &info) != mmapp->i.fd) {
		return EBADF;
	}

	pthread_mutex_lock(&fdmem_mutex);
	while(fdmem_close_lock) {
		pthread_cond_wait(&fdmem_condvar, &fdmem_mutex);
	}

	atomic_add(&fdmem_open_lock, 1);
	pthread_mutex_unlock(&fdmem_mutex);

	// make a connection to the channel
	if((fd = ConnectAttach(info.nd, info.pid, info.chid, 0, 0)) == -1) {
		error = errno;
		atomic_sub(&fdmem_open_lock, 1);
		MMAPFD_WAKEUP_CLOSER
		return error;
	}

	// @@@ what we need is a lock per scoid to serialize lookups and closes
	// send the mmap message

	//RUSH3: This needs to be reworked - the _IO_MMAP message should return
	//RUSH3: whether the client's fd has write perms. This code will need
	//RUSH3: to stay for old resource managers though

	orig_prot = mmapp->i.prot;
	prot = orig_prot | PROT_WRITE;
	if((mmapp->i.flags & MAP_TYPE) == MAP_PRIVATE) {
		orig_prot &= ~PROT_WRITE;
	}
	mmap_fd = io_mmap(&info, fd, ctp, mmapp, prot);
	if(mmap_fd == -1) {
		switch(errno) {
		case EACCES:	
		case EROFS:
			prot = orig_prot;
			if(!(prot & PROT_WRITE)) {
				// try again without forcing PROT_WRITE
				mmap_fd = io_mmap(&info, fd, ctp, mmapp, prot);
			}
			break;
		default:
			break;
		}
	}
	if(!(prot & PROT_WRITE) && ((mmapp->i.flags & MAP_TYPE) == MAP_SHARED)) {
		mmapp->i.flags |= IMAP_OCB_RDONLY;
	}

	if(mmap_fd == -1) {
		error = errno;
		ConnectDetach(fd);
		atomic_sub(&fdmem_open_lock, 1);
		MMAPFD_WAKEUP_CLOSER
		return error;
	}

	if(fd != mmap_fd) {
		// already accessing this file, use the first fd
		ConnectDetach(fd);
	}

	pthread_mutex_lock(&fdmem_mutex);
	obp = vector_lookup(&memmgr_fd_vector, mmap_fd);
	if(obp == NULL) {
		// first time accessing this file. Creator pays cost of creation
		obp = object_create(OBJECT_MEM_FD, (void *)mmap_fd, prp, sys_memclass_id);
		if(obp == NULL) {
			// not enough memory
			if(fd != mmap_fd) {
				pthread_mutex_unlock(&fdmem_mutex);
				atomic_sub(&fdmem_open_lock, 1);
				MMAPFD_WAKEUP_CLOSER
				return ENOMEM;
			}

			while(fdmem_open_lock != 1) {
				pthread_cond_wait(&fdmem_condvar, &fdmem_mutex);
			}
			if((obp = vector_lookup(&memmgr_fd_vector, mmap_fd)) == NULL) {
				atomic_sub(&fdmem_open_lock, 1);
				atomic_add(&fdmem_close_lock, 1);
				pthread_mutex_unlock(&fdmem_mutex);
				MMAPFD_WAKEUP_CLOSER
				close(mmap_fd);
				atomic_sub(&fdmem_close_lock, 1);
				MMAPFD_WAKEUP_OPENER
				return ENOMEM;
			} 
		}
	}

	if(obp->hdr.next != NULL) {
		OBJECT	**owner;
		OBJECT	*chk;

		CRASHCHECK(obp->mem.mm.refs != NULL);
		// It's been scheduled for removal, so we need to take it off the
		// zombie list since we're using it again
		owner = &fdmem_zombies;
		for( ;; ) {
			chk = *owner;
			CRASHCHECK(chk == NULL);
			if(chk == ZOMBIE_END) crash();
			if(chk == obp) break;
			owner = &chk->hdr.next;
		}
		*owner = chk->hdr.next;
		++fdmem_zombie_gen;
		obp->hdr.next = NULL;
	}

	// We have to first unlock the fdmem mutex to avoid deadlocks.
	// the object_done path locks "adp" then "fdmem" and we have to maintain
	// the same order here. The lock used to be below the wlock_adp() and
	// this would deadlock with the bk8_mmap testcase.
	pathmgr_object_clone(obp);
	pthread_mutex_unlock(&fdmem_mutex);

	// Have to re-obtain aspace write lock before locking the
	// object to avoid deadlocks
	status = proc_wlock_adp(prp);
	CRASHCHECK(status == -1);

	memobj_lock(obp);
	pathmgr_object_unclone(obp);

	atomic_sub(&fdmem_open_lock, 1);

	// wake up closers if any
	MMAPFD_WAKEUP_CLOSER

	*object = obp;

	return (0);
}


int 
memmgr_open_fd(resmgr_context_t *ctp, PROCESS *prp, mem_map_t *mmapp, OBJECT **obpp) {
	int					fd;
	struct stat64		sbuf;
	int					again = 1;
	int					ret;
	OBJECT				*obp;
	int status;

	if(mmapp->i.flags & MAP_ANON) {
		return EINVAL;
	}

again:
	proc_unlock_adp(prp);
	ret = mem_map_fd(ctp, prp, mmapp, obpp);
	if(ret != EOK) {
		// If mem_map_fd failed, we still haven't re-locked the aspace,
		// so do it now.
		status = proc_wlock_adp(prp);
		CRASHCHECK(status == -1);
		return ret;
	}

	obp = *obpp;
	// Successful lookup means that the object has been locked

	fd = obp->fdmem.fd;

	if(fstat64(fd, &sbuf) || S_ISDIR(sbuf.st_mode)) {
		memobj_unlock(obp);
		return ENOTSUP;
	}

	if((obp->fdmem.ino && obp->fdmem.ino != sbuf.st_ino)
			|| (obp->fdmem.dev && obp->fdmem.dev != sbuf.st_dev) ) {
// This code might not be needed. I leave it here in case.
		memobj_unlock(obp);
		if(again) {
			again = 0;
			goto again;
		}
		return ENOTSUP;
	}

	// obp->mem.ctime is used here for mtime.
	if(obp->fdmem.ino && obp->fdmem.ftime && obp->fdmem.ftime != sbuf.st_mtime) {
		off64_t	osize;

		// for sticky objects
		// the file is changed, reload the content for all who share it

		//FUTURE: Find a more efficient way of doing this (e.g. msync(MS_INVALIDATE)).
		osize = obp->mem.mm.size;
		(void)memmgr.resize(obp, 0);
		(void)memmgr.resize(obp, osize);
		obp->fdmem.ftime = sbuf.st_mtime;
	} else {
		obp->fdmem.ino = sbuf.st_ino;
		obp->fdmem.dev = sbuf.st_dev;
		obp->fdmem.ftime = sbuf.st_mtime;
	}

	obp->mem.mm.size = sbuf.st_size;
	if(sbuf.st_mode & S_ISVTX) obp->mem.mm.flags |= MM_FDMEM_STICKY;

	return 0;
}

//
// Go through mapped files, and clean up stale entries
// Always called from Proc or the termer
//
int 
memmgr_fd_compact(void) {
	if(!(atomic_set_value(&fdmem_cleanup, MMAPFD_CLEANUP_QUEUED) 
				& (MMAPFD_CLEANUP_QUEUED|MMAPFD_CLEANUP_INPROGRESS))) {
		if(MsgSendPulse(PROCMGR_COID, -1, memmgr_fd_code, NULL) != 0) {
			atomic_clr(&fdmem_cleanup, MMAPFD_CLEANUP_QUEUED);
		}
	}

	return EOK;
}


/* there is at most one memmgr_fd_cleanup run at any time */
static int 
memmgr_fd_cleanup(message_context_t *ctp, int code, unsigned flags, void *handle) {
	OBJECT					**owner;
	OBJECT					*obp;
	int						force;
	time_t					nexttime;
	time_t					currtime;
	time_t					mtime;
	unsigned				gen;

	#define TIME_INFINITY	(~(time_t)0)

	nexttime = TIME_INFINITY;

	if(atomic_set_value(&fdmem_cleanup, MMAPFD_CLEANUP_INPROGRESS) 
			& MMAPFD_CLEANUP_INPROGRESS) {
		// Somebody's already in here
		return EOK;
	}

	if(proc_thread_pool_reserve() == 0) {

		pthread_mutex_lock(&fdmem_mutex);
		//RUSH3: we used to set force if we had more than MMAPFD_STALES_HIGHWATER
		//RUSH3: to potentially free, but since we don't keep track of the
		//RUSH3: number any more, we can't do that. Can we get away without
		//RUSH3: that?
		force = 0;

		currtime = time(0);
		gen = fdmem_zombie_gen;
		owner = &fdmem_zombies;
		for( ;; ) {
			obp = *owner;
			CRASHCHECK(obp == NULL);
			if(obp == ZOMBIE_END) break;
			if(mem_free_size < MIN_MEM_FREE_SIZE) force = 1;
			if(force) {
				atomic_set(&fdmem_close_lock, MMAPFD_CLOSE_WAIT);
			}
			while(fdmem_open_lock) {
				// opening operation is still on, need to wait
				pthread_cond_wait(&fdmem_condvar, &fdmem_mutex);
			}
			atomic_clr(&fdmem_close_lock, MMAPFD_CLOSE_WAIT);
			if(gen != fdmem_zombie_gen) {
				// List has been changed, restart from begining
				gen = fdmem_zombie_gen;
				owner = &fdmem_zombies;
				// RUSH1: Do we want to limit the number of restarts?
				continue;
			}
			mtime = obp->mem.pm.mtime;
			if(force || (currtime >= mtime)) {
				int		r;

				*owner = obp->hdr.next;
				fdmem_cleaner_object = obp;
				r = object_done(obp);
				fdmem_cleaner_object = NULL;
				if(!r) {
					// Free failed for some reason, put things back
					*owner = obp;
					nexttime = currtime;
				}
				if(*owner == ZOMBIE_END) break;
			} else if(mtime < nexttime) {
				if(mtime == 0) crash();
				nexttime = mtime;
			}
			owner = &(*owner)->hdr.next;
		}
		pthread_mutex_unlock(&fdmem_mutex);

		proc_thread_pool_reserve_done();
	}

	fdmem_cleanup = 0;

	// wake up opener if any
	MMAPFD_WAKEUP_OPENER

	if(fdmem_zombies != ZOMBIE_END) {
		struct _itimer value;

		// There are fd's pending to be cleaned
		currtime = time(0);
		if(nexttime == TIME_INFINITY) nexttime = currtime;
		if(nexttime <= currtime) nexttime = currtime + STALE_CLEAN_INTERVAL;
		value.nsec = nexttime * (_Uint64t)1000000000,
		value.interval_nsec = 0;
		if(TimerSettime(timerid, TIMER_ABSTIME, &value, NULL) != 0) {
			kprintf("\nCannot arm timer (%d). Memory mapped file timed stale cleaning may not work.", errno);
		}
	}

	return EOK;
}


void 
memmgr_fd_init(void) {
	struct sigevent event;
	
	memmgr_fd_code = pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, memmgr_fd_cleanup, NULL);
	//RUSH3: getprio(0) or -1 end up as 255, probably aim for lower pulse pri?
	SIGEV_PULSE_INIT(&event, PROCMGR_COID, -1, memmgr_fd_code, 0);
	//RUSH3: Use CLOCK_MONTONIC?
	if((timerid = TimerCreate(CLOCK_REALTIME, &event)) == -1) {
		kprintf("\nCannot create timer (%d). Memory mapped file timed stale cleaning may not work.", errno);
	}
}

__SRCVERSION("memmgr_fd.c $Rev: 174894 $");
