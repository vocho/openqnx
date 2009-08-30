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

#define _FILE_OFFSET_BITS	64
#include <sys/memmsg.h>
#include <sys/procfs.h>
#include "mm_internal.h"


int 
memmgr_offset(resmgr_context_t *ctp, PROCESS *prp, mem_offset_t *msg) {
	paddr_t			paddr;
	size_t			len;
	procfs_mapinfo	info;
	OBJECT			*obp;
	OBJECT			*fd_obp;
	size_t			len_requested;
	size_t			got;
	int				fd;
	unsigned		flags;

	len_requested = msg->i.len;
	switch(msg->i.subtype) {
	case _MEM_OFFSET_PT:	
		flags = VI_PGTBL;
		goto get_info;
	case _MEM_OFFSET_PHYS:	
		flags = VI_INIT;
get_info:		
		if(memmgr.vaddrinfo(prp, msg->i.addr, &paddr, &len, flags) == PROT_NONE) {
			return EACCES;
		}
		msg->o.offset = paddr;
		msg->o.fd = -1;
		break;

	case _MEM_OFFSET_FD:
		got = memmgr.mapinfo(prp, msg->i.addr, &info, NULL, 0, &obp, &fd, &len);
		if((got == 0) || (msg->i.addr < info.vaddr) || (obp == NULL)) {
			return EACCES;
		}
		switch(obp->hdr.type) {
		case OBJECT_MEM_ANON:	
			return EACCES;
		case OBJECT_MEM_TYPED:
		case OBJECT_MEM_SHARED:
		case OBJECT_MEM_FD:
			if(fd != NOFD) {
				// Check if 'fd' is still open and pointing at the object.
				if((memmgr_find_object(ctp, prp, fd, NULL, &fd_obp) == EOK) && (fd_obp != NULL)) {
					memobj_unlock(fd_obp);
					if(fd_obp != obp) fd = NOFD;
				} else {
					fd = NOFD;
				}
			}
			msg->o.fd = fd;
			break;
		default:
			crash();
			break;
		}
		msg->o.offset = info.offset + msg->i.addr - info.vaddr;
		break;
	default:
		return ENOSYS;
	}
	msg->o.size = min(len_requested, len);
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

__SRCVERSION("memmgr_offset.c $Rev: 174147 $");
