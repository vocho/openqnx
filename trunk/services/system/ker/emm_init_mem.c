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
#include <sys/mman.h>
#include <unistd.h>

static int
add_mem(struct asinfo_entry *as, char *name, void *data) {
	(void) emm_pmem_add(as->start, (as->end - as->start) + 1);
	return 1;
}

void 
emm_init_mem(int phase) {
	if(phase == 0) {
		walk_asinfo("sysram", add_mem, NULL);
	}
}

__SRCVERSION("emm_init_mem.c $Rev: 153052 $");
