/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. All Rights Reserved.
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

int
debugpath(struct kdebug_entry *p, char *buf, unsigned bufsize) {
	struct kdebug_info	*kinfo;

	kinfo = private->kdebug_info;
	if(kinfo == NULL || p == NULL) {
		return -1;
	}
	if(kinfo->proc_version >= KDEBUG_PROC_HAS_REQUEST) {
		union kd_request	r;

		r.path.hdr.req = KDREQ_PATH;
		r.path.entry = p;
		r.path.buff = buf;
		r.path.len = bufsize;
		kinfo->request(&r);
		return r.path.len;
	}

	// This code can be deleted after a while and just use the 
	// kinfo->request version: 2008/04/24
	return kinfo->debug_path(p, buf, bufsize);
}
