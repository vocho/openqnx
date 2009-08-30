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
 * apsmgr_devctl
 * 
 * Provide resource manager devctl() processing for the scheduler partitioning
 * module
 * 
*/

#include "apsmgr.h"
#include "aps.h"

typedef char	schedpart_path_t[PATH_MAX+1];

int apsmgr_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *_ocb)
{
	apsmgr_ocb_t *apsmgr_ocb = (apsmgr_ocb_t *)_ocb;
	iofunc_ocb_t *ocb = &apsmgr_ocb->ocb;
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

    	case SCHEDPART_GET_INFO:
    	{
    		apsmgr_attr_t  *mp = (apsmgr_attr_t *)GET_PART_ATTR(ocb);
    		schedpart_info_t *info_out = (schedpart_info_t *)data;
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
			if (mp->type != part_type_SCHEDPART_REAL) {
				r = EBADF;
			} else if (SCHEDPART_GETINFO(mp->data.spid, info_out) == NULL) {
				r = EIO;
			} else {
				info_out->id = mp->data.spid;
				nbytes = sizeof(*info_out);
			}

			PART_ATTR_UNLOCK(mp);
			if (r != EOK) {
				return r;
			}
    		break;
    	}

		case SCHEDPART_CFG_CHG:
		{
			int  r = EOK;
			apsmgr_attr_t  *mp = (apsmgr_attr_t *)GET_PART_ATTR(ocb);
			schedpart_cfgchg_t  *cfg = (schedpart_cfgchg_t *)data;

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
			if (mp->type != part_type_SCHEDPART_REAL) {
				r = EBADF;
			} else if (mp->data.spid == part_id_t_INVALID) {
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
			 * Unless we are changing everything, get the current configuration
			 * so that the changes can be merged
			*/
			if (!(cfg->valid == cfgchg_t_ALL))
			{
				schedpart_t *spart = SCHEDPART_ID_TO_T(mp->data.spid);
				schedpart_cfg_t *cur_cfg;

				if (spart == NULL) {
					return EIO;
				}

				cur_cfg = &spart->info.cur_cfg;

				/* if not changing the terminal partition policy, use the current */
				if (!(cfg->valid & cfgchg_t_LOCK_POLICY)) {
					cfg->val.policy.config_lock = GET_SPART_POLICY_CFG_LOCK(cur_cfg);
				}
				
				/* if not changing the terminal partition policy, use the current */
				if (!(cfg->valid & cfgchg_t_TERMINAL_POLICY)) {
					cfg->val.policy.terminal = GET_SPART_POLICY_TERMINAL(cur_cfg);
				}

				/* if not changing the permanent partition policy, use the current */
				if (!(cfg->valid & cfgchg_t_PERMANENT_POLICY)) {
					cfg->val.policy.permanent = GET_SPART_POLICY_PERMANENT(cur_cfg);
				}

				/* if not changing the budget attribute, use the current */
				if (!(cfg->valid & cfgchg_t_ATTR_BUDGET)) {
					cfg->val.attr.budget_percent = cur_cfg->attr.budget_percent;
				}

				/* if not changing the critical budget attribute, use the current */
				if (!(cfg->valid & cfgchg_t_ATTR_CRIT_BUDGET)) {
					cfg->val.attr.critical_budget_ms = cur_cfg->attr.critical_budget_ms;
				}
			}

			/* sanity check the parameters */
			if ((r = VALIDATE_SP_CFG_MODIFICATION(mp->data.spid, &cfg->val)) != EOK)
			{
#ifndef NDEBUG
				if (ker_verbose > 1) {
					kprintf("Config Modification validation err %d\n", r);
				}
#endif	/* NDEBUG */
				PART_ATTR_UNLOCK(mp);
				return r;
			}

			/* should be good to change */
			if ((r = SCHEDPART_CHANGE(mp->data.spid, &cfg->val, apsmgr_ocb->create_key)) != EOK)
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
    		apsmgr_attr_t  *mp = (apsmgr_attr_t *)GET_PART_ATTR(ocb);

			if (mp == NULL) {
				return EIO;
			} else if ((ocb->ioflag & _IO_FLAG_RD) == 0) {
				return EACCES;
			} else if (msg->o.nbytes < sizeof(part_id_t)) {
				return EOVERFLOW;
			}

			if ((r = PART_ATTR_LOCK(mp)) != EOK) {
				return r;
			} else if (mp->type != part_type_SCHEDPART_REAL) {
				PART_ATTR_UNLOCK(mp);
				return EINVAL;
			} else {
				*((part_id_t *)data) = mp->data.spid;
				nbytes = sizeof(part_id_t);
				PART_ATTR_UNLOCK(mp);
				break;
			}
    	}
    	
#ifdef USE_PROC_OBJ_LISTS
    	case PART_GET_ASSOC_PLIST:
		{
			apsmgr_attr_t  *mp = (apsmgr_attr_t *)GET_PART_ATTR(ocb);
			schedpart_t  *spart;
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
			if (mp->type != part_type_SCHEDPART_REAL)
			{
				PART_ATTR_UNLOCK(mp);
				return EBADF;
			}

			if ((spart = SCHEDPART_ID_TO_T(mp->data.spid)) == NULL) {
				return EIO;
			}
			prp_list = (prp_node_t *)LIST_FIRST(spart->prp_list);
			num_pids = LIST_COUNT(spart->prp_list);
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

		case EVENT_REG:
		{
			apsmgr_attr_t  *sp = (apsmgr_attr_t *)GET_PART_ATTR(ocb);
    		evtreg_t *er = (evtreg_t *)data;
			int  r = EOK;
			int  i;

			CRASHCHECK(sp == NULL);
			CRASHCHECK(er == NULL);

			if (sp->type != part_type_SCHEDPART_REAL) {
				return EBADF;
			} else if ((ocb->ioflag & _IO_FLAG_RD) == 0) {
				return EACCES;
			} else if (msg->i.nbytes < EVTREG_T_SIZE(er->num_entries)) {
				return EOVERFLOW;
			}

			if ((r = PART_ATTR_LOCK(sp)) != EOK) {
				return r;
			}
			
			for (i=0; i<er->num_entries; i++)
			{
				int ret = EINVAL;
				evtdest_t evt_dest;
				
				evt_dest.pid = ctp->info.pid;
				evt_dest.rcvid = ctp->rcvid;
				evt_dest.in_progress = bool_t_FALSE;

				switch (er->evt[i].class)
				{
					case evtclass_t_SCHEDPART:
					{
						if ((er->evt[i].info.sp.type >= schedpart_evttype_t_first) &&
							(er->evt[i].info.sp.type <= schedpart_evttype_t_last)) {
							ret = register_spart_event(&er->evt[i], &evt_dest, ocb);
						}

						/* currently event registration is an all or nothing proposition */
						if (ret != EOK)
						{
							(void)unregister_events((apxmgr_attr_t *)sp, ocb, NUM_SCHEDPART_EVTTYPES);
							PART_ATTR_UNLOCK(sp);
							return ret;
						}
						break;
					}
					default:
						PART_ATTR_UNLOCK(sp);
						return EINVAL;
				}
			}
			PART_ATTR_UNLOCK(sp);
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

__SRCVERSION("$IQ: apsmgr_devctl.c,v 1.23 $");

