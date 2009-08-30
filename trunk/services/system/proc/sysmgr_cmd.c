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

int sysmgr_cmd(resmgr_context_t *ctp, sys_cmd_t *msg) {
	if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
		ENDIAN_SWAP16(&msg->i.cmd);
		ENDIAN_SWAP32(&msg->i.mode);
	}
	switch(msg->i.cmd) {
	case _SYS_CMD_REBOOT:
		if(!proc_isaccess(0, &ctp->info)) {
			return EPERM;
		}
		RebootSystem(0);
		break;
	case _SYS_CMD_CPUMODE:
		if(!proc_isaccess(0, &ctp->info)) {
			return EPERM;
		}
		return SysCpumode(msg->i.mode);
	default:
		break;
	}
	return ENOSYS;
}

__SRCVERSION("sysmgr_cmd.c $Rev: 153052 $");
