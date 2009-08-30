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

#include <string.h>
#include <sys/syspage.h>

int
walk_asinfo(const char *name, int (*func)(struct asinfo_entry *, char *, void *), void *data) {
	char					*str = SYSPAGE_ENTRY(strings)->data;
	struct asinfo_entry		*as = SYSPAGE_ENTRY(asinfo);
	char					*curr;
	unsigned				num;

	num = _syspage_ptr->asinfo.entry_size / sizeof(*as);
	for( ;; ) {
		if(num == 0) return 1;
		curr = &str[as->name];
		if(name == 0 || strcmp(curr, name) == 0) {
			if(!func(as, curr, data)) return 0;
		}
		++as;
		--num;
	}
}
