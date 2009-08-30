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
 * apmgr_unlink
 * 
 * Provide resource manager unlink() processing for the partitioning module
 * 
*/

#include "apmgr.h"

int apmgr_proc_unlink(resmgr_context_t *ctp, io_unlink_t *msg, void *handle, void *reserved)
{
	pid_t  pid;

	CRASHCHECK(proc_lookup_pid(ctp->info.pid) == NULL);

	/* unless its a 'pid', we don't handle it */
	if ((msg->connect.path == NULL) || (*msg->connect.path == '\0'))
		return ENOSYS;

	/*
	 * the first '/' delimited character sequence MUST represent a valid process
	 * id or it is not handled by this resource manager
	*/
	if (!isProcessPath(msg->connect.path, &pid))
		return ENOSYS;

	switch (msg->connect.subtype)
	{
		case _IO_CONNECT_UNLINK:
		{
			if (proc_lookup_pid(pid) == NULL)
				return ENOENT;

			/* does someone have it opened ? */
			if (dummy_apmgr_attr.count != 0)
				return EBUSY;

			return EOK;
		}
		default:
			return ENOSYS;
	}
}


__SRCVERSION("$IQ: apmgr_unlink.c,v 1.23 $");

