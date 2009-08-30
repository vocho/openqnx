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

struct kerargs_cred_dirty {
	pid_t					pid;
};

static void
kerext_cred_dirty(void *data) {
	struct kerargs_cred_dirty	*kap = data;
	PROCESS						*prp;

	if((prp = lookup_pid(kap->pid)) == NULL) {
		kererr(actives[KERNCPU], ESRCH);
		return;
	}
	lock_kernel();
	cred_dirty(prp);
	SETKSTATUS(actives[KERNCPU], 0);
}

int CredDirty(pid_t pid) {
	struct kerargs_cred_dirty	data;

	data.pid = pid;
	return (__Ring0(kerext_cred_dirty, &data));
}


struct kerargs_cred_get {
	pid_t				pid;
	struct _cred_info	*cip;
};

static void
kerext_cred_get(void *data) {
	struct kerargs_cred_get		*kap = data;
	PROCESS						*prp;

	if((prp = lookup_pid(kap->pid)) == NULL) {
		kererr(actives[KERNCPU], ESRCH);
		return;
	}
	*kap->cip = prp->cred->info;

	lock_kernel();
	SETKSTATUS(actives[KERNCPU], 0);
}

int CredGet(pid_t pid, struct _cred_info *cip) {
	struct kerargs_cred_get	data;

	data.pid = pid;
	data.cip = cip;
	return (__Ring0(kerext_cred_get, &data));
}


struct kerargs_cred_set {
	pid_t				pid;
	struct _cred_info	*cip;
};


static void
kerext_cred_set(void *data) {
	struct kerargs_cred_set	*kap = data;
	unsigned				 status;
	PROCESS					*prp;

	if((prp = lookup_pid(kap->pid)) == NULL) {
		kererr(actives[KERNCPU], ESRCH);
		return;
	}

	lock_kernel();
	cred_dirty(prp);

	//Don't do the cred_set in SETKSTATUS, may use parm twice
	status = cred_set(&prp->cred, kap->cip);
	SETKSTATUS(actives[KERNCPU], status);
}

int CredSet(pid_t pid, struct _cred_info *cip) {
	struct kerargs_cred_set	data;

	data.pid = pid;
	data.cip = cip;
	return (__Ring0( kerext_cred_set, &data));
}

__SRCVERSION("kerext_cred.c $Rev: 153052 $");
