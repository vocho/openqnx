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
 * apmmgr_unlink
 * 
 * Provide resource manager unlink() processing for the memory partitioning
 * module
 * 
*/

#include "apmmgr.h"

int apmmgr_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *reserved)
{
	switch (msg->connect.subtype)
	{
		case _IO_CONNECT_UNLINK:
		{
			int  r;
			unsigned e;
			apmmgr_attr_t *mp_parent = root_mpart;
			apmmgr_attr_t *mp = mpath_find((apmmgr_attr_t *)LIST_FIRST(root_mpart->children), msg->connect.path, &mp_parent);
			struct _client_info  ci;

			/* FIX ME
			 * On recursive remove, we get for example sys/1, /sys/1/partition, sys/1/as.
			 * These should not return ENOENT, but rather EPERM as this operation is
			 * not permitted on a real partition. This requires parsing the msg->connect.path
			 * in order to find out whether we have decsended into a 'pid' directory
			 * (which would normally be redirected) and if so, return EPERM
			*/
			if (mp == NULL)
			{
				char *part_name = msg->connect.path;
				bool  last = bool_t_FALSE;
				char *name_p;
				
				mp_parent = root_mpart;
				/* parse the connect path */
				while(1)
				{
					if ((name_p = strchr(part_name, '/')) == NULL) {
						last = bool_t_TRUE;
					} else {
						*name_p++ = '\0';		// remove the '/'
					}

					if (((mp = mpath_find((apmmgr_attr_t *)LIST_FIRST(mp_parent->children), part_name, NULL)) == NULL) && !last)
					{
						/*
						 * could not find the entry. See if it looks like a pid.
						 * FIX ME - may have to do more to verify a pid. Also, what
						 * happens if a partition is created with the same name as
						 * an associated pid. Currently, we won't find it so should
						 * check.
						 * Alternatively, I could make the PID's be links to /proc/
						*/
						if ((mp_parent->type == part_type_MEMPART_REAL) &&
							(isStringOfDigits(part_name))) {
							return EACCES;
						} else {
							return ENOENT;		// intermediate pathname missing
						}
					}
					if (last) break;

					mp_parent = mp;
					part_name = name_p;
				}
				return ENOENT;
			}

			CRASHCHECK(mp_parent == NULL);
			CRASHCHECK(mp == NULL);

			/* does someone have it opened ? */
			if (mp->attr.count != 0) {
				return EBUSY;
			}

			if ((r = PART_ATTR_LOCK(mp_parent)) != EOK) {
				return r;
			} else if ((r = PART_ATTR_LOCK(mp)) != EOK) {
				PART_ATTR_UNLOCK(mp_parent);
				return r;
			}

			/* make sure the partition is not marked permanent thus preventing removal */
			if (mp->type == part_type_MEMPART_REAL)
			{
				mempart_info_t tmp_info;
				mempart_info_t *meminfo = MEMPART_GETINFO(mp->data.mpid, &tmp_info);

				CRASHCHECK(meminfo == NULL);

				if (meminfo->cur_cfg.policy.permanent == bool_t_TRUE)
				{
					PART_ATTR_UNLOCK(mp);
					PART_ATTR_UNLOCK(mp_parent);
					return EACCES;
				}
			}
			
			/*
			 * this check does not catch associated processes for real partitions,
			 * they are caught below. It will catch child partitions however
			*/
			if (LIST_FIRST(mp->children) != NULL)
			{
				PART_ATTR_UNLOCK(mp);
				PART_ATTR_UNLOCK(mp_parent);
				return ENOTEMPTY;
			}

			CRASHCHECK(mp->parent == NULL);
			CRASHCHECK(LIST_FIRST(mp_parent->children) == NULL);
			CRASHCHECK(mp->attr.nlink < (S_ISDIR(mp->attr.mode) ? 2 : 1));
			CRASHCHECK(mp_parent->attr.nlink < 2);
			
			/* 
			 * try and clean up any inactive events. There should not be any
			 * on the active lists, but the inactive lists may still need to be
			 * be cleaned up
			*/
			if (mp->type == part_type_MEMPART_REAL) {
				(void)unregister_events((apxmgr_attr_t *)mp, NULL, NUM_MEMPART_EVTTYPES);
			} else if (mp->type == part_type_MEMCLASS) {
				(void)unregister_events((apxmgr_attr_t *)mp, NULL, NUM_MEMCLASS_EVTTYPES);
			}

			for (e=0; e<NUM_ELTS(mp->event_list); e++)
			{
			#ifndef NDEBUG
				CRASHCHECK(LIST_COUNT(mp->event_list[e].active.list) != 0);
				CRASHCHECK(LIST_FIRST(mp->event_list[e].active.list) != NULL);
				CRASHCHECK(LIST_COUNT(mp->event_list[e].inactive.list) != 0);
				CRASHCHECK(LIST_FIRST(mp->event_list[e].inactive.list) != NULL);
			#endif	/* NDEBUG */
				if ((LIST_COUNT(mp->event_list[e].active.list) != 0) ||
					(LIST_COUNT(mp->event_list[e].inactive.list) != 0)) {
					return EMORE;	/* something unique */
				}
			}

			/*
			 * check unlink permissions
			 * if removing a name under mp_parent, make sure write permission
			 * exists. This is necessary since the iofunc_unlink()->iofunc_check_access()
			 * call chain will succeed if euid == 0 (ie iofunc_check_access() does this)
			 * Important
			 * The check_access_perms() should be before iofunc_unlink() to avoid
			 * having to undo iofunc_unlink() operations.
			 * Also, we check 'mp' for access permissions and 'mp_parent' for
			 * writeablility
			*/
			if (((r = iofunc_client_info(ctp, msg->connect.ioflag, &ci)) != EOK) ||
				((r = check_access_perms(ctp, (apxmgr_attr_t *)mp, S_IWUSR | S_IWGRP | S_IWOTH, &ci, bool_t_FALSE)) != EOK) ||
				((r = check_access_perms(ctp, (apxmgr_attr_t *)mp_parent, S_IWRITE, &ci, bool_t_FALSE)) != EOK))
			{
				PART_ATTR_UNLOCK(mp);
				PART_ATTR_UNLOCK(mp_parent);
				return r;
			}

			/* handle 'name' type specific processing */
			switch (mp->type)
			{
				case part_type_MEMCLASS:
				{
					memclass_entry_t * mce = mp->data.mclass_entry;

					CRASHCHECK(mce == NULL);

					/* can't delete the system ram memory class */
					if (mce->data.info.id == sys_memclass_id)
					{
						PART_ATTR_UNLOCK(mp);
						PART_ATTR_UNLOCK(mp_parent);
						return EPERM;
					}
					
					(void)memclass_delete(NULL, mce->data.info.id);

					break;
				}
				
				case part_type_MEMPART_REAL:
				{
					CRASHCHECK(mp->data.mpid == part_id_t_INVALID);

					/* can't delete the system ram memory partition */
					if (mp->data.mpid == MEMPART_T_TO_ID(sys_mempart))
					{
						PART_ATTR_UNLOCK(mp);
						PART_ATTR_UNLOCK(mp_parent);
						return EPERM;
					}

#ifdef USE_PROC_OBJ_LISTS
					{
					mempart_t *mpart = MEMPART_ID_TO_T(mp->data.mpid);
					/* make sure there are no associated processes */
					if ((LIST_FIRST(mpart->prp_list) != NULL) ||
						(LIST_FIRST(mpart->obj_list) != NULL))
					{
						PART_ATTR_UNLOCK(mp);
						PART_ATTR_UNLOCK(mp_parent);
						return EBUSY;
					}
					}
#endif	/* USE_PROC_OBJ_LISTS */

					/* destroy the memory partition */
					if ((r = MEMPART_DESTROY(mp->data.mpid)) != EOK)
					{
						PART_ATTR_UNLOCK(mp);
						PART_ATTR_UNLOCK(mp_parent);
						return r;
					}
					break;
				}

				case part_type_MEMPART_PSEUDO:
					free(mp->data.symlink);
					break;

				default:
					break;
			}
			r = iofunc_unlink(ctp, msg, &mp->attr, mp_parent ? &mp_parent->attr : NULL, &ci);
			/* delete the name from the sibling list */
			LIST_DEL(mp_parent->children, mp);
// done by iofunc_unlink() above			--mp_parent->attr.nlink;
//			free((char *)mp->name);
			free((char *)mp->name);
			mp->name = NULL;
			PART_ATTR_UNLOCK(mp);
			PART_ATTR_UNLOCK(mp_parent);
			
			free(mp);

			/* have a bit of a problem at this point if iofunc_unlink() reported
			 * a failure. I had this code near the top by check_access_perms()
			 * but that caused a problem with iofunc_unlink() doing its thing
			 * and then a real partition reporting a non NULL prp or object list
			 * (these things take time to cleanup). For now, I will assert the
			 * debug load and return whatever iofunc_unlink() returns on a regular
			 * load. In both cases, everything will be removed. 
			*/
			CRASHCHECK(r != EOK);

			return r;
		}
		default:
			return ENOSYS;
	}
}


__SRCVERSION("$IQ: apmmgr_unlink.c,v 1.23 $");

