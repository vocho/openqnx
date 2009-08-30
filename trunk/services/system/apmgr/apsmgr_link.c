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
 * apsmgr_link
 * 
 * Provide resource manager symlink processing for the scheduler partitioning module.
 * This functionality implements the pseudo partition scheme which allows a
 * single name to refer to multiple partitions of differing scheduler classes so
 * that a single name can be used for association.
 * 
*/

#include "apsmgr.h"


int apsmgr_link(resmgr_context_t *ctp, io_link_t *msg, void *attr, io_link_extra_t *extra)
{
	switch (msg->connect.extra_type)
	{
		case _IO_CONNECT_EXTRA_SYMLINK:
		{
			struct _client_info  ci;
			int  r;
			apsmgr_attr_t  *mp, *mp_parent, *mp_real;
			char *pseudo_name = msg->connect.path;
			char *part_name = extra->path;
			char *chk_part_name = part_name;
			bool  last = bool_t_FALSE;

//FIX ME - check extra_len first
			if ((nametoolong(part_name, PATH_MAX, (void *)apsmgr_devno)) ||
				(nametoolong(part_name, NAME_MAX, (void *)apsmgr_devno)))
				return ENAMETOOLONG;

			/* insist on the real partition existing before allowing a pseudo to it */
			if (memcmp(part_name, "/partition/sched/", sizeof("/partition/sched/")-1) == 0)
				chk_part_name += sizeof("/partition/sched/") - 1;

			if ((mp_real = spath_find((apsmgr_attr_t *)LIST_FIRST(root_spart->children), chk_part_name, NULL)) == NULL)
				return ENOENT;
/*
			mp_parent = NULL;
			if ((mp = spath_find((apsmgr_attr_t *)LIST_FIRST(root_spart->children), pseudo_name, &mp_parent)) != NULL)
				return EEXIST;
*/
			mp_parent = root_spart;
			/* parse the pseudo path */
			chk_part_name = pseudo_name;
			while(1)
			{
				char *name_p;
				
				if ((name_p = strchr(chk_part_name, '/')) == NULL)
					last = bool_t_TRUE;
				else
					*name_p = '\0';		// remove the '/'

				mp = spath_find((apsmgr_attr_t *)LIST_FIRST(mp_parent->children), chk_part_name, NULL);

				if (name_p != NULL)
					*name_p++ = '/';	// restore the '/'

				if ((mp == NULL) && !last)
					return ENOENT;					// intermediate pathname missing
				else if ((mp != NULL) && (last))	// link target already exists
					return EEXIST;
				else if ((mp == NULL) && last)		// all intermediate names exist, target does not
					break;
				else
				{
					mp_parent = mp;
					chk_part_name = name_p;
				}
			}

			CRASHCHECK(mp_parent == NULL);

			if ((r = PART_ATTR_LOCK(mp_parent)) != EOK)
				return r;

			if ((r = PART_ATTR_LOCK(mp)) != EOK)
			{
					PART_ATTR_UNLOCK(mp_parent);
					return r;
			}

			/* cannot create a pseudo partition as a child or a real/pseudo partition */
			if ((mp_parent->type == part_type_SCHEDPART_REAL) ||
				(mp_parent->type == part_type_SCHEDPART_PSEUDO))
			{
				PART_ATTR_UNLOCK(mp);
				PART_ATTR_UNLOCK(mp_parent);
				return EACCES;
			}

			if ((r = iofunc_client_info(ctp, msg->connect.ioflag, &ci)) != EOK)
			{
				PART_ATTR_UNLOCK(mp);
				PART_ATTR_UNLOCK(mp_parent);
				return r;
			}

			if ((r = iofunc_open(ctp, (io_open_t *)msg, mp ? &mp->attr : NULL, mp ? NULL : &mp_parent->attr, &ci)) != EOK)
			{
				PART_ATTR_UNLOCK(mp);
				PART_ATTR_UNLOCK(mp_parent);
				return r;
			}
			
			if (msg->connect.ioflag & O_CREAT)
			{
				char *p;

				/* get the basename */
				if ((p = strrchr(pseudo_name, '/')) != NULL)
					pseudo_name = ++p;

				if ((mp == NULL) &&	((mp = calloc(1, sizeof(*mp))) == NULL))
				{
					PART_ATTR_UNLOCK(mp_parent);
					return ENOMEM;
				}

				mp->type = part_type_SCHEDPART_PSEUDO;
				mp->name = strdup(pseudo_name);
				LIST_INIT(mp->children);
				mp->hdr.next = NULL;
				mp->hdr.prev = NULL;
				mp->parent = mp_parent;
				/* use the spart field point to the schedpart_part_t of the real partition */
				mp->data.symlink = strdup(part_name);

				iofunc_attr_init(&mp->attr, S_IFLNK | (msg->connect.mode & ~S_IFMT), &mp_parent->attr, &ci);		
//				iofunc_attr_init(&mp->attr, S_IFLNK | (mp_parent->attr.mode & ~S_IFMT), &mp_parent->attr, &ci);		
				/* insert into sibling list */
				LIST_ADD(mp_parent->children, mp);
				if (S_ISDIR(mp->attr.mode)) ++mp_parent->attr.nlink;

				PART_ATTR_UNLOCK(mp_parent);
				return EOK;
			}
			else
			{
				PART_ATTR_UNLOCK(mp);
				PART_ATTR_UNLOCK(mp_parent);
				return EACCES;
			}
			break;
		}
		
		default:
			break;
	}
	return ENOSYS;
}


__SRCVERSION("$IQ: apsmgr_link.c,v 1.23 $");

