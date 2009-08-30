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
 * apmgr_open
 * 
 * /proc/<pid>/partition resource manager open()
 * 
*/

#include "apmgr.h"
#include "apmmgr.h"
#include "apsmgr.h"

/*
 * apmgr_open
 * 
 * This instance of the apmgr resource manager handles names under "/partition"
 * that are not handled by "/partition/mem" or "/partition/sched". By design,
 * only pseudo partitions or group names can exist under "/partition"
*/
int apmgr_open(resmgr_context_t *ctp, io_open_t *msg, void *extra, void *reserved)
{
	/* at this point we must be opening a pseudo or group name */
	switch (msg->connect.subtype)
	{
		case _IO_CONNECT_COMBINE_CLOSE:
		case _IO_CONNECT_OPEN:
		case _IO_CONNECT_COMBINE:
		{
			struct _client_info  ci;
			apmgr_attr_t *parent;
			apmgr_attr_t *p;
			char *name_p = NULL;
			char *name;
			bool  last = bool_t_FALSE;
			int  r;
			apmgr_ocb_t  *ocb;
			mode_t  dir_mode = 0;
			unsigned ioflag = msg->connect.ioflag & (~_IO_FLAG_MASK | msg->connect.access);
			mode_t  check_mode = ((ioflag & _IO_FLAG_RD) ? S_IREAD : 0) | ((ioflag & _IO_FLAG_WR) ? S_IWRITE : 0);
			bool  recurse = recurse = bool_t_FALSE;

			name = msg->connect.path;
			p = parent = root_npart;

			if ((memcmp(name, "mem", sizeof("mem")-1) == 0) ||
				(memcmp(name, "sched", sizeof("sched")-1) == 0)) {
				return ENOSYS;
			}

			if ((nametoolong(name, PATH_MAX, (void *)apmgr_devno)) ||
				(nametoolong(name, NAME_MAX, (void *)apmgr_devno)))
				return ENAMETOOLONG;

			/* get client info now. It will be required at some point */			
			if ((r = iofunc_client_info(ctp, msg->connect.ioflag, &ci)) != EOK)
				return r;

			/* try and locate 'name' */
			if ((p = npath_find((apmgr_attr_t *)LIST_FIRST(root_npart->children), name, &parent)) == NULL)
			{
				parent = root_npart;
				/* parse the connect path */
				while(1)
				{
					if ((name_p = strchr(name, '/')) == NULL)
						last = bool_t_TRUE;
					else
						*name_p++ = '\0';		// remove the '/'
					
					if (((p = npath_find((apmgr_attr_t *)LIST_FIRST(parent->children), name, NULL)) == NULL) && !last)
					{
						return ENOENT;
					}
				
					if (last)
						break;

					parent = p;
					name = name_p;
				}
			}

			if ((parent == root_npart) && (*name == '\0'))
				p = parent;

			if ((p == NULL) && !(msg->connect.ioflag & O_CREAT))
				return ENOENT;
			else if ((p != NULL) && (msg->connect.ioflag & O_CREAT))
				return EEXIST;

			CRASHCHECK((parent == NULL) && (p == NULL));
			CRASHCHECK((parent == NULL) && (p->type != part_type_ROOT));
			
			/* LOCK the required attributes structures */
			if ((r = PART_ATTR_LOCK(parent)) != EOK)
				return r;

			if ((r = PART_ATTR_LOCK(p)) != EOK)
			{
				PART_ATTR_UNLOCK(parent);
				return r;
			}
			/*
			 * check access to either the name to be opened, or the parent if
			 * creating a new entry. Note that we must temporarily change the
			 * attributes of 'p' to remove the S_IFDIR flag to prevent
			 * iofunc_open() returning EISDIR when opening for write
			*/
			dir_mode = ((p != NULL) ? p->attr.mode & S_IFDIR : 0);
			if (dir_mode) p->attr.mode &= ~S_IFDIR;
			if ((r = iofunc_open(ctp, msg, p ? &p->attr : NULL, p ? NULL : &parent->attr, &ci)) != EOK)
			{
				/*
				 * we mark names in the partition manager namespace as directories
				 * in order to identify them as having children and to allow tools
				 * like 'ls -R' to be used to browse the namespace hierarchy.
				 * iofunc_open() however does not allow writing directories so we
				 * have to do some additional checking. If the caller is opening
				 * for writing, this indicates that the partition attributes will
				 * be modified and we only allow this is the partition mode is
				 * set appropriately
				*/
//				if (r != EISDIR)
				if (p != NULL) p->attr.mode |= dir_mode;	// restore
				PART_ATTR_UNLOCK(p);
				PART_ATTR_UNLOCK(parent);
				return r;	// some other error 
			}
			if (p != NULL) p->attr.mode |= dir_mode;	// restore
			
			/*
			 * if creating a new name under parent, make sure write permission
			 * exists. This is necessary since the
			 * iofunc_open()->iofunc_create()->iofunc_check_access() call chain above
			 * will succeed if euid == 0 (ie iofunc_check_access() does this)
			 * Also, if p == NULL, we are creating a new entry and therefore write
			 * access is required in the parent hierarchy, therfore recurse == bool_t_TRUE;
			 * If p != NULL, we only check the permissions of p
			*/
			if (p == NULL)
			{
				check_mode |= S_IWRITE;
				recurse = bool_t_TRUE;
			}	
			if ((check_mode != 0) && ((r = check_access_perms(ctp, (apxmgr_attr_t *)(p ? p : parent), check_mode, &ci, recurse)) != EOK))
			{
				PART_ATTR_UNLOCK(p);
				PART_ATTR_UNLOCK(parent);
				return r;
			}
			
			if ((p == NULL) && (parent != NULL) && (msg->connect.ioflag & O_CREAT))
			{
				int rr;
				
				if ((p == NULL) && ((p = calloc(1, sizeof(*p))) == NULL))
				{
					PART_ATTR_UNLOCK(parent);
					return ENOMEM;
				}

				/* lock for common exit code */
				if ((rr = PART_ATTR_LOCK(p)) != EOK)
				{
					PART_ATTR_UNLOCK(parent);
					free(p);
					return rr;
				}
				/*
				 * request is to create a new name. If the name refers to an
				 * existing memory class, the name will be created as such,
				 * otherwise it will be considered a partition. Whether or
				 * not it is a group name (to hold pseudo partitions) depends
				 * on whether the parent is a memory class name. If so, this
				 * is NOT a group name, otherwise it must be
				*/
				switch (parent->type)
				{
					/*
					 * under the root, only a partition group or pseudo name
					 * is legal
					*/
					case part_type_ROOT:	
					case part_type_GROUP:
					{
						/*
						 * under a partition group name, all names are either
						 * additional group names or pseudo partitions. Pseudo
						 * partitions are handled by apmmgr_link() therefore
						 * only a new partition group name will be handled by
						 * this code path
						*/
						p->type = part_type_GROUP;
						p->name = strdup(name);
						LIST_INIT(p->children);
						p->hdr.next = NULL;
						p->hdr.prev = NULL;
						p->parent = parent;
						break;
					}
					default:
					{
						PART_ATTR_UNLOCK(p);
						PART_ATTR_UNLOCK(parent);
						free(p);
						return EBADF;
					}
				}
//				iofunc_attr_init(&p->attr, parent->attr.mode, &parent->attr, &ci);
				iofunc_attr_init(&p->attr, (msg->connect.mode & S_IPERMS) | (parent->attr.mode & S_IFMT), &parent->attr, &ci);
				if (S_ISDIR(p->attr.mode)) ++p->attr.nlink;	// iofunc_unlink() requires nlink >=2
				/* insert into sibling list */
				LIST_ADD(parent->children, p);
				if (S_ISDIR(p->attr.mode)) ++parent->attr.nlink;
			}

			// all partition related memory is currently from system, ocb should not be
			if	((ocb = calloc(1, sizeof(*ocb))) == NULL)
			{
				PART_ATTR_UNLOCK(p);
				PART_ATTR_UNLOCK(parent);
				return ENOMEM;
			}

			r = iofunc_ocb_attach(ctp, msg, &ocb->ocb, &p->attr, &apmgr_io_funcs);
#ifndef NDEBUG
			if ((r != EOK) && (ker_verbose)) kprintf("%d: ocb attach failed, err = %d\n", __LINE__, r);
#endif
			if ( r != EOK ) {
				free( ocb );
			} else {
				ocb->apmgr_type = (p == root_npart) ? apmgr_type_PART : apmgr_type_NAME;
				ocb->pid = -1;
				ocb->attr = p;
			}
			PART_ATTR_UNLOCK(p);
			PART_ATTR_UNLOCK(parent);
			return r; 
		}
		default:
			return ENOSYS;
	}
}


__SRCVERSION("$IQ: apmgr_open.c,v 1.23 $");

