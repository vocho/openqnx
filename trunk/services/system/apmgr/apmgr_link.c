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
 * apmgr_link
 * 
 * Provide resource manager symlink processing for the common partitioning module.
 * This functionality implements the pseudo partition scheme
*/

#include "apmgr.h"
#include "apmmgr.h"
#include "apsmgr.h"

int apmgr_link(resmgr_context_t *ctp, io_link_t *msg, void *attr, io_link_extra_t *extra)
{
	switch (msg->connect.extra_type)
	{
		case _IO_CONNECT_EXTRA_SYMLINK:
		{
			struct _client_info  ci;
			int  r;
			apmgr_attr_t  *p, *parent;
			char *pseudo_name = msg->connect.path;
			char *part_name = extra->path;
			char *chk_part_name = part_name;
			bool  last = bool_t_FALSE;
			part_type_t  part_type;

//FIX ME - check extra_len first
			if ((nametoolong(part_name, PATH_MAX, (void *)apmmgr_devno)) ||
				(nametoolong(part_name, NAME_MAX, (void *)apmmgr_devno)))
				return ENAMETOOLONG;

			/* insist on the real partition existing before allowing a pseudo to it */
			if (memcmp(part_name, "/partition/mem/", sizeof("/partition/mem/")-1) == 0)
			{
				chk_part_name += sizeof("/partition/mem/") - 1;

				if (APMMGR_GETATTR(chk_part_name) == NULL) {
					return ENOENT;
				}
				part_type = part_type_MEMPART_PSEUDO;
			}
			else if (memcmp(part_name, "/partition/sched/", sizeof("/partition/sched/")-1) == 0)
			{
				chk_part_name += sizeof("/partition/sched/") - 1;

				if (APSMGR_GETATTR(chk_part_name) == NULL) {
					return ENOENT;
				}
				part_type = part_type_SCHEDPART_PSEUDO;
			}
			else
			{
				/* don't currently allow pseudo -> pseudo -> ... -> pseudo -> real */
				return EINVAL;
			}

			parent = root_npart;
			/* parse the pseudo path */
			chk_part_name = pseudo_name;
			while(1)
			{
				char *name_p;
				
				if ((name_p = strchr(chk_part_name, '/')) == NULL)
					last = bool_t_TRUE;
				else
					*name_p = '\0';		// remove the '/'

				p = npath_find((apmgr_attr_t *)LIST_FIRST(parent->children), chk_part_name, NULL);

				if (name_p != NULL)
					*name_p++ = '/';	// restore the '/'

				if ((p == NULL) && !last)
					return ENOENT;					// intermediate pathname missing
				else if ((p != NULL) && (last))	// link target already exists
					return EEXIST;
				else if ((p == NULL) && last)		// all intermediate names exist, target does not
					break;
				else
				{
					parent = p;
					chk_part_name = name_p;
				}
			}

			CRASHCHECK(parent == NULL);

			if ((r = PART_ATTR_LOCK(parent)) != EOK)
				return r;

			if ((r = PART_ATTR_LOCK(p)) != EOK)
			{
					PART_ATTR_UNLOCK(parent);
					return r;
			}

			CRASHCHECK((parent->type != part_type_ROOT) && (parent->type != part_type_GROUP));

			if ((r = iofunc_client_info(ctp, msg->connect.ioflag, &ci)) != EOK)
			{
				PART_ATTR_UNLOCK(p);
				PART_ATTR_UNLOCK(parent);
				return r;
			}

			if ((r = iofunc_open(ctp, (io_open_t *)msg, p ? &p->attr : NULL, p ? NULL : &parent->attr, &ci)) != EOK)
			{
				PART_ATTR_UNLOCK(p);
				PART_ATTR_UNLOCK(parent);
				return r;
			}
			
			if (msg->connect.ioflag & O_CREAT)
			{
				char *s;

				CRASHCHECK(p != NULL);

				/* get the basename */
				if ((s = strrchr(pseudo_name, '/')) != NULL)
					pseudo_name = ++s;

				if ((p == NULL) &&	((p = calloc(1, sizeof(*p))) == NULL))
				{
					PART_ATTR_UNLOCK(parent);
					return ENOMEM;
				}

				p->type = part_type;
				p->name = strdup(pseudo_name);
				LIST_INIT(p->children);
				p->hdr.next = NULL;
				p->hdr.prev = NULL;
				p->parent = parent;
				p->symlink = strdup(part_name);

				iofunc_attr_init(&p->attr, S_IFLNK | (msg->connect.mode & ~S_IFMT), &parent->attr, &ci);		
//				iofunc_attr_init(&p->attr, S_IFLNK | (parent->attr.mode & ~S_IFMT), &parent->attr, &ci);		
				/* insert into sibling list */
				LIST_ADD(parent->children, p);
				if (S_ISDIR(p->attr.mode)) ++parent->attr.nlink;

				PART_ATTR_UNLOCK(parent);
				return EOK;
			}
			else
			{
				PART_ATTR_UNLOCK(p);
				PART_ATTR_UNLOCK(parent);
				return EACCES;
			}
			break;
		}
		
		default:
			break;
	}
	return ENOSYS;
}


__SRCVERSION("$IQ: apmmgr_link.c,v 1.23 $");

