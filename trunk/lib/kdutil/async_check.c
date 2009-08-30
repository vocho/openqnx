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

#include <sys/neutrino.h>
#include "kdintl.h"

extern int	outside_timer_reload(struct syspage_entry *, struct qtime_entry *);

static int 	(*old_timer_reload)(struct syspage_entry *, struct qtime_entry *);
static int 	(*break_detect)(struct syspage_entry *);

static int
dummy_timer_reload(struct syspage_entry *sysp, struct qtime_entry *qtime) {
	return 1;
}

void
async_check_init(unsigned channel) {
	struct callout_entry	*callout;

	callout = SYSPAGE_ENTRY(callout);
	break_detect = callout->debug[channel].break_detect;
	if(break_detect != NULL) {
		old_timer_reload = callout->timer_reload;
		if(old_timer_reload == NULL) old_timer_reload = dummy_timer_reload;
		callout->timer_reload = outside_timer_reload;
	}
}


int
async_timer_reload(struct syspage_entry *sysp, struct qtime_entry *qtime) {
	int	ret;

	ret = old_timer_reload(sysp, qtime);
	if(break_detect(_syspage_ptr)) {
		DebugKDBreak();
	}
	return(ret);
}


int
async_check_active(void) {
	return (break_detect != NULL) ? break_detect(_syspage_ptr) : 0;
}
