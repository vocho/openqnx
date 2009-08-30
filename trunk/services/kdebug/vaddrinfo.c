/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
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

#include "kdebug.h"

unsigned
vaddrinfo(struct kdebug_entry *p, uintptr_t vaddr, paddr_t *addr, size_t *size) {
	struct kdebug_info	*kinfo;
	paddr64_t			paddr;
	paddr64_t			psize;
	unsigned			prot;

	kinfo = private->kdebug_info;
	if(kinfo == NULL) {
		prot = cpu_vaddrinfo((void *)vaddr, &paddr, size);
		*addr = paddr;
		return prot;
	}
	if(kinfo->proc_version >= KDEBUG_PROC_HAS_REQUEST) {
		union kd_request	r;

		r.vaddrinfo.hdr.req = KDREQ_VADDRINFO;
		r.vaddrinfo.entry = p;
		r.vaddrinfo.vaddr = vaddr;
		kinfo->request(&r);
		*addr = r.vaddrinfo.paddr;
		*size = r.vaddrinfo.size;
		return r.vaddrinfo.prot;
	}

	// This code can be deleted after a while and just use the 
	// kinfo->request version: 2008/04/24
	prot = kinfo->vaddr_to_paddr2(p, vaddr, &paddr, &psize);
	*addr = paddr;
	*size = psize;
	return prot;
}
