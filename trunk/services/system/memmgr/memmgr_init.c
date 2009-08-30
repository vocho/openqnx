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

#include "mm_internal.h"


static int 
memmgr_handler(message_context_t *mctp, int code, unsigned flags, void *handle) {
	resmgr_context_t				*ctp = (resmgr_context_t *)mctp;
	union {
		uint16_t						type;
		mem_map_t						map;
		mem_ctrl_t						ctrl;
		mem_info_t						info;
		mem_offset_t					offset;
		mem_debug_info_t				debug_info;
		mem_swap_t						swap;
		mem_pmem_add_t					add;
		mem_peer_t						peer;
	}								*msg = (void *)ctp->msg;
	PROCESS							*prp;
	PROCESS							*peer;
	CREDENTIAL						*src;
	CREDENTIAL						*dst;
	int								status;

	// We are reply-blocked on proc, so prp cannot go away
	if(!(prp = proc_lookup_pid(ctp->info.pid))) {
		return proc_status(ctp, EL2HLT);
	}

	ctp->status = 0;

peer_restart:	
	ProcessBind(prp->pid);
	switch(msg->type) {
	case _MEM_PEER:	
		ProcessBind(0);
		if(ctp->info.msglen <= msg->peer.i.peer_msg_len) {
			// no message following
			return proc_status(ctp, EINVAL);
		}
		// Have to lock the peer, since it might disappear on us otherwise
		if(msg->peer.i.pid == ctp->info.pid) {
			peer = prp;
		} else if(!(peer = proc_lock_pid(msg->peer.i.pid))) {
			return proc_status(ctp, ESRCH);
		} 
		if(peer->flags & (_NTO_PF_TERMING | _NTO_PF_DESTROYALL | _NTO_PF_ZOMBIE)){
			// Not allowed to do things with a peer process that's 
			// terminating.
			if(peer != prp) proc_unlock(peer);
			return proc_status(ctp, ESRCH);
		}
		// You might be tempted to just adjust the msg pointer to after
		// the peer message. The problem is that the various memmgr_*
		// routines use msg as both an input and output area. If we just
		// add to 'msg', the reply message being built will go in the
		// wrong spot, since the proc_status() call down at the bottom
		// of this routine MsgReply's from the start of the message buffer.
		memmove(msg, (uint8_t *)msg + msg->peer.i.peer_msg_len, 
						(ctp->info.msglen - msg->peer.i.peer_msg_len));
		switch(msg->type) {
		case _MEM_MAP:
		case _MEM_CTRL:
			src = prp->cred;
			dst = peer->cred;
			if(!(src->info.euid == 0  ||
			   src->info.ruid == dst->info.ruid  ||
			   src->info.ruid == dst->info.suid  ||
			   src->info.euid == dst->info.ruid  ||
			   src->info.euid == dst->info.suid)) {
				// Only root and processes with the same userid are allowed
				// to manipulate another process.
				if(peer != prp) proc_unlock(peer);
				return proc_status(ctp, EPERM);
			}
			break;
		case _MEM_INFO:
		case _MEM_OFFSET:
		case _MEM_DEBUG_INFO:
			// always OK to look at another process
			break;	
		default:
			if(peer != prp) proc_unlock(peer);
			return proc_status(ctp, EINVAL);
		}
		// Try again, with the peer process
		prp = peer;
		goto peer_restart;

	case _MEM_MAP:
		status = proc_wlock_adp(prp);
		CRASHCHECK(status == -1);	
		status = memmgr_map(ctp, prp, &msg->map);
		break;

	case _MEM_CTRL:
		status = proc_wlock_adp(prp);
		CRASHCHECK(status == -1);
		status = memmgr_ctrl(prp, &msg->ctrl);
		break;

	case _MEM_INFO:
		status = proc_rlock_adp(prp);
		CRASHCHECK(status == -1);
		status = memmgr_info(ctp, prp, &msg->info);
		break;

	case _MEM_OFFSET:
		status = proc_rlock_adp(prp);
		CRASHCHECK(status == -1);
		status = memmgr_offset(ctp, prp, &msg->offset);
		break;

	case _MEM_DEBUG_INFO:
		status = proc_rlock_adp(prp);
		CRASHCHECK(status == -1);
		status = memmgr_debug_info(ctp, prp, &msg->debug_info);
		break;

	case _MEM_SWAP:
		// Didn't really need the ProcessBind(prp->pid) up above,
		// but it makes the rest of the code faster to just go ahead
		// and undo it here
		ProcessBind(0);
		status = memmgr_swap(ctp, prp, &msg->swap);
		return proc_status(ctp, status);

	case _MEM_PMEM_ADD:
		// Didn't really need the ProcessBind(prp->pid) up above,
		// but it makes the rest of the code faster to just go ahead
		// and undo it here
		ProcessBind(0);
		if(!proc_isaccess(NULL, &ctp->info)) {
			// Only 'root' is allowed to add memory, since passing
			// in bum values can cause us to crash.
			return proc_status(ctp, EPERM);
		}
		status = memmgr.pmem_add(msg->add.i.addr, msg->add.i.len);
		return proc_status(ctp, status);

	default:
		ProcessBind(0);
		return proc_status(ctp, ENOSYS);
	}

	ProcessBind(0);

	proc_unlock_adp(prp);

	if(prp->pid != ctp->info.pid) {
		// We're doing a peer process manipulation
		proc_unlock(prp);
	}

	return proc_status(ctp, status);
}


void 
memmgr_init(void) {
	guardpagesize = memmgr.pagesize;
    devzero_init();
	devmem_init();
	memmgr_fd_init();

	// Redirect _MEMMGR_* messages to mem handler
	message_attach(dpp, NULL, _MEMMGR_BASE, _MEMMGR_MAX, memmgr_handler, NULL);

	// Initialize for any background processing that the memory
	// manager needs to do.
	memmgr.init_mem(1);
}


int 
memmgr_resize(OBJECT *obj, size_t size) {
	if(obj->mem.mm.flags & MM_SHMEM_SPECIAL) {
		return EINVAL;
	}
	return memmgr.resize(obj, size);
}


// This is a temporary location for this function - we'll move it
// to it's final position once we've got the rest of the memory
// manager sorted out
void
init_memmgr(void) {
	extern MEMMGR memmgr_virtual;

	if(SYSPAGE_ENTRY(cpuinfo)->flags & CPU_FLAG_MMU) {
		memmgr = memmgr_virtual;
	} else {
#ifdef ENABLE_PHYSICAL_MEMMGR
		extern MEMMGR memmgr_phys;
		memmgr = memmgr_phys;
#else
		crash();
#endif
	}
	address_cookie = object_register_data(&process_souls, memmgr.sizeof_address_entry);
}

__SRCVERSION("memmgr_init.c $Rev: 199396 $");
