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





#include <sys/pm.h>
#include <sys/iomsg.h>

int pm_get_power(int fd, pm_power_attr_t *attr) {
	io_power_t			msg;

	msg.i.type = _IO_POWER;
	msg.i.combine_len = sizeof msg;
	msg.i.subtype = _IO_POWER_GET;
	msg.i.mode = 0;
	msg.i.flags = 0;

	return MsgSend(fd, &msg.i, sizeof msg.i, attr, sizeof *attr);
}

__SRCVERSION("pm_get_power.c $Rev: 153052 $");
