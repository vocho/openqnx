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
 * apmmgr_devctl
 * 
 * Provide resource manager devctl() processing for the memory partitioning
 * module
 * 
*/

#include "apmmgr.h"


typedef char	mempart_path_t[PATH_MAX+1];

int apmmgr_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *_ocb)
{
	apmmgr_ocb_t *apmmgr_ocb = (apmmgr_ocb_t *)_ocb;
	iofunc_ocb_t *ocb = &apmmgr_ocb->ocb;
	void	*data = (void *)(msg + 1);
    int		nbytes = 0;

    switch(msg->i.dcmd)
    {
    	// FIX ME - these SET/GET flags should be handled by iofunc_devctl_default()
    	case DCMD_ALL_GETFLAGS:
    	{
			if ((ocb->ioflag & _IO_FLAG_RD) == 0) {
				return EACCES;
			} else if (msg->o.nbytes < sizeof(ocb->ioflag)) {
				return EOVERFLOW;
			} else {
				int  ioflag = ocb ? ((iofunc_ocb_t *)ocb)->ioflag : 0;
				*((int *)data) = (ioflag & ~O_ACCMODE) | ((ioflag - 1) & O_ACCMODE);
				nbytes = sizeof(ioflag);
				break;
			}
    	}
		case DCMD_ALL_SETFLAGS:
/* FIX ME - appears not to be being obeyed .. should it ?
			if ((ocb->ioflag & _IO_FLAG_WR) == 0)
				return EBADF;
*/			
			if (msg->i.nbytes < sizeof(ocb->ioflag)) {
				return EINVAL;
			}
			if(ocb) {
				ocb->ioflag = (ocb->ioflag & ~O_SETFLAG) | (*((int *)data) & O_SETFLAG);
			}
			break;

    	case MEMPART_GET_INFO:
    	{
    		apmmgr_attr_t  *mp = (apmmgr_attr_t *)GET_PART_ATTR(ocb);
    		mempart_info_t *info_out = (mempart_info_t *)data;
			int  r = EOK;

			if (mp == NULL) {
				return EIO;
			} else if ((ocb->ioflag & _IO_FLAG_RD) == 0) {
				return EACCES;
			} else if (msg->o.nbytes < sizeof(*info_out)) {
				return EOVERFLOW;
			}

			if ((r = PART_ATTR_LOCK(mp)) != EOK) {
				return r;
			}

			memset(info_out, 0, sizeof(*info_out));

			/* can only be called on 'real' partitions */
			if (mp->type != part_type_MEMPART_REAL) {
				r = EBADF;
			} else if (MEMPART_GETINFO(mp->data.mpid, info_out) == NULL) {
				r = EIO;
			} else {
				info_out->id = mp->data.mpid;
				nbytes = sizeof(*info_out);
			}

			PART_ATTR_UNLOCK(mp);
			if (r != EOK) {
				return r;
			}
    		break;
    	}

		case MEMPART_CFG_CHG:
		{
			int  r = EOK;
			apmmgr_attr_t  *mp = (apmmgr_attr_t *)GET_PART_ATTR(ocb);
			mempart_cfgchg_t  *cfg = (mempart_cfgchg_t *)data;

			if (mp == NULL) {
				return EIO;
			} else if ((ocb->ioflag & _IO_FLAG_WR) == 0) {
				return EACCES;
			} else if (msg->i.nbytes < sizeof(*cfg)) {
				return EINVAL;
			}

			if ((r = PART_ATTR_LOCK(mp)) != EOK) {
				return r;
			}

			/* can only be called on 'real' partitions */
			if (mp->type != part_type_MEMPART_REAL) {
				r = EBADF;
			} else if (mp->data.mpid == part_id_t_INVALID) {
				r = EIO;
			/* caller did not provide info on what to change */
			} else if (cfg->valid == cfgchg_t_NONE) {
				r = EINVAL;
			}

			if (r != EOK)
			{
				PART_ATTR_UNLOCK(mp);
				return r;
			}

			/*
			 * external representation of 'memsize_t' can be larger than the
			 * internal representation of 'memsize_t' so ckeck for overflows
			 * on incoming size attributes
			*/
#ifdef _MEMSIZE_and_memsize_are_different_
			if (MEMSIZE_OFLOW(cfg->val.attr.size.min) ||
				MEMSIZE_OFLOW(cfg->val.attr.size.max))
			{
#ifndef NDEBUG
				if (ker_verbose) {
					kprintf("partition size attribute %d bit overflow\n",
							sizeof(_MEMSIZE_T_) * 8);
				}
#endif
				PART_ATTR_UNLOCK(mp);
				return EINVAL;
			}
#endif	/* _MEMSIZE_and_memsize_are_different_ */
			
			/*
			 * Unless we are changing everything, get the current configuration
			 * so that the changes can be merged
			*/
			if (!(cfg->valid == cfgchg_t_ALL))
			{
				mempart_t *mpart = MEMPART_ID_TO_T(mp->data.mpid);
				mempart_cfg_t *cur_cfg;

				if (mpart == NULL) {
					return EIO;
				}

				cur_cfg = &mpart->info.cur_cfg;

				/* if not changing the alloc policy, use the current */
				if (!(cfg->valid & cfgchg_t_ALLOC_POLICY)) {
					cfg->val.policy.alloc = cur_cfg->policy.alloc;
				}
				
				/* if not changing the terminal partition policy, use the current */
				if (!(cfg->valid & cfgchg_t_LOCK_POLICY)) {
					cfg->val.policy.config_lock = GET_MPART_POLICY_CFG_LOCK(cur_cfg);
				}
				
				/* if not changing the terminal partition policy, use the current */
				if (!(cfg->valid & cfgchg_t_TERMINAL_POLICY)) {
					cfg->val.policy.terminal = GET_MPART_POLICY_TERMINAL(cur_cfg);
				}

				/* if not changing the permanent partition policy, use the current */
				if (!(cfg->valid & cfgchg_t_PERMANENT_POLICY)) {
					cfg->val.policy.permanent = GET_MPART_POLICY_PERMANENT(cur_cfg);
				}
				
				/* if not changing the maximum size attribute, use the current */
				if (!(cfg->valid & cfgchg_t_ATTR_MAX)) {
					cfg->val.attr.size.max = cur_cfg->attr.size.max;
				}

				/* if not changing the minimum size attribute, use the current */
				if (!(cfg->valid & cfgchg_t_ATTR_MIN)) {
					cfg->val.attr.size.min = cur_cfg->attr.size.min;
				}
			}

			/* sanity check the parameters */
			if ((r = VALIDATE_MP_CFG_MODIFICATION(mp->data.mpid, &cfg->val)) != EOK)
			{
#ifndef NDEBUG
				if (ker_verbose > 1) {
					kprintf("Config Modification validation err %d\n", r);
				}
#endif	/* NDEBUG */
				PART_ATTR_UNLOCK(mp);
				return r;
			}
			/*
			 * one additional check that we have to do here (because the
			 * the validation routines don't know about children, only parents)
			 * is to prevent an allocation policy change from
			 * mempart_alloc_policy_t_HIERARCHICAL to something else if the
			 * partition already has children.
			*/
			if ((cfg->val.policy.alloc != mempart_alloc_policy_t_HIERARCHICAL) &&
				LIST_COUNT(mp->children) != 0)
			{
				PART_ATTR_UNLOCK(mp);
				return EINVAL;
			}

			/* should be good to change */
			if ((r = MEMPART_CHANGE(mp->data.mpid, &cfg->val, apmmgr_ocb->create_key)) != EOK)
			{
				PART_ATTR_UNLOCK(mp);
				return r;
			}
			
			PART_ATTR_UNLOCK(mp);
			break;
		}

    	case PART_GET_ID:
    	{
    		int r;
    		apmmgr_attr_t  *mp = (apmmgr_attr_t *)GET_PART_ATTR(ocb);

			if (mp == NULL) {
				return EIO;
			} else if ((ocb->ioflag & _IO_FLAG_RD) == 0) {
				return EACCES;
			} else if (msg->o.nbytes < sizeof(part_id_t)) {
				return EOVERFLOW;
			}

			if ((r = PART_ATTR_LOCK(mp)) != EOK) {
				return r;
			} else if ((mp->type != part_type_MEMPART_REAL) && (mp->type != part_type_MEMCLASS)) {
				PART_ATTR_UNLOCK(mp);
				return EINVAL;
			} else {
				*((part_id_t *)data) = mp->data.mpid;
				nbytes = sizeof(part_id_t);
				PART_ATTR_UNLOCK(mp);
				break;
			}
    	}
    	
#ifdef USE_PROC_OBJ_LISTS
    	case PART_GET_ASSOC_PLIST:
		{
			apmmgr_attr_t  *mp = (apmmgr_attr_t *)GET_PART_ATTR(ocb);
			mempart_t  *mpart;
    		part_plist_t *plist_out = (part_plist_t *)data;
			prp_node_t  *prp_list;
			unsigned int  num_pids = 0;
			unsigned int  num_entries;
			int  r = EOK;

			if (mp == NULL) {
				return EIO;
			} else if ((ocb->ioflag & _IO_FLAG_RD) == 0) {
				return EACCES;
			} else if (plist_out->num_entries < 0) {
				return EINVAL;
			} else if (msg->o.nbytes < PART_PLIST_T_SIZE(plist_out->num_entries)) {
				return EOVERFLOW;
			}

			if ((r = PART_ATTR_LOCK(mp)) != EOK) {
				return r;
			}

			/* can only be called on real partitions */
			if (mp->type != part_type_MEMPART_REAL)
			{
				PART_ATTR_UNLOCK(mp);
				return EBADF;
			}

			if ((mpart = MEMPART_ID_TO_T(mp->data.mpid)) == NULL) {
				return EIO;
			}
			prp_list = (prp_node_t *)LIST_FIRST(mpart->prp_list);
			num_pids = LIST_COUNT(mpart->prp_list);
			num_entries = min(num_pids, plist_out->num_entries);

			if (num_entries > 0)
			{
				unsigned i;
				for (i=0; i<num_entries; i++)
				{
					plist_out->pid[i] = prp_list->prp->pid;
					prp_list = (prp_node_t *)LIST_NEXT(prp_list);
				}
			}
			if (num_entries < num_pids) {
				plist_out->num_entries = -(num_pids - num_entries);
			} else {
				plist_out->num_entries = num_pids;
			}

			nbytes = PART_PLIST_T_SIZE(num_entries);
			PART_ATTR_UNLOCK(mp);
			break;
		}
#endif	/* USE_PROC_OBJ_LISTS */

		case MEMCLASS_GET_INFO:
		{
    		apmmgr_attr_t  *mp = (apmmgr_attr_t *)GET_PART_ATTR(ocb);
    		memclass_entry_t *memclass = NULL;
    		memclass_info_t *info_out = (memclass_info_t *)data;
			int  r = EOK;

			if (mp == NULL) {
				return EIO;
			} else if ((ocb->ioflag & _IO_FLAG_RD) == 0) {
				return EACCES;
			} else if (msg->o.nbytes < sizeof(*info_out)) {
				return EOVERFLOW;
			}

			if ((r = PART_ATTR_LOCK(mp)) != EOK) {
				return r;
			}

			/* can only be called on 'class' partitions */
			if (mp->type != part_type_MEMCLASS) {
				r = EBADF;
			} else if ((memclass = mp->data.mclass_entry) == NULL) {
				r = ENOENT;
			} else if ((memclass->data.allocator.size_info == NULL) ||
					 (memclass->data.allocator.size_info(&info_out->size, &memclass->data.info) == NULL)) {
				r = EIO;
			} else {		
				/* fill in remainder of the memclass_info_t */
				info_out->id = memclass->data.info.id;
				info_out->attr = memclass->data.info.attr;

				nbytes = sizeof(*info_out);
			}
			PART_ATTR_UNLOCK(mp);
			if (r != EOK) {
				return r;
			}
			break;
		}

		case EVENT_REG:
		{
			apmmgr_attr_t  *mp = (apmmgr_attr_t *)GET_PART_ATTR(ocb);
    		evtreg_t *er = (evtreg_t *)data;
			int  r = EOK;
			int  i;
			evtdest_t evt_dest;
			unsigned num_events = NUM_MEMCLASS_EVTTYPES;
			evtclass_t evtclass;
			evttype_t evttype_first;
			evttype_t evttype_last;
			int (*evtreg_func)(struct evt_info_s *evt, evtdest_t *evtdest, void *ocb);	

			CRASHCHECK(mp == NULL);
			CRASHCHECK(er == NULL);

			if ((ocb->ioflag & _IO_FLAG_RD) == 0) {
				return EACCES;
			} else if (msg->i.nbytes < EVTREG_T_SIZE(er->num_entries)) {
				return EOVERFLOW;
			}

			if ((r = PART_ATTR_LOCK(mp)) != EOK) {
				return r;
			}

			/* set up the registration data variables */
			if (mp->type == part_type_MEMPART_REAL)
			{
				num_events = NUM_MEMPART_EVTTYPES;
				evtclass = evtclass_t_MEMPART;
				evttype_first = mempart_evttype_t_first;
				evttype_last = mempart_evttype_t_last;
				evtreg_func = register_mpart_event;
			}
			else if (mp->type == part_type_MEMCLASS)
			{
				num_events = NUM_MEMCLASS_EVTTYPES;
				evtclass = evtclass_t_MEMCLASS;
				evttype_first = memclass_evttype_t_first;
				evttype_last = memclass_evttype_t_last;
				evtreg_func = register_mclass_event;
			}
			else
			{
				PART_ATTR_UNLOCK(mp);
				return EBADF;
			}

			evt_dest.pid = ctp->info.pid;
			evt_dest.rcvid = ctp->rcvid;
			evt_dest.in_progress = bool_t_FALSE;

#define GET_EVTTYPE_P(_ei_) \
		(((_ei_)->class == evtclass_t_MEMCLASS) ? &(_ei_)->info.mc.type : \
			((_ei_)->class == evtclass_t_MEMPART) ? &(_ei_)->info.mp.type : NULL)

			for (i=0; i<er->num_entries; i++)
			{
				int ret = EINVAL;
				evttype_t *evttype = GET_EVTTYPE_P(&er->evt[i]);

				/* currently event registration is an all or nothing proposition */
				if ((er->evt[i].class != evtclass) || (evttype == NULL) ||
					(*evttype < evttype_first) || (*evttype > evttype_last) ||
					((ret = evtreg_func(&er->evt[i], &evt_dest, ocb)) != EOK))
				{
					(void)unregister_events((apxmgr_attr_t *)mp, ocb, num_events);
					PART_ATTR_UNLOCK(mp);
					return ret;
				}
			}
			PART_ATTR_UNLOCK(mp);
			break;
		}

		case MEMPART_CNTR_RESET:
    	{
    		int r;
    		apmmgr_attr_t  *mp = (apmmgr_attr_t *)GET_PART_ATTR(ocb);
			mempart_t  *mpart;

			if ((ocb->ioflag & _IO_FLAG_WR) == 0) {
				return EACCES;
			}
			else if (msg->o.nbytes < sizeof(_MEMSIZE_T_)) {
				return EOVERFLOW;
			}
			else if ((r = PART_ATTR_LOCK(mp)) != EOK) {
				return r;
			}
			/* can only be called on 'real' partitions */
			else if (mp->type != part_type_MEMPART_REAL) {
				PART_ATTR_UNLOCK(mp);
				return EBADF;
			}
			else
			{
				mpart = MEMPART_ID_TO_T(mp->data.mpid);
				*((_MEMSIZE_T_ *)data) = mpart->info.hi_size;
				mpart->info.hi_size = 0;
				nbytes = sizeof(_MEMSIZE_T_);
			
				PART_ATTR_UNLOCK(mp);
			}
			break;
    	}

    	default:
        	return ENOSYS;
    }

    if(nbytes)
    {
        msg->o.ret_val = EOK;
        return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + nbytes);
    }
	return EOK;
}

__SRCVERSION("$IQ: apmmgr_devctl.c,v 1.23 $");

