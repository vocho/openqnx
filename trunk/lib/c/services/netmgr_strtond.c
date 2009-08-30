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
#include <string.h>
#include <sys/neutrino.h>
#include "netmgr_send.h"

int netmgr_strtond(const char *nodename, char **endstr) {
	netmgr_strtond_t		msg;
	int						status;

	msg.i.hdr.type = _IO_MSG;
	msg.i.hdr.combine_len = sizeof msg.i;
	msg.i.hdr.mgrid = _IOMGR_NETMGR;
	msg.i.hdr.subtype = _NETMGR_STRTOND;
	msg.i.len = strlen(nodename);
	msg.i.zero = 0;
	if((status = __netmgr_send(&msg.i, sizeof msg.i, nodename, msg.i.len, &msg.o, sizeof msg.o)) == -1) {
		msg.o.len = 0;
	}
	if(endstr) {
		*endstr = (char *)nodename + msg.o.len;
	}
	if(status == -1) {
		return status;
	}
	return msg.o.nd;
}

__SRCVERSION("netmgr_strtond.c $Rev: 153052 $");
