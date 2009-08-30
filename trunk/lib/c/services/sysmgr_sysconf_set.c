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




#include <sys/conf.h>
#include <sys/sysmgr.h>
#include <sys/sysmsg.h>

int sysmgr_sysconf_set(int cmd, int name, long value) {
	sys_conf_t				msg;

	msg.i.type = _SYS_CONF;	
	msg.i.subtype = _SYS_SUB_SET;
	msg.i.cmd = _CONF_NUM | cmd;
	msg.i.name = name;
	msg.i.value = value;
	return MsgSendnc(SYSMGR_COID, &msg.i, sizeof msg.i, 0, 0);
}

__SRCVERSION("sysmgr_sysconf_set.c $Rev: 153052 $");
