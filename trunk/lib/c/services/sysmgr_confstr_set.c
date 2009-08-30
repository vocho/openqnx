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




#include <string.h>
#include <sys/conf.h>
#include <sys/sysmgr.h>
#include <sys/sysmsg.h>

int sysmgr_confstr_set(int cmd, int name, const char *str) {
	sys_conf_t				msg;
	iov_t					iov[2];

	msg.i.type = _SYS_CONF;	
	msg.i.subtype = _SYS_SUB_SET;
	msg.i.cmd = _CONF_STR | cmd;
	msg.i.name = name;
	msg.i.value = str ? strlen(str) : 0;
	SETIOV(iov + 0, &msg.i, sizeof msg.i);
	SETIOV(iov + 1, str, msg.i.value);
	return MsgSendvnc(SYSMGR_COID, iov, 2, 0, 0);
}

__SRCVERSION("sysmgr_confstr_set.c $Rev: 153052 $");
