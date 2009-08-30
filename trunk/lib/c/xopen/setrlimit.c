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




#undef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS	32
#include <errno.h>
#include <sys/resource.h>
#include <sys/procmsg.h>

/*
DESCRIPTION 
When using the setrlimit() function, if the requested new limit is 
RLIM_INFINITY the new limit will be "no limit"; otherwise if the 
requested new limit is RLIM_SAVED_MAX the new limit will be the 
corresponding saved hard limit; otherwise if the requested new limit is 
RLIM_SAVED_CUR the new limit will be the corresponding saved soft limit; 
otherwise the new limit will be the requested value. In addition, if the 
corresponding saved limit can be represented correctly in an object of 
type rlim_t then it will be overwritten with the new limit. 

The result of setting a limit to RLIM_SAVED_MAX or RLIM_SAVED_CUR is 
unspecified unless a previous call to getrlimit() returned that value as 
the soft or hard limit for the corresponding resource limit. 

The determination of whether a limit can be correctly represented in an 
object of type rlim_t is implementation-dependent. For example, some 
implementations permit a limit whose value is greater than RLIM_INFINITY 
and others do not. 
*/

int  setrlimit64(int resource, const struct rlimit64 *rlp) {
	proc_resource_setlimit_t	msg;

	msg.i.type = _PROC_RESOURCE;
	msg.i.subtype = _PROC_RESOURCE_SETLIMIT;
	msg.i.pid = 0;
	msg.i.count = 1;
	msg.i.entry[0].resource = resource;
	msg.i.entry[0].limit = *rlp;
	return MsgSendnc(PROCMGR_COID, &msg.i, sizeof msg.i, 0, 0);
}

int  setrlimit(int resource , const struct rlimit *rlp) {
	struct rlimit64		limit;
	struct rlimit64		old;

	/* These values need the saved values for unix98 LFS support */
	if(		rlp->rlim_cur == RLIM_SAVED_MAX ||
			rlp->rlim_cur == RLIM_SAVED_CUR ||
			rlp->rlim_max == RLIM_SAVED_MAX ||
			rlp->rlim_max == RLIM_SAVED_CUR) {
		if(getrlimit64(resource, &old) == -1) {
			return -1;
		}
	}

	if(rlp->rlim_cur == RLIM_INFINITY) {
		limit.rlim_cur = RLIM64_INFINITY;
	} else if(rlp->rlim_cur == RLIM_SAVED_MAX) {
		limit.rlim_cur = old.rlim_max;
	} else if(rlp->rlim_cur == RLIM_SAVED_CUR) {
		limit.rlim_cur = old.rlim_cur;
	} else {
		limit.rlim_cur = rlp->rlim_cur;
	}

	if(rlp->rlim_max == RLIM_INFINITY) {
		limit.rlim_max = RLIM64_INFINITY;
	} else if(rlp->rlim_max == RLIM_SAVED_MAX) {
		limit.rlim_max = old.rlim_max;
	} else if(rlp->rlim_max == RLIM_SAVED_CUR) {
		limit.rlim_max = old.rlim_max;
	} else {
		limit.rlim_max = rlp->rlim_max;
	}

	return setrlimit64(resource, &limit);
}

__SRCVERSION("setrlimit.c $Rev: 153052 $");
