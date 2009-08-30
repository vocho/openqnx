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


struct kerargs_limits_get {
	uid_t		uid;
	LIMITS		*limits;
};

void 
kerext_limits_get(void *data) {
	struct kerargs_limits_get *kap = data;
	THREAD	*act = actives[KERNCPU];
	LIMITS	*lip;

	if((lip = lookup_limits(kap->uid)) == NULL) {
		kererr(act, ESRCH);
		return;
	}

	*kap->limits = *lip;
	lock_kernel();
	SETKSTATUS(act,0);
}

int
LimitsGet(uid_t uid, LIMITS *limits) {
	struct kerargs_limits_get		data;

	data.uid = uid;
	data.limits = limits;
	return(__Ring0(kerext_limits_get, &data));
}


struct kerargs_limits_set {
	uid_t			uid;
	unsigned		**maxes;
};

void 
kerext_limits_set(void *data) {
	struct kerargs_limits_set *kap = data;
	THREAD	*act = actives[KERNCPU];
	LIMITS	*lip;

	if((lip = lookup_limits(kap->uid)) == NULL) {
		lock_kernel();
		if((lip = object_alloc(NULL, &limits_souls)) == NULL) {
			kererr(act, ESRCH);
			return;
		}

		lip->next = limits_list;
		limits_list = lip;
	}

	memcpy(lip->max, kap->maxes, sizeof(lip->max));

	lock_kernel();
	SETKSTATUS(act,0);
}

int
LimitsSet(uid_t uid, unsigned **maxes) {
	struct kerargs_limits_set		data;

	data.uid = uid;
	data.maxes = maxes;
	return(__Ring0(kerext_limits_set, &data));
}

__SRCVERSION("kerext_limits.c $Rev: 153052 $");
