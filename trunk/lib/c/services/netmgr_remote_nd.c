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




#include <errno.h>
#include <sys/neutrino.h>
#include "netmgr_send.h"

int netmgr_remote_nd(int remote_nd, int local_nd) {
	netmgr_remote_nd_t			msg;

	if(ND_NODE_CMP(remote_nd, ND_LOCAL_NODE) == 0) {
		return(local_nd);
	}
	if(ND_NODE_CMP(remote_nd, local_nd) == 0) {
		return(ND_LOCAL_NODE);
	}

	msg.i.hdr.type = _IO_MSG;
	msg.i.hdr.combine_len = sizeof msg.i;
	msg.i.hdr.mgrid = _IOMGR_NETMGR;
	msg.i.hdr.subtype = _NETMGR_REMOTE_ND;
	msg.i.remote_nd = remote_nd;
	msg.i.local_nd = local_nd;
	return __netmgr_send(&msg.i, sizeof msg.i, 0, 0, 0, 0);
}

__SRCVERSION("netmgr_remote_nd.c $Rev: 153052 $");
