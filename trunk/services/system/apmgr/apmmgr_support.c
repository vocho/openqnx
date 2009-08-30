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
 * apmmgr_support
 * 
 * Resource manager miscellaneous support functions
 * 
*/

#include "apmmgr.h"



/*******************************************************************************
 * mpath_find
 * 
 * search from <root> looking for <name> where root is in the memory partitioning
 * resource managers namespace (ie. below root_mpart). If found return a pointer
 * to the apmmgr_attr_t for <name>, otherwise return NULL.
 * If <parent> is non-NULL, return the parent apmmgr_attr_t
*/
apmmgr_attr_t *mpath_find(apmmgr_attr_t *root, char *path, apmmgr_attr_t **parent)
{
	return (apmmgr_attr_t *)path_find(apmgr_type_MEM, (apxmgr_attr_t *)root, path, (apxmgr_attr_t **)parent);
}

/*******************************************************************************
 * mpmgr_getattr
 * 
 * Return a pointer to the 'apmmgr_attr_t' associated with <path>.
 * This function hooks the generic apmgr code to the apmmgr code.
*/
apmmgr_attr_t *mpmgr_getattr(char *path)
{
	if (*path == '/') ++path;
	return mpath_find((apmmgr_attr_t *)LIST_FIRST(root_mpart->children), path, NULL);
}

/*******************************************************************************
 * find_pid_in_mpart_hierarchy
 * 
 * Determine whether process <pid> is associated with any partition in the
 * partition hierarchy starting at <mp>.
 *
 * If so, a pointer to the 'apmmgr_attr_t' that the process is associated
 * with will be returned, otherwise NULL will be returned.
 * If an 'apmmgr_attr_t' structure is returned, it is guaranteed to be a
 * child, grandchild, great-grandchild, etc of <mp>. 
*/
apmmgr_attr_t *find_pid_in_mpart_hierarchy(pid_t pid, apmmgr_attr_t *mp)
{
	PROCESS *prp;
	mempart_t *mpart;
	part_id_t mpid;
	apmmgr_attr_t *mp_to_find;

	if (((prp = proc_lookup_pid(pid)) == NULL) ||
		((mpid = mempart_getid(prp, mempart_get_classid(mp->data.mpid))) == part_id_t_INVALID)) {
		return NULL;
	}

	mpart = MEMPART_ID_TO_T(mpid);
	CRASHCHECK(mpart == NULL);

	mp_to_find = (apmmgr_attr_t *)mpart->rmgr_attr_p;
	CRASHCHECK(mp_to_find == NULL);

	if (mp == mp_to_find) {
		return mp;
	} else {
		while ((mp_to_find = mp_to_find->parent) != NULL) {
			if (mp_to_find == mp) {
				return mp;
			}
		}
		return NULL;
	}
}


/*******************************************************************************
 * register_mpart_event
 * 
 * Register memory partition event <evt> to the memory partition identified by
 * <ocb> and <rcvid>.
 * 
 * If successful, a 'mempart_evt_t' structure will be allocated, initialized
 * and added to the queue of events for the event type.
 * 
 * Returns: EOK on success otherwise an errno
 * 
 * 'memsize_t' overflow check
 * external representation of 'memsize_t' can be larger than the internal
 * representation of 'memsize_t' so ckeck for overflows on incoming size
 * attributes
*/
int register_mpart_event(struct evt_info_s *evt, evtdest_t *evtdest, void *ocb)
{
	apmmgr_attr_t  *mp = (apmmgr_attr_t *)GET_PART_ATTR(ocb);
	mempart_evt_t  *event = calloc(1, sizeof(*event));
	int i, r;

	CRASHCHECK(mp->type != part_type_MEMPART_REAL);
	CRASHCHECK(evt->class != evtclass_t_MEMPART);
	CRASHCHECK(evt->info.mp.type < mempart_evttype_t_first);
	CRASHCHECK(evt->info.mp.type > mempart_evttype_t_last);

	if (event == NULL) return ENOMEM;

	if ((evt->info.mp.type == mempart_evttype_t_THRESHOLD_CROSS_OVER) ||
		(evt->info.mp.type == mempart_evttype_t_THRESHOLD_CROSS_UNDER))
	{
#ifdef _MEMSIZE_and_memsize_are_different_
		if (MEMSIZE_OFLOW(evt->info.mp.val.threshold_cross))
		{
#ifndef NDEBUG
			kprintf("event threshold size %d bit overflow\n", sizeof(_MEMSIZE_T_) * 8);
#endif
			free(event);
			return EOVERFLOW;
		}
#endif	/*  _MEMSIZE_and_memsize_are_different_ */
	}
	else if ((evt->info.mp.type == mempart_evttype_t_DELTA_INCR) ||
		(evt->info.mp.type == mempart_evttype_t_DELTA_DECR))
	{
#ifdef _MEMSIZE_and_memsize_are_different_
		if (MEMSIZE_OFLOW(evt->info.mp.val.delta))
		{
#ifndef NDEBUG
			kprintf("event delta size %d bit overflow\n", sizeof(_MEMSIZE_T_) * 8);
#endif
			free(event);
			return EOVERFLOW;
		}
#endif	/*  _MEMSIZE_and_memsize_are_different_ */

		/* cause the proper value to be initialized once the event is registered */
		event->evt_data.size = (memsize_t)memsize_t_INFINITY;
	}
	else if ((evt->info.mp.type == mempart_evttype_t_CONFIG_CHG_ATTR_MIN) ||
		(evt->info.mp.type == mempart_evttype_t_CONFIG_CHG_ATTR_MAX))
	{
#ifdef _MEMSIZE_and_memsize_are_different_
		if (MEMSIZE_OFLOW(evt->info.mp.val.cfg_chg.attr.min))
		{
#ifndef NDEBUG
			kprintf("event attribute size %d bit overflow\n", sizeof(_MEMSIZE_T_) * 8);
#endif
			free(event);
			return EOVERFLOW;
		}
#endif	/*  _MEMSIZE_and_memsize_are_different_ */
	}

	event->evt_reg = *evt;
	event->evt_dest = *evtdest;
	event->ocb = ocb;
	event->evt_reg.flags &= ~evtflags_RESERVED;

	if ((r = PART_ATTR_LOCK(mp)) != EOK) {
		free(event);
		return r;
	}
	INTR_LOCK(&mp->event_list[MEMPART_EVTYPE_IDX(evt->info.mp.type)].active.lock);
	LIST_ADD(mp->event_list[MEMPART_EVTYPE_IDX(evt->info.mp.type)].active.list, event);
	event->onlist = &mp->event_list[MEMPART_EVTYPE_IDX(evt->info.mp.type)].active;
	INTR_UNLOCK(&mp->event_list[MEMPART_EVTYPE_IDX(evt->info.mp.type)].active.lock);

	/* clean up any inactive events */
	for (i=0; i<NUM_ELTS(mp->event_list); i++)
	{
		/* clean up the inactive events list */
		clean_inactive_list(&mp->event_list[i].inactive);
	}

	PART_ATTR_UNLOCK(mp);
	return EOK;
}

/*******************************************************************************
 * register_mclass_event
 * 
 * Register memory class event <evt> to the memory class identified by <ocb>
 * and <rcvid>.
 * 
 * If successful, a 'mempart_evt_t' structure will be allocated, initialized
 * and added to the queue of events for the event type.
 * 
 * Returns: EOK on success otherwise an errno
 * 
 * 'memsize_t' overflow check
 * external representation of 'memsize_t' can be larger than the internal
 * representation of 'memsize_t' so ckeck for overflows on incoming size
 * attributes
*/
int register_mclass_event(struct evt_info_s *evt, evtdest_t *evtdest, void *ocb)
{
	apmmgr_attr_t  *mp = (apmmgr_attr_t *)GET_PART_ATTR(ocb);
	mempart_evt_t  *event = calloc(1, sizeof(*event));
	int i, r;

	CRASHCHECK(mp->type != part_type_MEMCLASS);
	CRASHCHECK(evt->class != evtclass_t_MEMCLASS);
	CRASHCHECK(evt->info.mc.type < memclass_evttype_t_first);
	CRASHCHECK(evt->info.mc.type > memclass_evttype_t_last);

	if (event == NULL) return ENOMEM;

	switch (evt->info.mc.type)
	{
		case memclass_evttype_t_THRESHOLD_CROSS_TF_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_TF_UNDER:
		case memclass_evttype_t_THRESHOLD_CROSS_TU_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_TU_UNDER:
		case memclass_evttype_t_THRESHOLD_CROSS_RF_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_RF_UNDER:
		case memclass_evttype_t_THRESHOLD_CROSS_UF_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_UF_UNDER:
		case memclass_evttype_t_THRESHOLD_CROSS_RU_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_RU_UNDER:
		case memclass_evttype_t_THRESHOLD_CROSS_UU_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_UU_UNDER:
		{
#ifdef _MEMSIZE_and_memsize_are_different_
			if (MEMSIZE_OFLOW(evt->info.mc.val.threshold_cross))
			{
#ifndef NDEBUG
				kprintf("event threshold size %d bit overflow\n", sizeof(_MEMSIZE_T_) * 8);
#endif
				free(event);
				return EOVERFLOW;
			}
#endif	/* _MEMSIZE_and_memsize_are_different_ */
			break;
		}
		
		case memclass_evttype_t_DELTA_TF_INCR:
		case memclass_evttype_t_DELTA_TF_DECR:
		case memclass_evttype_t_DELTA_TU_INCR:
		case memclass_evttype_t_DELTA_TU_DECR:
		case memclass_evttype_t_DELTA_RF_INCR:
		case memclass_evttype_t_DELTA_RF_DECR:
		case memclass_evttype_t_DELTA_UF_INCR:
		case memclass_evttype_t_DELTA_UF_DECR:
		case memclass_evttype_t_DELTA_RU_INCR:
		case memclass_evttype_t_DELTA_RU_DECR:
		case memclass_evttype_t_DELTA_UU_INCR:
		case memclass_evttype_t_DELTA_UU_DECR:
		{
#ifdef _MEMSIZE_and_memsize_are_different_
			if (MEMSIZE_OFLOW(evt->info.mc.val.delta))
			{
#ifndef NDEBUG
				kprintf("event delta size %d bit overflow\n", sizeof(_MEMSIZE_T_) * 8);
#endif
				free(event);
				return EOVERFLOW;
			}
#endif	/* _MEMSIZE_and_memsize_are_different_ */
			/* cause the proper value to be initialized once the event is registered */
			event->evt_data.size = (memsize_t)memsize_t_INFINITY;
			break;
		}
		
		default:	/* other type of event */
			break;
	}

#ifndef NDEBUG
	/* FIX ME - remove */
	if (!(evt->flags & evtflags_SIGEV_FLAG_SIGINFO)) {
		kprintf("Forcing evtflags_SIGEV_FLAG_SIGINFO flag\n");
		evt->flags |= evtflags_SIGEV_FLAG_SIGINFO;
	}
#endif	/* NDEBUG */

	event->evt_reg = *evt;
	event->evt_dest = *evtdest;
	event->ocb = ocb;
	event->evt_reg.flags &= ~evtflags_RESERVED;

	if ((r = PART_ATTR_LOCK(mp)) != EOK) {
		free(event);
		return r;
	}
	INTR_LOCK(&mp->event_list[MEMCLASS_EVTYPE_IDX(evt->info.mc.type)].active.lock);
	LIST_ADD(mp->event_list[MEMCLASS_EVTYPE_IDX(evt->info.mc.type)].active.list, event);
	event->onlist = &mp->event_list[MEMCLASS_EVTYPE_IDX(evt->info.mc.type)].active;
	INTR_UNLOCK(&mp->event_list[MEMCLASS_EVTYPE_IDX(evt->info.mc.type)].active.lock);

	/* clean up any inactive events */
	for (i=0; i<NUM_ELTS(mp->event_list); i++)
	{
		/* clean up the inactive events list */
		clean_inactive_list(&mp->event_list[i].inactive);
	}

	PART_ATTR_UNLOCK(mp);
	return EOK;
}


/*******************************************************************************
 * validate_mp_association
 * 
 * This function determines whether or not the partition identified by <mpart>
 * can be associated with based on the 'struct _cred_info' <cred>.
 *
 * if S_IXOTH is set in the attributes for <mpart> then EOK is returned otherwise
 * an appropriate uid/gid match is attempted.
 *  
 * Returns: EOK if association is permitted, otherwise an errno.
 * 
*/
int validate_mp_association(apmmgr_attr_t *attr, struct _cred_info *cred)
{
	if (attr->attr.mode & S_IXOTH) return EOK;
	if ((attr->attr.gid == cred->rgid) && (attr->attr.mode & S_IXGRP)) return EOK;
	if ((attr->attr.uid == cred->ruid) && (attr->attr.mode & S_IXUSR)) return EOK;
	
	return ((attr->attr.gid == cred->rgid) || (attr->attr.uid == cred->ruid)) ? EACCES : EPERM;
}


/*
 * ===========================================================================
 * 
 * 							Internal support routines
 * 
 * ===========================================================================
*/



__SRCVERSION("$IQ: apmmgr_support.c,v 1.23 $");

