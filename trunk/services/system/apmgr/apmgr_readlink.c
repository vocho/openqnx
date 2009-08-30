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
 * apmmgr_readlink
 * 
 * Provide resource manager readlink() processing for the memory partitioning
 * module.
 * This function is used to obtain the contents if symbolic links which are used
 * to implement pseudo partitions
 * 
*/

#include "apmgr.h"
#include "apmmgr.h"
#include "apsmgr.h"

/*******************************************************************************
 * apmgr_readlink
 * 
 * This routine provides the readlink implementation for pseudo partitions
 * under "/partition" (but not "/partition/mem")in the name space.
 * 
*/
int apmgr_readlink(resmgr_context_t *ctp, io_readlink_t *msg, RESMGR_HANDLE_T *handle, void *reserved)
{
	apmgr_attr_t  *p;
	int  r;

	if ((p = npath_find((apmgr_attr_t *)LIST_FIRST(root_npart->children), msg->connect.path, NULL)) == NULL)
		return ENOENT;

	if ((r = PART_ATTR_LOCK(p)) != EOK)
		return r;

	if (S_ISLNK(p->attr.mode))
	{
		switch(p->type)
		{
			case part_type_MEMPART_PSEUDO:
			{
				apmmgr_attr_t *real;
				char *link_name = p->symlink;

				if (memcmp(link_name, "/partition/mem/", sizeof("/partition/mem/")-1) == 0)
					link_name += sizeof("/partition/mem/") - 1;

				/* make sure the symlink is still valid */
				if ((real = APMMGR_GETATTR(link_name)) == NULL)
					r = ENOLINK;
				else if (real->type != part_type_MEMPART_REAL)
					r = EBADF;

				break;
			}

			case part_type_SCHEDPART_PSEUDO:
			{
				apsmgr_attr_t *real;
				char *link_name = p->symlink;

				if (memcmp(link_name, "/partition/sched/", sizeof("/partition/sched/")-1) == 0)
					link_name += sizeof("/partition/sched/") - 1;

				/* make sure the symlink is still valid */
				if ((real = APSMGR_GETATTR(link_name)) == NULL)
					r = ENOLINK;
				else if (real->type != part_type_SCHEDPART_REAL)
					r = EBADF;

				break;
			}
			
			default:
				r = EBADF;
				break;
		}
		
		if (r == EOK)
		{
			unsigned eflag = msg->connect.eflag;		
			struct _io_connect_link_reply  *link_reply_msg = (struct _io_connect_link_reply *)msg;
			char *link_string = (char *)&link_reply_msg[1];
			int space = msg->connect.reply_max - sizeof(*link_reply_msg);

			memset(link_reply_msg, 0, sizeof(*link_reply_msg));
			STRLCPY(link_string, p->symlink, space);

			link_reply_msg->path_len = strlen(link_string)+1;
			link_reply_msg->eflag = eflag;
			PART_ATTR_UNLOCK(p);
			return _RESMGR_PTR(ctp, link_reply_msg, sizeof(*link_reply_msg) + link_reply_msg->path_len);
		}
	}
	
	PART_ATTR_UNLOCK(p);
	return EBADF;
}


__SRCVERSION("$IQ: apmmgr_readlink.c,v 1.23 $");

