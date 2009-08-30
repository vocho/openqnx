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
 * apmgr_support
 * 
 * Resource manager miscellaneous support functions
 * 
*/

#include "apmgr.h"
#include "apmmgr.h"
#include "apsmgr.h"

static apxmgr_attr_t *_path_find(apxmgr_attr_t *root, char *path, apxmgr_attr_t **parent);
static apxmgr_attr_t *find_sibling(apxmgr_attr_t *root, const char *name);
static int _check_access_perms(resmgr_context_t *ctp, iofunc_attr_t *attr, mode_t check, struct _client_info *info);
static void unregister_event(part_qnodehdr2_t *evlist, part_evt_t *reg_event);


/*******************************************************************************
 * path_find
 * 
 * search from <root> looking for <name>. If found return a pointer to the
 * apmmgr_attr_t for <name>, otherwise return NULL.
 * If <parent> is non-NULL, return the parent apmmgr_attr_t
*/
apxmgr_attr_t *path_find(apmgr_type_t type, apxmgr_attr_t *root, char *path, apxmgr_attr_t **parent)
{
	if (*path == '\0')
	{
		apxmgr_attr_t *ret_root;
		if (type == apmgr_type_MEM) {
			ret_root = (apxmgr_attr_t *)root_mpart;
		} else if (type == apmgr_type_SCHED) {
			ret_root = (apxmgr_attr_t *)root_spart;
		} else if (type == apmgr_type_NAME) {
			ret_root = (apxmgr_attr_t *)root_npart;
		} else {
			return NULL;
		}
		if (parent != NULL) *parent = ret_root->parent;
		return ret_root;
	}
	else
	{
		if (parent != NULL) *parent = (root != NULL) ? root->parent : NULL;
		return _path_find(root, path, parent);
	}
}


/*******************************************************************************
 * isStringOfDigits
 * 
 * Return True or False based on whether or not <part_name> is a string of
 * digits between (0-9)
*/
bool isStringOfDigits(char *part_name)
{
	char  c;
	while((c = *part_name++) != '\0')
		if ((c < 0x30) || (c > 0x39))
			return 0;	// FALSE
	
	return 1;	// TRUE;
}


/*******************************************************************************
 * check_access_perms
 * 
 * Recursively check the hierarchy starting at <mp> for accessibility
 * 
 * Returns: EOK on success, otherwise and errno
*/
int check_access_perms(resmgr_context_t *ctp, apxmgr_attr_t *mp, mode_t check,
						struct _client_info *info, bool recurse)
{
	int  r;

	if ((r = _check_access_perms(ctp, &mp->attr, check, info)) == EOK)
		if ((mp->parent != NULL) && (mp->parent->type != part_type_ROOT) && recurse)
			r = check_access_perms(ctp, mp->parent, check, info, recurse);
	return r;
}


/*******************************************************************************
 * npath_find
 * 
 * search from <root> looking for <name> where root is in the common partitioning
 * resource managers namespace (ie. below root_npart). If found return a pointer
 * to the apmgr_attr_t for <name>, otherwise return NULL.
 * If <parent> is non-NULL, return the parent apmgr_attr_t
*/
apmgr_attr_t *npath_find(apmgr_attr_t *root, char *path, apmgr_attr_t **parent)
{
	return (apmgr_attr_t *)path_find(apmgr_type_NAME, (apxmgr_attr_t *)root, path, (apxmgr_attr_t **)parent);
}




/*******************************************************************************
 * deactivate_event
 * 
 * This function will move <event> from its active list to its inactive list.
 * This happens during event dregistration and or when an event has been deemed
 * undeliverable a specified number of times. Inactive lists are cleaned up
 * whenever a registration or deregistration takes place. 
*/
void deactivate_event(part_evtlist_t *evlist, part_evt_t  *event)
{
	/* remove the event from the active list and put it on the inactive list */
	if (event->onlist == &evlist->active)
	{
		INTR_LOCK(&evlist->active.lock);
//kprintf("deactivate: event %p off of active %p\n", event, &evlist->active);
		if (event->onlist == &evlist->active)
		{
			LIST_DEL(evlist->active.list, event);
			event->onlist = NULL;
		}
		INTR_UNLOCK(&evlist->active.lock);
	}
	if (event->onlist == NULL)
	{
		INTR_LOCK(&evlist->inactive.lock);
//kprintf("deactivate: event %p onto inactive %p\n", event, &evlist->inactive);
		if (event->onlist == NULL)
		{
			LIST_ADD(evlist->inactive.list, event);
			event->onlist = &evlist->inactive;
		}
		INTR_UNLOCK(&evlist->inactive.lock);

#ifndef NDEBUG
		if ((event->undeliverable_count != 0) ||
			(event->evt_reg.flags != evtflags_SIGEV_FLAG_SIGINFO))
		{
/*
			kprintf("event %d to pid %d deactivated (cnt %u, fl = 0x%x)\n",
					event->evt_reg.mp.info.type,
					event->evt_dest.pid,
					event->undeliverable_count,
					event->evt_reg.mp.flags);
*/
		}
#endif	/* NDEBUG */
	}
}


/*******************************************************************************
 * unregister_events
 * 
 * Remove any events for <mp> that were attached through <ocb>.
 * If <ocb> is NULL, all events are removed for <mp> by explicitly deactivating
 * them. This mode is typeically only used during partition removal
 * 
 * FIX ME - currently I run all the event lists for the mempart referenced by
 * <ocb>. This is very inefficient. What I should do is also link all events
 * registered through this ocb to the ocb as well so that cleanup will be a
 * breeze. This requires a private ocb type which I currently don't do. 
*/
int unregister_events(apxmgr_attr_t *p, iofunc_ocb_t *ocb, int num_evts)
{
	unsigned  i;
	int  r;
	part_evt_t *event;

	CRASHCHECK(p == NULL);
	CRASHCHECK((p->type != part_type_MEMPART_REAL) && 
			   (p->type != part_type_SCHEDPART_REAL) &&
			   (p->type != part_type_MEMCLASS)); 

	if ((r = PART_ATTR_LOCK(p)) != EOK) return r;
	
	for (i=0; i<num_evts; i++)
	{
		part_evtlist_t *evlist;
		if ((p->type == part_type_MEMPART_REAL) ||  (p->type == part_type_MEMCLASS)) {
			apmmgr_attr_t *mp = (apmmgr_attr_t *)p;
			evlist = &mp->event_list[i];
		} else if (p->type == part_type_SCHEDPART_REAL) {
			apsmgr_attr_t *sp = (apsmgr_attr_t *)p;
			evlist = &sp->event_list[i];
		}
		else {
			continue;
		}
restart:
		INTR_LOCK(&evlist->active.lock);
		event = (part_evt_t *)LIST_FIRST(evlist->active.list);
		INTR_UNLOCK(&evlist->active.lock);

		while (event != NULL)
		{
			/*
			 * when scanning through the active list, it is possible for an
			 * event to transition onto the inactive list such that event->next
			 * gives us the next event on the inactive, not the active list. In
			 * this case, restart the scan from the head of the active list. It
			 * does not matter if we reprocess events, as long as we get through
			 * the entire active list. Note that the reimplementation note above
			 * (FIX ME) will allow me unhook all of the events registered by 'ocb'
			 * at once. Once that is done, they won;t be sent and we can safely
			 * delete them
			*/
			if (event->onlist != &evlist->active) {
/*
				kprintf("restart unregister of evlist %p (%u)\n",
						&evlist->active.list, LIST_COUNT(evlist->active.list));
*/
				goto restart;
			}

			if ((ocb == NULL) || (event->ocb == ocb))
			{
				/*
				 * set the DEACTIVATE flag and clear the REARM flag. This will
				 * cause the event to be (eventually if not immediately) moved
				 * to the inactive list where it will be cleaned up
				*/
				event->evt_reg.flags = evtflags_DEACTIVATE;
				/* force deactivation by moving event to teh inactive list */
				if (ocb == NULL) {
					deactivate_event(evlist, event);
					goto restart;
				}
			}
			INTR_LOCK(&evlist->active.lock);
			event = (part_evt_t *)LIST_NEXT(event);
			INTR_UNLOCK(&evlist->active.lock);
		}
		/* clean up the inactive events list */
		clean_inactive_list(&evlist->inactive);
	}
	PART_ATTR_UNLOCK(p);
	return EOK;
}


/*******************************************************************************
 * clean_inactive_list
 * 
 * clean the inactive events list.
 * We accomplish the necessary synchonization for this by moving all of the
 * inactive events to a temporary list so that the primary list for the event
 * can remain in use by the event delivery code. While the temp list is being
 * processed it is possible that more inactive events can be assed to to real
 * inactive list but that's ok, they'll get cleaned up next time
*/
void clean_inactive_list(part_qnodehdr2_t *evlist)
{
	part_qnodehdr2_t  tmp_evlist;
	part_evt_t  *event;

	if (LIST_COUNT(evlist->list) > 0)
	{
		/* move all inactive nodes to a temporary list */
		INTR_LOCK(&evlist->lock);
		tmp_evlist.list = evlist->list;
		/* patch up the first node) */
		if (evlist->list.head != NULL) evlist->list.head->prev = &tmp_evlist.list.head;
		LIST_INIT(evlist->list);
		INTR_UNLOCK(&evlist->lock);

//	kprintf("Delete %d nodes from evlist %p\n", LIST_COUNT(tmp_evlist.list), evlist);
		event = (part_evt_t *)LIST_FIRST(tmp_evlist.list);
		while (event != NULL)
		{
			part_evt_t *next_event = (part_evt_t *)LIST_NEXT(event);
			CRASHCHECK(event->onlist != evlist);
			if (event->inuse == 0) {
				unregister_event(&tmp_evlist, event);
			}
			event = next_event;
		}
		if (LIST_COUNT(tmp_evlist.list) > 0)
		{
//	kprintf("Need to return %d nodes to evlist %p with %d nodes\n", LIST_COUNT(tmp_evlist.list), evlist, LIST_COUNT(evlist->list));
			/* return any nodes that could not be released back to the inactive list */
			while ((event = (part_evt_t *)LIST_FIRST(tmp_evlist.list)) != NULL)
			{
				LIST_DEL(tmp_evlist.list, event);
				INTR_LOCK(&evlist->lock);
				LIST_ADD(evlist->list, event);
				INTR_UNLOCK(&evlist->lock);
			}
		}
	}
}


/*
 * ===========================================================================
 * 
 * 							Internal support routines
 * 
 * ===========================================================================
*/

/*******************************************************************************
 * find_sibling
 * 
 * Find <name> in <root> or its siblings.
 * 
 * Returns: a apmmgr_attr_t on success, otherwise NULL
*/
static apxmgr_attr_t *find_sibling(apxmgr_attr_t *root, const char *name)
{
	while(root != NULL)
	{
		if (strcmp(name, root->name) == 0)
		{
//			if (parent) *parent = root->parent;
			return(root);
		}
		root = (apxmgr_attr_t *)LIST_NEXT(root);
	}
	return NULL;
}

/*******************************************************************************
 * _path_find
 * 
 * Recurse down through a hierarchy starting at <root> looking for <path>
 * 
 * Returns: a apmmgr_attr_t on success, otherwise NULL
*/
static apxmgr_attr_t *_path_find(apxmgr_attr_t *root, char *path, apxmgr_attr_t **parent)
{
	if (root != NULL)
	{
		char *p;

		if (strcmp(root->name, path) == 0)
		{
			if (parent != NULL) *parent = root->parent;
			return root;
		}

		if ((p = strchr(path, '/')) != NULL)
		{
			char name[NAME_MAX + 1];
			unsigned len = p - path;

			CRASHCHECK((len + 1) > sizeof(name));

			memcpy(name, path, len);
			name[len] = '\0';
			if ((root = find_sibling(root, name)) != NULL)
			{
				if (parent != NULL) *parent = root;
				return _path_find((apxmgr_attr_t *)LIST_FIRST(root->children), ++p, parent);		// skip over leading '/'
			}
		}
		else
			root = find_sibling(root, path);
	}
	return root; 
}


/*
 * _check_access_perms
 * 
 * local (modified) copy of iofunc_check_access() to eliminate the "root can do
 * anything" pass-through
*/
static int _check_access_perms(resmgr_context_t *ctp, iofunc_attr_t *attr, mode_t check, struct _client_info *info)
{
	mode_t  mode;

	// Must supply an info entry
	if(!info) {
		return ENOSYS;
	}

	// Root can do anything
//	if(info->cred.euid == 0) {
//		return EOK;
//	}

	// Check for matching owner
	if((check & S_ISUID) && info->cred.euid == attr->uid) {
		return EOK;
	}

	// Check for matching group
	if(check & S_ISGID) {
		unsigned  n;

		if(info->cred.egid == attr->gid) {
			return EOK;
		}

		for(n = 0; n < info->cred.ngroups; n++) {
			if(info->cred.grouplist[n] == attr->gid) {
				return EOK;
			}
		}
	}

	// If not checking read/write/exec perms, return error
	if((check &=  S_IRWXU) == 0) {
		return EPERM;
	}

	if(info->cred.euid == attr->uid) {
		mode = attr->mode;
	} else if(info->cred.egid == attr->gid) {
		mode = attr->mode << 3;
	} else {
		unsigned						n;

		mode = attr->mode << 6;
		for(n = 0; n < info->cred.ngroups; n++) {
			if(info->cred.grouplist[n] == attr->gid) {
				mode = attr->mode << 3;
				break;
			}
		}
	}

	// Check if all permissions are true
	if((mode & check) != check) {
		return EACCES;
	}

	return EOK;
}

/*
 * unregister_event
 * 
 * unregister a single event and delete it
*/
static void unregister_event(part_qnodehdr2_t *evlist, part_evt_t *reg_event)
{
	CRASHCHECK(evlist == NULL);
	CRASHCHECK(reg_event == NULL);
	CRASHCHECK(reg_event->inuse > 0);

	LIST_DEL(evlist->list, reg_event);

	free(reg_event);
}



__SRCVERSION("$IQ: apmgr_support.c,v 1.23 $");

