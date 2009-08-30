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

#include <sys/mman.h>
#include <sys/trace.h>
#include "mm_internal.h"

static int (* const fs_check[])(const resmgr_io_funcs_t *, mem_map_t *, void *, OBJECT **) = {
	devmem_check, devzero_check, imagefs_check 
};

int
memmgr_find_object(resmgr_context_t *ctp, PROCESS *prp, int fd, mem_map_t *msg, OBJECT **obpp) {
	struct _msg_info		info;
	void					*handle;
	const resmgr_io_funcs_t	*funcs;
	int						i;
	int						status;
	OBJECT					*object;

	/*	defends against invalid coid/fd range, memmgr_open_fd() will
		do validation for those coid/fds which are within range.
	*/
	if(fd < 0) {
		return EBADF;
	}

	info = ctp->info;
	info.coid = fd;
	object = NULL;

	funcs = _resmgr_iofuncs(ctp, &info);
	if(funcs != NULL) {
		handle = _resmgr_ocb(ctp, &info);
		i = 0;
		for( ;; ) {
			status = fs_check[i](funcs, msg, handle, &object);
			if(status == -2) {
				// File system recognized it, but wants memmgr
				// to treat it like a normal device (imagefs copy attr)
				funcs = NULL;
				object = NULL;
				break;
			}
			if(status == EOK) break;
			if(status != -1) return status;
			++i;
			if(i >= NUM_ELTS(fs_check)) return ENOTSUP;
		}
	}

	if(object != NULL) {
		memobj_lock(object);
	}

	//RUSH3: If msg == NULL, we'd really like to query the memory mapped
	//RUSH3: file code to see if 'fd' is still open and pointing at
	//RUSH3: a memory mapped file, without creating one if it doesn't exist
	if((funcs == NULL) && (msg != NULL)) {
		if(proc_thread_pool_reserve() != 0) {
			return EAGAIN;
		}
		status = memmgr_open_fd(ctp, prp, msg, &object);
		proc_thread_pool_reserve_done();
	} else {
		status = EOK;
	}
	*obpp = object;
	return status;
}

int 
memmgr_map(resmgr_context_t *ctp, PROCESS *prp, mem_map_t *msg) {
	void		*addr;
	unsigned	size;
	int			status;
	OBJECT		*object = NULL;
	int			try_again;
	size_t		pg_offset;
	pid_t		pid;
	part_id_t mpart_id;

	pid = prp->pid;

	switch(msg->i.flags & MAP_TYPE) {
	case 0:	
		if((mm_flags & MM_FLAG_BACKWARDS_COMPAT) || (msg->i.flags & MAP_ANON)) {
			// User didn't specify MAP_PRIVATE or MAP_SHARED. Really should
			// error out, but the libc malloc code does this sillyness :-(.
			// Assume they meant MAP_PRIVATE.
			msg->i.flags |= MAP_PRIVATE;
		} else {
			return EINVAL;
		}
		break;
	case MAP_PRIVATEANON:	
		msg->i.flags = (msg->i.flags & ~MAP_TYPE) | (MAP_ANON|MAP_PRIVATE);
		break;
	default:
		break;
	}
	if(msg->i.len == 0) {
		return EINVAL;
	}
	//parm rather than prot like they should.
	if(msg->i.prot & ~PROT_MASK) {
		return EINVAL;
	}
	//MAP_SYSRAM is just a status indicator - not allowed to pass it in.
check_again:	
	if(msg->i.flags & (PROT_MASK|MAP_SYSRAM|MAP_RESERVMASK|PG_MASK)) {
		if(msg->i.flags & PROT_NOCACHE) {
			//Kludge fix for test programs that pass PROT_NOCACHE in flags
			msg->i.prot |= PROT_NOCACHE;
			msg->i.flags &= ~PROT_NOCACHE;
			goto check_again;
		}
		return EINVAL;
	}
	if((msg->i.fd == NOFD) && !(msg->i.flags & MAP_PHYS)) {
		msg->i.flags |= MAP_ANON;
	}
	if(msg->i.flags & MAP_ANON) {
		if(msg->i.fd != NOFD) {
			return EINVAL;
		}
		if((msg->i.flags & MAP_LAZY) && (msg->i.flags & MAP_PHYS)) {
			// Asking for lazy allocation of physically contiguous memory
			// is just asking for trouble :-).
			return EINVAL;
		}
	} else if(msg->i.flags & MAP_PHYS) {
		paddr64_t	end;

		if(!proc_isaccess(0, &ctp->info)) {
			// This is the test for MAP_PRIVATE mappings.
			// The test for MAP_PHYS|MAP_SHARED should check permissions
			// on /dev/mem, but this more restrictive "must be root" check 
			// will do for now..
			return EPERM;
		}
		if(msg->i.fd != NOFD) {
			return EINVAL;
		}
		end = ROUNDUP(msg->i.offset + msg->i.len, __PAGESIZE) - 1;
		if((end < msg->i.offset) || (end > last_paddr)) {
			// attempt to map beyond end of physical memory.
			return EINVAL;
		}
	} else {
		status = memmgr_find_object(ctp, prp, msg->i.fd, msg, &object);
		if(status != EOK) return status;
	}

	if((object == NULL) && ((msg->i.flags & (MAP_ANON|MAP_TYPE)) == (MAP_ANON|MAP_SHARED))) {
		// anonymous shared object
		// FIX ME - is sys_memclass always correct ?
		object = object_create(OBJECT_MEM_SHARED, msg, prp, sys_memclass_id);
		if(object == NULL) {
			return ENOMEM;
		}
		msg->i.flags &= ~(MAP_ANON|MAP_PHYS);
		memobj_lock(object);
	}

	if(proc_thread_pool_reserve() != 0) {
		if(object != NULL) {
			memobj_unlock(object);
		}
		return EAGAIN;
	}

	mpart_id = (object != NULL) ? object->hdr.mpid : mempart_getid(prp, sys_memclass_id);
	for( ;; ) {
		int lock_status;
		status = memmgr.mmap(prp, msg->i.addr, msg->i.len, msg->i.prot, 
					msg->i.flags, object, msg->i.offset, msg->i.align, 
					msg->i.preload, msg->i.fd, &addr, &size, mpart_id);
		if((status != ENOMEM) && (status != EAGAIN)) break;
		//RUSH3: This only schedules the compaction, it doesn't actually
		//RUSH3: do it - we really should wait until complete, or at least
		//RUSH3: until something's been freed, and it needs to set 
		//RUSH3: try_again to TRUE if it worked (assignment below should be |=).
		(void)memmgr_fd_compact();
		ProcessBind(0);
		proc_unlock_adp(prp);
		// @@@ We have to do this some other way.
		try_again = memmgr_swap_freemem(pid, msg->i.len, 10);
		prp = proc_lookup_pid(pid);
		CRASHCHECK(prp == NULL);
		// We always aquire the aspace lock first, then the underlying
		// object (avoids deadlocks). Right now we've got the object locked 
		// and we need to get the aspace back again. We need to release
		// the object lock first. The memobj_lock() that we originally
		// did upped the reference count on the object, so no one will
		// delete it while we've got it unlocked
		if(object != NULL) {
			proc_mux_unlock(&object->mem.mm.mux);
		}
		// Now aquire the locks in the proper order.
		lock_status = proc_wlock_adp(prp);
		CRASHCHECK(lock_status == -1);
		ProcessBind(pid);
		if(object != NULL) {
			proc_mux_lock(&object->mem.mm.mux);
		}
		if(!try_again) break;
	}

#if defined(VARIANT_instr) 
	{
		iov_t 	iov[7];
		int 	iovcnt = 0;
		//Overwrite the incoming address, we wipe it out shortly anyway
		msg->i.addr = (uintptr_t)addr;
		SETIOV(iov + iovcnt, &pid, sizeof(pid)); iovcnt++;
		SETIOV(iov + iovcnt, &msg->i.addr, sizeof(msg->i.addr) + sizeof(msg->i.len)); iovcnt++;
		SETIOV(iov + iovcnt, &msg->i.flags, sizeof(msg->i.flags)); iovcnt++;
		//Wide mode only
		SETIOV(iov + iovcnt, &msg->i.prot, sizeof(msg->i.prot)); iovcnt++;
		SETIOV(iov + iovcnt, &msg->i.fd, sizeof(msg->i.fd)); iovcnt++;
		SETIOV(iov + iovcnt, &msg->i.align, sizeof(msg->i.align) + sizeof(msg->i.offset)); iovcnt++;
		//@@@ This never works for imagefs files
		if(object != NULL && object->hdr.type == OBJECT_MEM_FD && object->fdmem.name) {
			SETIOV(iov + iovcnt, object->fdmem.name, strlen(object->fdmem.name)); iovcnt++;
		}
		(void) KerextAddTraceEventIOV(_TRACE_SYSTEM_C, _NTO_TRACE_SYS_MMAP, iov, iovcnt);
	}
#endif

	proc_thread_pool_reserve_done();
	
	if(object != NULL) {
		memobj_unlock(object);
	}
	
	if(status) {
		return status;
	}

	msg->o.addr = (uintptr_t)addr;
	pg_offset = (uintptr_t)addr % memmgr.pagesize;
	msg->o.real_size = size + pg_offset;
	msg->o.real_addr = msg->o.addr - pg_offset;

	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

__SRCVERSION("memmgr_map.c $Rev: 174894 $");
