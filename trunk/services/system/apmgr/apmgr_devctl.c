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
 * apmgr_devctl
 * 
 * Provide resource manager devctl() processing for the partitioning module
 * 
*/

#include "apmgr.h"
#include "apmmgr.h"
#include "apsmgr.h"


int apmgr_devctl(resmgr_context_t *ctp, io_devctl_t *msg, void *_ocb)
{
	apmgr_ocb_t *ocb = (apmgr_ocb_t *)_ocb;
	void	*data = (void *)(msg + 1);
    int		nbytes = 0;

    switch(msg->i.dcmd)
    {
    	case DCMD_ALL_GETFLAGS:
    	{
			int  ioflag = ocb ? ((iofunc_ocb_t *)ocb)->ioflag : 0;
        	*((int *)data) = (ioflag & ~O_ACCMODE) | ((ioflag - 1) & O_ACCMODE);
        	nbytes = sizeof(ioflag);
        	break;
    	}

		case DCMD_ALL_SETFLAGS:
			if(ocb) {
				ocb->ocb.ioflag = (ocb->ocb.ioflag & ~O_SETFLAG) | (*((int *)data) & O_SETFLAG);
			}
			break;

    	case PART_SETFLAGS:
    	{
    		PROCESS *prp;
    		apxmgr_attr_t *p = (apxmgr_attr_t *)ocb->attr;
    		part_dcmd_flags_t *part_flags = (part_dcmd_flags_t *)data;
			int  r = EOK;

			if ((ocb->apmgr_type != apmgr_type_MEM) &&
				(ocb->apmgr_type != apmgr_type_SCHED)) {
				return EBADF;
			} else if ((ocb->ocb.ioflag & _IO_FLAG_WR) == 0) {
				return EACCES;
			} else if (msg->i.nbytes < sizeof(*part_flags)) {
				return EINVAL;
			} else if ((prp = proc_lock_pid(ocb->pid)) == NULL) {
				return ENOENT;
			}

			if ((r = PART_ATTR_LOCK(p)) != EOK) {
				proc_unlock(prp);
				return r;
			}

			if (p->type == part_type_MEMPART_REAL)
			{
				apmmgr_attr_t *attr = (apmmgr_attr_t *)p;
				memclass_id_t mclass_id = mempart_get_classid(attr->data.mpid);
				mempart_node_t *mp_node = MEMPART_NODEGET(prp, mclass_id);
				
				CRASHCHECK(mp_node == NULL);	// cannot be NULL at this point
				mp_node->flags = *part_flags & (part_flags_COMMON_MASK | part_flags_MEM_MASK);
			}
			else if (p->type == part_type_SCHEDPART_REAL)
			{
				schedpart_node_t *sp_node = SCHEDPART_NODEGET(prp);
				
				CRASHCHECK(sp_node == NULL);	// cannot be NULL at this point
				sp_node->flags = *part_flags & (part_flags_COMMON_MASK | part_flags_SCHED_MASK);
			}
			else
			{
				PART_ATTR_UNLOCK(p);
				proc_unlock(prp);
				return EBADF;
			}

			PART_ATTR_UNLOCK(p);
			proc_unlock(prp);
    		break;
    	}

    	case PART_GETFLAGS:
    	{
    		PROCESS *prp;
    		apxmgr_attr_t *p = (apxmgr_attr_t *)ocb->attr;
    		part_dcmd_flags_t *part_flags = (part_dcmd_flags_t *)data;
			int  r = EOK;

			if ((ocb->apmgr_type != apmgr_type_MEM) &&
				(ocb->apmgr_type != apmgr_type_SCHED)) {
				return EBADF;
			} else if ((ocb->ocb.ioflag & _IO_FLAG_RD) == 0) {
				return EACCES;
			} else if (msg->o.nbytes < sizeof(*part_flags)) {
				return EOVERFLOW;
			} else if ((prp = proc_lock_pid(ocb->pid)) == NULL) {
				return ENOENT;
			}

			if ((r = PART_ATTR_LOCK(p)) != EOK) {
				proc_unlock(prp);
				return r;
			}

			if (p->type == part_type_MEMPART_REAL)
			{
				apmmgr_attr_t *attr = (apmmgr_attr_t *)p;
				memclass_id_t mclass_id = mempart_get_classid(attr->data.mpid);
				mempart_node_t *mp_node = MEMPART_NODEGET(prp, mclass_id);
				
				CRASHCHECK(mp_node == NULL);	// cannot be NULL at this point
				*part_flags = mp_node->flags;
			}
			else if (p->type == part_type_SCHEDPART_REAL)
			{
				schedpart_node_t *sp_node = SCHEDPART_NODEGET(prp);
				
				CRASHCHECK(sp_node == NULL);	// cannot be NULL at this point
				*part_flags = sp_node->flags;
			}
			else
			{
				PART_ATTR_UNLOCK(p);
				proc_unlock(prp);
				return EBADF;
			}

			PART_ATTR_UNLOCK(p);
			proc_unlock(prp);
			nbytes = sizeof(*part_flags);
    		break;
    	}

		case APMGR_GET_PROC_MEMINFO:
		{
    		proc_memclass_sizeinfo_t *info_out = (proc_memclass_sizeinfo_t *)data;
    		unsigned int  num_ids = 0;
			unsigned int  num_entries;
			PROCESS *prp;
			part_list_t *mpart_list;

			if (ocb->apmgr_type != apmgr_type_PROC_PART) {
				return EBADF;
			} else if ((ocb->ocb.ioflag & _IO_FLAG_RD) == 0) {
				return EACCES;
			} else if (info_out->num_entries < 0) {
				return EINVAL;
			} else if (msg->o.nbytes < PROC_MEMCLASS_SIZEINFO_T_SIZE(info_out->num_entries)) {
				return EOVERFLOW;
			} else if ((prp = proc_lock_pid(ocb->pid)) == NULL) {
				return ENOENT;
			}

			num_ids = MEMPART_GETLIST(prp, NULL, 0, NULL, NULL);
			num_entries = min(num_ids, info_out->num_entries);
			
			if (((mpart_list = alloca(PART_LIST_T_SIZE(num_ids))) == NULL) ||
				(MEMPART_GETLIST(prp, mpart_list, num_ids, mempart_flags_t_GETLIST_ALL, NULL) != 0))
			{
				proc_unlock(prp);
				return ENOMEM;
			}
			proc_unlock(prp);		// done with prp now

			if (num_entries > 0)
			{
				unsigned i;
				for (i=0; i<num_entries; i++)
				{
					mempart_node_t *mp;
					info_out->si[i].mclass_id = mempart_get_classid(mpart_list->i[i].id);
					mp = MEMPART_NODEGET(prp, info_out->si[i].mclass_id);
					info_out->si[i].used = (_MEMSIZE_T_)((mp == NULL) ? 0 : mp->mem_used);
					info_out->si[i].free = free_mem(ocb->pid, mpart_list->i[i].id);
				}
			}
			if (num_entries < num_ids) {
				info_out->num_entries = -(num_ids - num_entries);
			} else {
				info_out->num_entries = num_ids;
			}

			nbytes = PROC_MEMCLASS_SIZEINFO_T_SIZE(num_entries);
    		break;
		}

    	default:
			/*
			 * the following allows devctl() operations on
			 * "/proc/<pid>/partition/mem/<memclass>/<partition>"
			 * and "/proc/<pid>/partition/sched/<partition>"
			*/
			if (ocb->apmgr_type == apmgr_type_MEM)
			{
				iofunc_ocb_t mpart_ocb;
				mpart_ocb = ocb->ocb;
				mpart_ocb.attr = &ocb->attr->attr;
				return apmmgr_devctl(ctp, msg, &mpart_ocb);
			}
			else if (ocb->apmgr_type == apmgr_type_SCHED)
			{
				iofunc_ocb_t spart_ocb;
				spart_ocb = ocb->ocb;
				spart_ocb.attr = &ocb->attr->attr;
				return apsmgr_devctl(ctp, msg, &spart_ocb);
			}
			else {
				return ENOSYS;
			}
    }

    if(nbytes)
    {
        msg->o.ret_val = EOK;
        return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + nbytes);
    }
    return EOK;
}


__SRCVERSION("$IQ: apmgr_devctl.c,v 1.23 $");

