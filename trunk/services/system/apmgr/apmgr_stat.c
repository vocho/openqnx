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
 * apmgr_stat
 * 
 * Provide resource manager stat() processing for the partitioning module
 * 
*/

#include "apmgr.h"
#include "apmmgr.h"
#include "apsmgr.h"

int apmgr_stat(resmgr_context_t *ctp, io_stat_t *msg, void *_ocb)
{
	apmgr_ocb_t *ocb = (apmgr_ocb_t *)_ocb;
	PROCESS  *prp = NULL;
	int r = EOK;

	if (ocb == NULL) {
		return ENOENT;
	} else if ((ocb->pid > 0) && ((prp = proc_lock_pid(ocb->pid)) == NULL)) {
		return EBADF;
	}

	switch (ocb->apmgr_type)
	{
		/*
		 * for apmgr_type_NONE, st_size = 1 (ie. partition/)
		 * for apmgr_type_PROC_PART, st_size = 2 (ie. partition/sched/ and partition/mem/)
		 * for apmgr_type_PART, st_size = 2 + n (ie. partition/sched/ and partition/mem/ + whatever
		 * 											group and pseudo names exist)
		*/
		case apmgr_type_NONE:
		{
			if ((r = PART_ATTR_LOCK(ocb->attr)) == EOK)
			{
				if ((r = iofunc_stat(ctp, ocb->ocb.attr, &msg->o)) == EOK) {
					msg->o.st_size = 1;
				}
				PART_ATTR_UNLOCK(ocb->attr);
			}
			break;
		}

		case apmgr_type_PART:
		{
			if ((r = PART_ATTR_LOCK(ocb->attr)) == EOK)
			{
				if ((r = iofunc_stat(ctp, &ocb->attr->attr, &msg->o)) == EOK)
				{
					msg->o.st_size = 0;
#if !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
					msg->o.st_size_hi = 0;
#endif
					msg->o.st_size += (MEMPART_INSTALLED() ? 1 : 0);
					msg->o.st_size += (SCHEDPART_INSTALLED() ? 1 : 0);
					msg->o.st_size += LIST_COUNT(ocb->attr->children);
				}
				PART_ATTR_UNLOCK(ocb->attr);
			}
			break;
		}

		case apmgr_type_PROC_PART:
		{
			if ((r = PART_ATTR_LOCK(ocb->attr)) == EOK)
			{
				if ((r = iofunc_stat(ctp, ocb->ocb.attr, &msg->o)) == EOK)
				{
					msg->o.st_size = 0;
					msg->o.st_size += (MEMPART_INSTALLED() ? 1 : 0);
					msg->o.st_size += (SCHEDPART_INSTALLED() ? 1 : 0);
				}
				PART_ATTR_UNLOCK(ocb->attr);
			}
			break;
		}

		case apmgr_type_MEM:
		{
			struct stat  s;
			/* can't hold the attr lock since mpmgr_get_st_size() will grab it */
			if ((r = mpmgr_get_st_size((apmmgr_attr_t *)ocb->attr, &s)) == EOK)
			{
				if ((r = PART_ATTR_LOCK(ocb->attr)) == EOK)
				{
					if ((r = iofunc_stat(ctp, &ocb->attr->attr, &msg->o)) == EOK) {
						msg->o.st_size = s.st_size;
					}
					PART_ATTR_UNLOCK(ocb->attr);
				}
			}
			break;
		}

		case apmgr_type_SCHED:
		{
			struct stat  s;
			/* can't hold the attr lock since spmgr_get_st_size() will grab it */
			if ((r = spmgr_get_st_size((apsmgr_attr_t *)ocb->attr, &s)) == EOK)
			{
				if ((r = PART_ATTR_LOCK(ocb->attr)) == EOK)
				{
					if ((r = iofunc_stat(ctp, &ocb->attr->attr, &msg->o)) == EOK) {
						msg->o.st_size = s.st_size;
#if !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
						msg->o.st_size_hi = 0;
#endif
					}
					PART_ATTR_UNLOCK(ocb->attr);
				}
			}
			break;
		}
		
		case apmgr_type_NAME:
		{
			if ((r = PART_ATTR_LOCK(ocb->attr)) == EOK)
			{
				if ((r = iofunc_stat(ctp, &ocb->attr->attr, &msg->o)) == EOK)
				{
					switch(ocb->attr->type)
					{
						case part_type_ROOT:
						case part_type_GROUP:
							msg->o.st_size = LIST_COUNT(ocb->attr->children);
							break;
						case part_type_MEMPART_PSEUDO:
						case part_type_SCHEDPART_PSEUDO:
							msg->o.st_size = strlen(ocb->attr->symlink);
							break;
						default:
							r = EBADF;
							break;
					}
				}
				PART_ATTR_UNLOCK(ocb->attr);
			}
			break;
		}

		default:
			crash();
	}

	if (prp != NULL) proc_unlock(prp);
	return (r != EOK) ? r : _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}


__SRCVERSION("$IQ: apmgr_stat.c,v 1.23 $");

