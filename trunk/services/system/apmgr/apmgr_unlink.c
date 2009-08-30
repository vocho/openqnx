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
 * apmgr_unlink
 * 
 * Provide resource manager unlink() processing for the partitioning module
 * 
*/

#include "apmgr.h"

int apmgr_unlink(resmgr_context_t *ctp, io_unlink_t *msg, void *handle, void *reserved)
{
	switch (msg->connect.subtype)
	{
		case _IO_CONNECT_UNLINK:
		{
			int  r;
			apmgr_attr_t *parent = root_npart;
			apmgr_attr_t *p = npath_find((apmgr_attr_t *)LIST_FIRST(root_npart->children), msg->connect.path, &parent);
			struct _client_info  ci;

			/* FIX ME
			 * On recursive remove, we get for example sys/1, /sys/1/partition, sys/1/as.
			 * These should not return ENOENT, but rather EPERM as this operation is
			 * not permitted on a real partition. This requires parsing the msg->connect.path
			 * in order to find out whether we have decsended into a 'pid' directory
			 * (which would normally be redirected) and if so, return EPERM
			*/
			if (p == NULL)
			{
				char *part_name = msg->connect.path;
				bool  last = bool_t_FALSE;
				char *name_p;
				
				parent = root_npart;
				/* parse the connect path */
				while(1)
				{
					if ((name_p = strchr(part_name, '/')) == NULL)
						last = bool_t_TRUE;
					else
						*name_p++ = '\0';		// remove the '/'

					if (((p = npath_find((apmgr_attr_t *)LIST_FIRST(parent->children), part_name, NULL)) == NULL) && !last)
					{
						return ENOENT;		// intermediate pathname missing
					}
					if (last)
						break;

					parent = p;
					part_name = name_p;
				}
				return ENOENT;
			}

			CRASHCHECK(parent == NULL);
			CRASHCHECK(p == NULL);

			/* does someone have it opened ? */
			if (p->attr.count != 0)
				return EBUSY;

			if ((r = PART_ATTR_LOCK(parent)) != EOK)
					return r;

			if ((r = PART_ATTR_LOCK(p)) != EOK)
			{
				PART_ATTR_UNLOCK(parent);
				return r;
			}

			/*
			 * this check does not catch associated processes for real partitions,
			 * they are caught below. It will catch child partitions however
			*/
			if (LIST_FIRST(p->children) != NULL)
			{
				PART_ATTR_UNLOCK(p);
				PART_ATTR_UNLOCK(parent);
				return ENOTEMPTY;
			}

			CRASHCHECK(p->parent == NULL);
			CRASHCHECK(LIST_FIRST(parent->children) == NULL);
			CRASHCHECK(p->attr.nlink < (S_ISDIR(p->attr.mode) ? 2 : 1));
			CRASHCHECK(parent->attr.nlink < 2);

			/*
			 * check unlink permissions
			 * if removing a name under parent, make sure write permission
			 * exists. This is necessary since the iofunc_unlink()->iofunc_check_access()
			 * call chain will succeed if euid == 0 (ie iofunc_check_access() does this)
			 * Important
			 * The check_access_perms() should be before iofunc_unlink() to avoid
			 * having to undo iofunc_unlink() operations.
			 * Also, we check 'p' for access permissions and 'parent' for
			 * writeablility
			*/
			if (((r = iofunc_client_info(ctp, msg->connect.ioflag, &ci)) != EOK) ||
				((r = check_access_perms(ctp, (apxmgr_attr_t *)p, S_IWUSR | S_IWGRP | S_IWOTH, &ci, bool_t_FALSE)) != EOK) ||
				((r = check_access_perms(ctp, (apxmgr_attr_t *)parent, S_IWRITE, &ci, bool_t_FALSE)) != EOK))
			{
				PART_ATTR_UNLOCK(p);
				PART_ATTR_UNLOCK(parent);
				return r;
			}

			/* handle 'name' type specific processing */
			switch (p->type)
			{
				case part_type_MEMPART_PSEUDO:
					free(p->symlink);
					break;

				default:
					break;
			}
			r = iofunc_unlink(ctp, msg, &p->attr, parent ? &parent->attr : NULL, &ci);
			/* delete the name from the sibling list */
			LIST_DEL(parent->children, p);
// done by iofunc_unlink() above			--parent->attr.nlink;
//			free((char *)p->name);
			free((char *)p->name);
			p->name = NULL;
			PART_ATTR_UNLOCK(p);
			PART_ATTR_UNLOCK(parent);
			
			free(p);

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


__SRCVERSION("$IQ: apmgr_unlink.c,v 1.23 $");

