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
extern const char		timestamp[];

static const struct kdebug_private private = {
	0,
	actives,
	aspaces_prp,
	(void **)&intrespsave,
	(struct kdebug_vector_entry *)&process_vector,
	KDEBUG_VECTOR_VER,
	offsetof(THREAD, tid),
	offsetof(THREAD, reg),
	offsetof(THREAD, process),
	offsetof(PROCESS, pid),
	offsetof(PROCESS, debug_name),
	sizeof(void *) * 3,				// must match kernel.s
	offsetof(PROCESS, threads),
	offsetof(PROCESS, kdebug),
	offsetof(THREAD, priority),
	offsetof(PROCESS, memory),
};

SMP_SPINVAR(static, kdebug_slock);


unsigned 
kdebug_vtop(struct kdebug_entry *entry, uintptr_t vaddr,
			paddr64_t *paddrp, paddr64_t *sizep) {
	PROCESS					*prp;
	paddr_t					paddr;
	size_t					size;
	unsigned				r;

	if(entry) {
		prp = (entry->type == KDEBUG_TYPE_PROCESS) ? (PROCESS *)entry->ptr : 0;
	} else {
		if(!(prp = aspaces_prp[KERNCPU])) {
			prp = sysmgr_prp;
		}
	}
	if(!prp) {
		return 0;
	}

	r = memmgr.vaddrinfo(prp, vaddr, &paddr, &size, VI_KDEBUG);
	*paddrp = paddr;
	if(size < *sizep) *sizep = size;
	return r;
}

int (*kdebug_path_callout)(struct kdebug_entry *entry, char *buff, int buffsize);

int
kd_request(union kd_request *r) {
	unsigned	i;

	switch(r->hdr.req) {
	case KDREQ_VADDRINFO:
		r->vaddrinfo.prot = 
			kdebug_vtop(r->vaddrinfo.entry, r->vaddrinfo.vaddr,
						&r->vaddrinfo.paddr, &r->vaddrinfo.size);
		break;
	case KDREQ_PATH:
		if(kdebug_path_callout != NULL) {
			r->path.len = kdebug_path_callout(r->path.entry, r->path.buff, r->path.len);
		} else {
			r->path.len = 0;
		}
		break;
	case KDREQ_PARKIT:	
		for(i = 0; i < NUM_PROCESSORS; ++i) {
			if((i != RUNCPU) && alives[i]) {
				SENDIPI(i, IPI_PARKIT);
			}
		}
		break;
	case KDREQ_INVOKE:	
		if(kdinvoke_hook != NULL) {
			return kdinvoke_hook(r);
		}
		// Fall through
	default:
		return ENOSYS;
	}
	return EOK;
}


static struct kdebug_info *
register_info(void) {
	struct kdebug_info	*info;

	info = privateptr->kdebug_info;
	if(info == NULL) {
		info = _scalloc(sizeof *info);

		info->proc_version = KDEBUG_PROC_CURRENT;
		info->request = outside_kd_request;
		// We can get rid of outside_kdebug_path & vaddr_to_paddr2 
		// after a while: 2008/04/24
		info->debug_path = outside_kdebug_path;
		info->vaddr_to_paddr2 = outside_vaddr_to_paddr;
		info->kdbg_private = &private;
		info->timestamp = timestamp;

		privateptr->kdebug_info = info;
	}
	return info;
}


void 
kdebug_init(int (*kdebug_path)(struct kdebug_entry *entry, char *buff, int buffsize)) {

	kdebug_path_callout = kdebug_path;
	(void) register_info();
}


//	LINK2_BEG(kdebug_info.resident_list, &prp->kdebug, struct kdebug_entry);
int 
kdebug_attach(struct kdebug_entry *entry, int resident) {
	struct kdebug_info		*info;
	struct kdebug_entry		**head;

	info = register_info();

	if(!entry->prev) {
		head = resident ? &info->resident_list : &info->process_list;
		INTR_LOCK(&kdebug_slock);
		entry->prev = head;
		if((entry->next = *head)) {
			entry->next->prev = &entry->next;
		}
		*head = entry;
		INTR_UNLOCK(&kdebug_slock);
	}
	info->flags |= KDEBUG_FLAG_DIRTY;
	return 0;
}


int 
kdebug_unlink(struct kdebug_entry *entry) {
	INTR_LOCK(&kdebug_slock);
	if((*entry->prev = entry->next)) {
		entry->next->prev = entry->prev;
	}
	entry->prev = 0;
	INTR_UNLOCK(&kdebug_slock);
	return 0;
}


static void 
do_kdebug_detach(void *data) {
	struct kdebug_entry	*entry = data;

	if(entry->flags & KDEBUG_FLAG_DEBUGGED) {
		struct kdebug_callback *call;

		entry->flags |= KDEBUG_FLAG_UNLOADED;
		register_info()->flags |= KDEBUG_FLAG_DIRTY;
		call = privateptr->kdebug_call;
		if((call != NULL) && (call->update_plist != NULL)) {
			call->update_plist(entry);
		}
	}
}


int 
kdebug_detach(struct kdebug_entry *entry) {
	__Ring0(do_kdebug_detach, entry);
	return 0;
}


int 
kdebug_watch_entry(struct kdebug_entry *entry, uintptr_t ip) {
	struct kdebug_callback *call;

	call = privateptr->kdebug_call;
	if(call != NULL) {
		if(call->update_plist != NULL) {
			//NYI: pass in process pointer
			call->update_plist(NULL);
		}
		if(call->watch_entry(entry, ip) == -1) {
			(void)kdprintf("\nStarted process, main=0x%x\n", ip);
			DebugKDBreak();
		}
	}
	return 0;
}


void
kdebug_kdump_private(const struct kdump_private *kdump) {
	register_info()->kdump_private = kdump;
}

__SRCVERSION("nano_kerdebug.c $Rev: 207533 $");
