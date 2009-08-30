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
 * apsmgr_support
 * 
 * Resource manager miscellaneous support functions
 * 
*/

#include "apsmgr.h"



/*******************************************************************************
 * spath_find
 * 
 * search from <root> looking for <name> where root is in the scheduler partitioning
 * resource managers namespace (ie. below root_spart). If found return a pointer
 * to the apsmgr_attr_t for <name>, otherwise return NULL.
 * If <parent> is non-NULL, return the parent apsmgr_attr_t
*/
apsmgr_attr_t *spath_find(apsmgr_attr_t *root, char *path, apsmgr_attr_t **parent)
{
	return (apsmgr_attr_t *)path_find(apmgr_type_SCHED, (apxmgr_attr_t *)root, path, (apxmgr_attr_t **)parent);
}

/*******************************************************************************
 * spmgr_getattr
 * 
 * Return a pointer to the 'apsmgr_attr_t' associated with <path>.
 * This function hooks the generic apsgr code to the apsmgr code.
*/
apsmgr_attr_t *spmgr_getattr(char *path)
{
	if (*path == '/') ++path;
	return spath_find((apsmgr_attr_t *)LIST_FIRST(root_spart->children), path, NULL);
}

/*******************************************************************************
 * find_pid_in_spart_hierarchy
 * 
 * Determine whether process <pid> is associated with any partition in the
 * partition hierarchy starting at <sp>.
 *
 * If so, a pointer to the 'apsmgr_attr_t' that the process is associated
 * with will be returned, otherwise NULL will be returned.
 * If an 'apsmgr_attr_t' structure is returned, it is guaranteed to be a
 * child, grandchild, great-grandchild, etc of <sp>. 
*/
apsmgr_attr_t *find_pid_in_spart_hierarchy(pid_t pid, apsmgr_attr_t *sp)
{
	PROCESS *prp;
	schedpart_t *spart;
	part_id_t spid;
	apsmgr_attr_t *sp_to_find;

	if (((prp = proc_lookup_pid(pid)) == NULL) ||
		((spid = schedpart_getid(prp)) == part_id_t_INVALID))
		return NULL;

	spart = SCHEDPART_ID_TO_T(spid);
	CRASHCHECK(spart == NULL);

	sp_to_find = (apsmgr_attr_t *)spart->rmgr_attr_p;
	CRASHCHECK(sp_to_find == NULL);

	if (sp == sp_to_find)
		return sp;
	else
	{
		while ((sp_to_find = sp_to_find->parent) != NULL) {
			if (sp_to_find == sp) {
				return sp;
			}
		}
		return NULL;
	}
}


/*******************************************************************************
 * register_spart_event
 * 
 * Register scheduler partition event <evt> to the scheduler partition identified by
 * <ocb> and <rcvid>.
 * 
 * If successful, a 'schedpart_evt_t' structure will be allocated, initialized
 * and added to the queue of events for the event type.
 * 
 * Returns: EOK on success otherwise an errno
 * 
 * 'memsize_t' overflow check
 * external representation of 'memsize_t' can be larger than the internal
 * representation of 'memsize_t' so ckeck for overflows on incoming size
 * attributes
*/
int register_spart_event(struct evt_info_s *evt, evtdest_t *evtdest, void *ocb)
{
	apsmgr_attr_t  *sp = (apsmgr_attr_t *)GET_PART_ATTR(ocb);
	schedpart_evt_t  *event = calloc(1, sizeof(*event));
	int r;
	unsigned i;

	CRASHCHECK(sp == NULL);
	CRASHCHECK(sp->type != part_type_SCHEDPART_REAL);
	CRASHCHECK(evt->class != evtclass_t_SCHEDPART);
	CRASHCHECK(evt->info.sp.type < schedpart_evttype_t_first);
	CRASHCHECK(evt->info.sp.type > schedpart_evttype_t_last);

	if (event == NULL) return ENOMEM;

	event->evt_reg = *evt;
	event->evt_dest = *evtdest;
	event->ocb = ocb;
	event->evt_reg.flags &= ~evtflags_RESERVED;

	if ((r = PART_ATTR_LOCK(sp)) != EOK) {
		free(event);
		return r;
	}
	INTR_LOCK(&sp->event_list[SCHEDPART_EVTYPE_IDX(evt->info.sp.type)].active.lock);
	LIST_ADD(sp->event_list[SCHEDPART_EVTYPE_IDX(evt->info.sp.type)].active.list, event);
	event->onlist = &sp->event_list[SCHEDPART_EVTYPE_IDX(evt->info.sp.type)].active;
	INTR_UNLOCK(&sp->event_list[SCHEDPART_EVTYPE_IDX(evt->info.sp.type)].active.lock);

	/* clean up any inactive events */
	for (i=0; i<NUM_ELTS(sp->event_list); i++)
	{
		/* clean up the inactive events list */
		clean_inactive_list(&sp->event_list[i].inactive);
	}

	PART_ATTR_UNLOCK(sp);
	return EOK;
}


/*******************************************************************************
 * validate_sp_association
 * 
 * This function determines whether or not the partition identified by <spart>
 * can be associated with based on the 'struct _cred_info' <cred>.
 *
 * if S_IXOTH is set in the attributes for <spart> then EOK is returned otherwise
 * an appropriate uid/gid match is attempted.
 *  
 * Returns: EOK if association is permitted, otherwise an errno.
 * 
*/
int validate_sp_association(apsmgr_attr_t *attr, struct _cred_info *cred)
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




__SRCVERSION("$IQ: apsmgr_support.c,v 1.23 $");

