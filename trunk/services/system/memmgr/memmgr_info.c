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

#include <sys/memmsg.h>
#include "mm_internal.h"

static int
get_flags(resmgr_context_t *ctp, int fd) {
	struct _msg_info		info;
	void					*handle;
	const resmgr_io_funcs_t	*funcs;
	OBJECT					*object;
	mem_map_t				msg;

	info = ctp->info;
	info.coid = fd;

	funcs = _resmgr_iofuncs(ctp, &info);
	if(funcs == NULL) crash();
	handle = _resmgr_ocb(ctp, &info);
	msg.i.flags = 0;
	if(devmem_check(funcs, &msg, handle, &object) == -1) crash();
	return msg.i.flags;
}


int 
memmgr_info(resmgr_context_t *ctp, PROCESS *prp, mem_info_t *msg) {
	OBJECT		*obp;
	int			r;
	unsigned	flags;
	paddr_t		size;

	if(msg->i.fd != NOFD) {
		r = memmgr_find_object(ctp, prp, msg->i.fd, NULL, &obp);
		switch(r) {
		case EOK:	
			if(obp != NULL) break;
			// fall through
		case ENOTSUP:
			r = ENODEV;
			// fall through
		default:
			return r;
		}
		switch(obp->hdr.type) {
		case OBJECT_MEM_TYPED:	
			flags = get_flags(ctp, msg->i.fd);
			if(flags & IMAP_TYMEM_ALLOCATE) {
				tymem_free_info(obp, &size, NULL);
			} else if(flags & IMAP_TYMEM_ALLOCATE_CONTIG) {
				tymem_free_info(obp, NULL, &size);
			} else {
				size = 0;
			}
			memset(&msg->o, 0x00, sizeof msg->o);
			msg->o.info.posix_tmi_length = size;
			memobj_unlock(obp);
			return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
		default:
			break;
		}
		
		memobj_unlock(obp);
		return ENODEV;
	}
	if(!(msg->i.flags & MAP_ANON)) {
		switch(msg->i.flags & MAP_TYPE) {
		case MAP_PRIVATEANON:
		case MAP_PRIVATE:
			break;
		case MAP_SHARED:
			return EINVAL;
		default:
			break;
		}
	}
	if(msg->i.flags & MAP_PHYS) {
		/* @@@ Don't support returning largest chuck yet */
		return EINVAL;
	}

	memset(&msg->o, 0x00, sizeof msg->o);
	msg->o.info.posix_tmi_length = mem_free_size;
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

__SRCVERSION("memmgr_info.c $Rev: 174147 $");
