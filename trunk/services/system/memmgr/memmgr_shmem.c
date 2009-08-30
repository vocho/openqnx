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

#include <sys/dcmd_proc.h>
#include "mm_internal.h"


int
shmem_create(OBJECT *obp, void *d) {
	mem_map_t		*msg = d;
	int		r;

	//RUSH3: Interface break
	obp->mem.pm.mtime = time(0);
	//RUSH2: Need a way to inherit/modify the restrict list.
	obp->mem.mm.restriction = restrict_user;
	obp->mem.mm.pmem_cache = &obp->mem.mm.pmem;
	obp->shmem.vaddr = VA_INVALID;
	if(msg != NULL) {
		if(msg->i.flags & MAP_ANON) {
			if(msg->i.flags & MAP_PHYS) {
				obp->mem.mm.flags = SHMCTL_ANON | SHMCTL_PHYS | MM_SHMEM_SPECIAL;
			}
			if(msg->i.flags & MAP_NOX64K) {
				obp->mem.mm.flags |= SHMCTL_NOX64K | MM_SHMEM_SPECIAL;
			}
			if(msg->i.flags & MAP_LAZY) {
				obp->mem.mm.flags |= SHMCTL_LAZY | MM_SHMEM_SPECIAL;
			}
			pathmgr_object_clone(obp);
			proc_mux_lock(&obp->mem.mm.mux);
			r = memmgr.resize(obp, msg->i.len);
			proc_mux_unlock(&obp->mem.mm.mux);
			pathmgr_object_unclone(obp);
			if(r != EOK) return r;
		} else if(msg->i.flags & MAP_PHYS) {
			obp->mem.mm.size = msg->i.len;
			obp->mem.mm.flags = MM_SHMEM_SPECIAL | SHMCTL_PHYS | MM_SHMEM_IFS;
			pathmgr_object_clone(obp);
			proc_mux_lock(&obp->mem.mm.mux);
			r = memmgr.memobj_phys(obp, msg->i.offset);
			proc_mux_unlock(&obp->mem.mm.mux);
			pathmgr_object_unclone(obp);
			if(r != EOK) return r;
		} else {
			crash();
		}
	}
	return EOK;
}


int 
shmem_done(OBJECT *obp) {
	if(obp->mem.mm.refs != NULL) return 0;
	(void)memmgr.resize(obp, 0);
	return 1;
}


size_t
shmem_name(OBJECT *obp, size_t max, char *dest) {
	return pathmgr_object_pathname(obp, max, dest);
}


int 
shmem_devctl(resmgr_context_t *ctp, io_devctl_t *msg, OBJECT *obp) {
	union {
		unsigned				ptr;
		dcmd_memmgr_memobj		memobj;
	}							*data = (void *)(msg + 1);
	struct _client_info			info;
	paddr64_t					end;
	int							r;

	switch((unsigned)msg->i.dcmd) {
	case DCMD_MEMMGR_MEMOBJ_OLD:
		data->memobj.special = 0;
		// fall through
	case DCMD_MEMMGR_MEMOBJ: 
		r = 0;
		if((obp->mem.mm.flags & MM_SHMEM_SPECIAL) || (obp->mem.mm.pmem != NULL)) {
			// The object is already "special"
			r = -1;
		} else if(ConnectClientInfo(ctp->info.scoid, &info, 0) == -1) {
			r = -1;
		} else if(!proc_isaccess(0, &ctp->info) && ((data->memobj.flags & (SHMCTL_ANON|SHMCTL_PHYS)) == SHMCTL_PHYS)) {
			// Should check permissions on /dev/mem, but this more 
			// restrictive "must be root" check will do for now..
			r = -1;
		} else {
			memobj_lock(obp);
			if ((obp->mem.mm.flags & (SHMCTL_PHYS|SHMCTL_GLOBAL|SHMCTL_HAS_SPECIAL)) && (info.cred.euid != 0)) {
				r = -1;
			} else {
				obp->mem.mm.flags |= (data->memobj.flags & SHMCTL_FLAG_MASK) | MM_SHMEM_SPECIAL;
				obp->shmem.special = data->memobj.special;
				if(obp->mem.mm.flags & SHMCTL_ANON) {
					if((obp->mem.mm.flags & (SHMCTL_PHYS|SHMCTL_LAZY)) == (SHMCTL_PHYS|SHMCTL_LAZY)) {
						// Asking for lazy allocation of physically contiguous
						// memory doesn't make sense
						r = -1; 
					} else if(data->memobj.size > ~(size_t)0) {
						r = -1;
					} else if(memmgr.resize(obp, data->memobj.size) != EOK) {
						r = -1;
					}
				} else if((obp->mem.mm.flags & SHMCTL_PHYS) && !(data->memobj.size & (memmgr.pagesize - 1)) && !(data->memobj.offset & (memmgr.pagesize - 1))) {
					// Set our memobj to be the offset and size specified
					obp->mem.mm.size = data->memobj.size;
					end = ROUNDUP(data->memobj.offset + data->memobj.size, __PAGESIZE) - 1;
					if((end < data->memobj.offset) || (end > last_paddr)) {
						r = -1;
					} else if(memmgr.memobj_phys(obp, data->memobj.offset) != EOK) {
						r = -1;
					}
				} else {
					r = -1;
				}
			}
			memobj_unlock(obp);
		}
		msg->o.ret_val = r;
		break;
	default:
		return _RESMGR_DEFAULT;
	}
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

__SRCVERSION("memmgr_shmem.c $Rev: 198777 $");
