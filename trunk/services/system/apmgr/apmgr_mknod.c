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

/*==============================================================================
 * 
 * apmgr_mknod
 * 
 * Provide resource manager handling for the common partitioning module.
 * This entry point allows the use of mkdir for the creation of a partition
 * group name.
 * 
*/

#include "apmgr.h"


/*******************************************************************************
 * apmgr_mknod
 * 
*/
int apmgr_mknod(resmgr_context_t *ctp, io_mknod_t *msg, RESMGR_HANDLE_T *handle, void *reserved)
{
	switch (msg->connect.subtype)
	{
		case _IO_CONNECT_MKNOD:
		{
			unsigned r; 
			msg->connect.subtype = _IO_CONNECT_OPEN;
			r = apmgr_open(ctp, (io_open_t *)msg, handle, reserved);
			msg->connect.subtype = _IO_CONNECT_MKNOD;
			return r;
		}
		
		default:
			return ENOSYS;
	}
}

__SRCVERSION("$IQ: apmgr_mknod.c,v 1.23 $");

