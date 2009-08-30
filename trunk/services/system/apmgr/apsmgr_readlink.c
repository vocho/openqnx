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
 * apsmgr_readlink
 * 
 * Provide resource manager readlink() processing for the scheduler partitioning
 * module.
 * This function is used to obtain the contents if symbolic links which are used
 * to implement pseudo partitions
 * 
*/

#include "apsmgr.h"


/*******************************************************************************
 * apsmgr_readlink
 * 
 * This routine provides the readlink implementation for scheduler partitioning.
 * This is used for pseudo partitions.
 * 
*/
int apsmgr_readlink(resmgr_context_t *ctp, io_readlink_t *msg, RESMGR_HANDLE_T *handle, void *reserved)
{
	apsmgr_attr_t  *mp;
	int  r;

	if ((mp = spath_find((apsmgr_attr_t *)LIST_FIRST(root_spart->children), msg->connect.path, NULL)) == NULL)
		return ENOENT;

	if ((r = PART_ATTR_LOCK(mp)) != EOK)
		return r;

	if (S_ISLNK(mp->attr.mode) && (mp->type == part_type_SCHEDPART_PSEUDO))
	{
		apsmgr_attr_t *mp_real;
		char *link_name = mp->data.symlink;

		if (memcmp(link_name, "/partition/sched/", sizeof("/partition/sched/")-1) == 0)
			link_name += sizeof("/partition/sched/") - 1;

		/* make sure the symlink is still valid */
		if ((mp_real = spath_find((apsmgr_attr_t *)LIST_FIRST(root_spart->children), link_name, NULL)) == NULL)
			r = ENOLINK;
		else if (mp_real->type != part_type_SCHEDPART_REAL)
			r = EBADF;
		else
		{
			unsigned eflag = msg->connect.eflag;		
			struct _io_connect_link_reply  *link_reply_msg = (struct _io_connect_link_reply *)msg;
			char *link_string = (char *)&link_reply_msg[1];
			int space = msg->connect.reply_max - sizeof(*link_reply_msg);

			memset(link_reply_msg, 0, sizeof(*link_reply_msg));
			STRLCPY(link_string, mp->data.symlink, space);

			link_reply_msg->path_len = strlen(link_string)+1;
			link_reply_msg->eflag = eflag;
			PART_ATTR_UNLOCK(mp);
			return _RESMGR_PTR(ctp, link_reply_msg, sizeof(*link_reply_msg) + link_reply_msg->path_len);
		}
	}
	else
		r = EBADF;
	
	PART_ATTR_UNLOCK(mp);
	return r;
}


__SRCVERSION("$IQ: apsmgr_readlink.c,v 1.23 $");

