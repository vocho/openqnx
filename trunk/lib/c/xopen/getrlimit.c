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
#include <unistd.h>
#include <sys/procmsg.h>
#include <sys/resource.h>

/*
DESCRIPTION 
When using the getrlimit() function, if a resource limit can be 
represented correctly in an object of type rlim_t then its 
representation is returned; otherwise if the value of the resource limit 
is equal to that of the corresponding saved hard limit the value 
returned is RLIM_SAVED_MAX; otherwise the value returned is 
RLIM_SAVED_CUR. 

The determination of whether a limit can be correctly represented in an 
object of type rlim_t is implementation-dependent. For example, some 
implementations permit a limit whose value is greater than RLIM_INFINITY 
and others do not. 
*/

int  getrlimit64(int resource, struct rlimit64 *rlp) {
	proc_resource_getlimit_t		msg;

	msg.i.type = _PROC_RESOURCE;
	msg.i.subtype = _PROC_RESOURCE_GETLIMIT;
	msg.i.pid = 0;
	msg.i.count = 1;
	msg.i.resource[0] = resource;
	return MsgSendnc(PROCMGR_COID, &msg.i, sizeof msg.i, rlp, sizeof *rlp);
}

int  getrlimit(int resource, struct rlimit *rlp) {
	struct rlimit64			*rlp64 = (struct rlimit64 *)rlp;

	if(getrlimit64(resource, rlp64) == -1) {
		return -1;
	}

	if(rlp64->rlim_cur == RLIM64_SAVED_CUR) {
		rlp->rlim_cur = RLIM_SAVED_CUR;
	} else if(rlp64->rlim_cur == RLIM64_SAVED_MAX) {
		rlp->rlim_cur = RLIM_SAVED_MAX;
	} else if(rlp64->rlim_cur == RLIM64_INFINITY) {
		rlp->rlim_cur = RLIM_INFINITY;
	} else if(rlp64->rlim_cur < RLIM_SAVED_CUR) {
		rlp->rlim_cur = rlp64->rlim_cur;
	} else if(rlp64->rlim_cur == rlp64->rlim_max) {
		// 64-bit value doesn't fit in 32-bit rlimit, adjust values as per UNIX98
		rlp->rlim_cur = RLIM_SAVED_MAX;
	} else {
		rlp->rlim_cur = RLIM_SAVED_CUR;
	}

	if(rlp64->rlim_max == RLIM64_SAVED_CUR) {
		rlp->rlim_max = RLIM_SAVED_CUR;
	} else if(rlp64->rlim_max == RLIM64_SAVED_MAX) {
		rlp->rlim_max = RLIM_SAVED_MAX;
	} else if(rlp64->rlim_max == RLIM64_INFINITY) {
		rlp->rlim_max = RLIM_INFINITY;
	} else if(rlp64->rlim_max < RLIM_SAVED_CUR) {
		rlp->rlim_max = rlp64->rlim_max;
	} else {
		// 64-bit value doesn't fit in 32-bit rlimit, adjust values as per UNIX98
		rlp->rlim_max = RLIM_SAVED_MAX;
	}

	return 0;
}

__SRCVERSION("getrlimit.c $Rev: 153052 $");
