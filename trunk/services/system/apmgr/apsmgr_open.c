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
 * apsmgr_open
 * 
 * Provide resource manager open() processing for the scheduler partitioning module
 * 
*/

#include "apsmgr.h"

static int redirect_pid_open(resmgr_context_t *ctp, io_open_t *msg, char *name);

int apsmgr_open(resmgr_context_t *ctp, io_open_t *msg, void *extra, void *reserved)
{
	switch (msg->connect.subtype)
	{
		case _IO_CONNECT_OPEN:
		case _IO_CONNECT_COMBINE:
		case _IO_CONNECT_COMBINE_CLOSE:
		{
			struct _client_info  ci;
			apsmgr_attr_t *mp_parent;
			apsmgr_attr_t *mp;
			char *name_p = NULL;
			char *part_name;
			bool  last = bool_t_FALSE;
			int  r;
			apsmgr_ocb_t  *ocb;
			mode_t  dir_mode = 0;
			unsigned ioflag = msg->connect.ioflag & (~_IO_FLAG_MASK | msg->connect.access);
			mode_t  check_mode = ((ioflag & _IO_FLAG_RD) ? S_IREAD : 0) | ((ioflag & _IO_FLAG_WR) ? S_IWRITE : 0);
			bool  recurse = recurse = bool_t_FALSE;
			unsigned initial_create = 0;

			part_name = msg->connect.path;
			mp = mp_parent = root_spart;

			if ((nametoolong(part_name, PATH_MAX, (void *)apsmgr_devno)) ||
				(nametoolong(part_name, NAME_MAX, (void *)apsmgr_devno)))
				return ENAMETOOLONG;

			/* get client info now. It will be required at some point */			
			if ((r = iofunc_client_info(ctp, msg->connect.ioflag, &ci)) != EOK)
				return r;

			/* try and locate 'part_name' */
			if ((mp = spath_find((apsmgr_attr_t *)LIST_FIRST(root_spart->children), part_name, &mp_parent)) == NULL)
			{
				mp_parent = root_spart;
				/* parse the connect path */
				while(1)
				{
					if ((name_p = strchr(part_name, '/')) == NULL)
						last = bool_t_TRUE;
					else
						*name_p++ = '\0';		// remove the '/'
					
					if (((mp = spath_find((apsmgr_attr_t *)LIST_FIRST(mp_parent->children), part_name, NULL)) == NULL) && !last)
					{
						/*
						 * could not find the entry. See if it looks like a pid.
						 * Note that we prevent creating names under real partitions
						 * that could conflict with a pid_t so a string of digits
						 * truly is a pid at this point.
						*/
check_for_pid:
						if ((mp_parent->type == part_type_SCHEDPART_REAL) &&
							(isStringOfDigits(part_name)))
						{
							pid_t part_pid = strtoul(part_name, NULL, NULL);
							mode_t chk_mode = 0;
							ioflag = msg->connect.ioflag & (~_IO_FLAG_MASK | msg->connect.access);
							chk_mode |= (ioflag & _IO_FLAG_RD) ? S_IREAD : 0;
							chk_mode |= (ioflag & _IO_FLAG_WR) ? S_IWRITE : 0;

							/*
							 * before we do any redirection, let's check the
							 * access permissions of the parent (ie. real partition)
							 * and also ensure that this process is really associated
							 * with this partition
							*/
							if ((chk_mode != 0) &&	// ??
								((r = check_access_perms(ctp, (apxmgr_attr_t *)mp_parent, chk_mode, &ci, bool_t_FALSE)) != EOK))
									return r;
							else if ((mp_parent->data.spid == part_id_t_INVALID) ||
									 (SCHEDPART_FINDPID(part_pid, mp_parent->data.spid) == NULL))
								return ENOENT;
							else
							{
								char  name[PATH_MAX + 1];
								if ((name_p != NULL) && (*name_p != '\0'))
								{
									*(name_p - 1) = '/';	// restore '/'
									/*
									 * this next line terminates the listing of
									 * /partition/sched/<partition>/. The contents
									 * below this root should be accessed from /proc/<pid>/partition/
									 * Note that the dummy_attr structure results in a stat of
									 * '0' size, thus terminating the listing
									*/						
									if (memcmp(name_p, "partition", sizeof("partition")-1) == 0)
									{
										static apsmgr_attr_t  dummy_attr = {{NULL}};
										ocb = calloc(1, sizeof(*ocb));
										if (ocb == NULL) return ENOMEM;
										/* FIX ME - should only do this init once */
										iofunc_attr_init(&dummy_attr.attr, 0000 | S_IFDIR, NULL, NULL);
										// all partition related scheduler is currently from system, ocb should not be
										r = iofunc_ocb_attach(ctp, msg, &ocb->ocb, &dummy_attr.attr, NULL);
										if ( r != EOK )
											free(ocb);
										return r;
									}
								}
								STRLCPY(name, part_name, sizeof(name));
								return redirect_pid_open(ctp, msg, name);
							}
						}
						else
							return ENOENT;		// intermediate pathname missing
					}
				
					if (last)
						break;

					mp_parent = mp;
					part_name = name_p;
				}
			}

			if ((mp_parent == root_spart) && (*part_name == '\0'))
				mp = mp_parent;

			if ((mp == NULL) && !(msg->connect.ioflag & O_CREAT))
				goto check_for_pid;
			else if ((mp != NULL) && (msg->connect.ioflag & O_CREAT))
				return EEXIST;

			CRASHCHECK((mp_parent == NULL) && ((mp == NULL) || (mp->type != part_type_ROOT)));
			
			/* LOCK the required attributes structures */
			if ((r = PART_ATTR_LOCK(mp_parent)) != EOK)
				return r;

			if ((r = PART_ATTR_LOCK(mp)) != EOK)
			{
				PART_ATTR_UNLOCK(mp_parent);
				return r;
			}
			/*
			 * check access to either the name to be opened, or the parent if
			 * creating a new entry. Note that we must temporarily change the
			 * attributes of 'mp' to remove the S_IFDIR flag to prevent
			 * iofunc_open() returning EISDIR when opening for write
			*/
			dir_mode = ((mp != NULL) ? mp->attr.mode & S_IFDIR : 0);
			if (dir_mode) mp->attr.mode &= ~S_IFDIR;
			if ((r = iofunc_open(ctp, msg, mp ? &mp->attr : NULL, mp ? NULL : &mp_parent->attr, &ci)) != EOK)
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
				if (mp != NULL) mp->attr.mode |= dir_mode;	// restore
				PART_ATTR_UNLOCK(mp);
				PART_ATTR_UNLOCK(mp_parent);
				return r;	// some other error 
			}
			if (mp != NULL) mp->attr.mode |= dir_mode;	// restore
			
			/*
			 * if creating a new name under mp_parent, make sure write permission
			 * exists. This is necessary since the
			 * iofunc_open()->iofunc_create()->iofunc_check_access() call chain above
			 * will succeed if euid == 0 (ie iofunc_check_access() does this)
			 * Also, if mp == NULL, we are creating a new entry and therefore write
			 * access is required in the parent hierarchy, therfore recurse == bool_t_TRUE;
			 * If mp != NULL, we only check the permissions of mp
			*/
			if (mp == NULL)
			{
				check_mode |= S_IWRITE;
				recurse = bool_t_TRUE;
			}	
			if ((check_mode != 0) && ((r = check_access_perms(ctp, (apxmgr_attr_t *)(mp ? mp : mp_parent), check_mode, &ci, recurse)) != EOK))
			{
				PART_ATTR_UNLOCK(mp);
				PART_ATTR_UNLOCK(mp_parent);
				return r;
			}
			
			if ((mp == NULL) && (mp_parent != NULL) && (msg->connect.ioflag & O_CREAT))
			{
				int rr;
				
				if ((mp == NULL) && ((mp = calloc(1, sizeof(*mp))) == NULL))
				{
					PART_ATTR_UNLOCK(mp_parent);
					return ENOMEM;
				}

				/* lock for common exit code */
				if ((rr = PART_ATTR_LOCK(mp)) != EOK)
				{
					PART_ATTR_UNLOCK(mp_parent);
					free(mp);
					return rr;
				}
				/*
				 * request is to create a new name. If the parent is a real
				 * scheduler partition then the new name must also be a real
				 * scheduler partition
				*/
				switch (mp_parent->type)
				{	
					case part_type_SCHEDPART_REAL:
					{
						/*
						 * under the root or a real partition, all names are
						 * considered to be a new 'real' partition
						*/
						part_id_t spid;

						/*
						 * in order to prevent the creation of child partitions
						 * that may conflict with 'pid_t' values for processes
						 * associated with the parent, we don't allow a partition
						 * name which consists of only a string of digits
						*/
						if ((mp_parent->type == part_type_SCHEDPART_REAL) &&
							isStringOfDigits(part_name))
						{
							PART_ATTR_UNLOCK(mp);
							PART_ATTR_UNLOCK(mp_parent);
							free(mp);
							return ENOTUNIQ;
						}

						spid = mp_parent->data.spid;
						
						if ((rr = VALIDATE_SP_CFG_CREATION(spid, NULL)) != EOK)
						{
#ifndef NDEBUG
							if (ker_verbose) {
								kprintf("Creation validation err %d, parent (%s), class %p\n", rr, mp_parent->name);
							}
#endif	/* NDEBUG */
							PART_ATTR_UNLOCK(mp);
							PART_ATTR_UNLOCK(mp_parent);
							free(mp);
							return rr;
						}

						if ((mp->data.spid = SCHEDPART_CREATE(spid, part_name, NULL)) == part_id_t_INVALID)
						{
							PART_ATTR_UNLOCK(mp);
							PART_ATTR_UNLOCK(mp_parent);
							free(mp);
							return EIO;
						}
						mp->type = part_type_SCHEDPART_REAL;
						mp->name = strdup(part_name);
						LIST_INIT(mp->children);
						mp->hdr.next = NULL;
						mp->hdr.prev = NULL;
						mp->parent = mp_parent;
						{
							schedpart_t *spart = SCHEDPART_ID_TO_T(mp->data.spid);
							CRASHCHECK(spart == NULL);
							spart->rmgr_attr_p = (void *)mp;	// back pointer to the resource manager structure
							initial_create = spart->create_key;
						}
						break;
					}

					case part_type_ROOT:
					case part_type_GROUP:
					{
						/*
						 * under root or a partition group name, all names are
						 * either additional group names or pseudo partitions.
						 * Pseudo partitions are handled by apsmgr_link() therefore
						 * only a new partition group name will be handled by
						 * this code path
						*/
						mp->type = part_type_GROUP;
						mp->name = strdup(part_name);
						LIST_INIT(mp->children);
						mp->hdr.next = NULL;
						mp->hdr.prev = NULL;
						mp->parent = mp_parent;
						mp->data.spid = part_id_t_INVALID;
						break;
					}
					default:
					{
						PART_ATTR_UNLOCK(mp);
						PART_ATTR_UNLOCK(mp_parent);
						free(mp);
						return EBADF;
					}
				}
//				iofunc_attr_init(&mp->attr, mp_parent->attr.mode, &mp_parent->attr, &ci);
				iofunc_attr_init(&mp->attr, (msg->connect.mode & S_IPERMS) | (mp_parent->attr.mode & S_IFMT), &mp_parent->attr, &ci);
				if (S_ISDIR(mp->attr.mode)) ++mp->attr.nlink;	// iofunc_unlink() requires nlink >=2
				/* insert into sibling list */
				LIST_ADD(mp_parent->children, mp);
				if (S_ISDIR(mp->attr.mode)) ++mp_parent->attr.nlink;
			}

			// all partition related scheduler is currently from system, ocb should not be
			if	((ocb = calloc(1, sizeof(*ocb))) == NULL)
			{
				PART_ATTR_UNLOCK(mp);
				PART_ATTR_UNLOCK(mp_parent);
				return ENOMEM;
			}

			r = iofunc_ocb_attach(ctp, msg, &ocb->ocb, &mp->attr, &apsmgr_io_funcs);
			ocb->create_key = initial_create;

#ifndef NDEBUG
			if ((r != EOK) && (ker_verbose)) kprintf("%d: ocb attach failed, err = %d\n", __LINE__, r);
#endif
			if ( r != EOK )
				free( ocb );
			PART_ATTR_UNLOCK(mp);
			PART_ATTR_UNLOCK(mp_parent);
			return r; 
		}
		default:
			return ENOSYS;
	}
}

/*******************************************************************************
 * redirect_pid_open
 * 
 * redirect the open on a process id that is associated with a partition to the
 * procfs filesystem
 * 
 * Returns: a value that can be returned to the resource manager library
*/
static int redirect_pid_open(resmgr_context_t *ctp, io_open_t *msg, char *name)
{
	unsigned						space = msg->connect.reply_max;
	struct _io_connect_link_reply	*linkp = (void *)msg;
	char 							*path = (void *)(linkp + 1);
	unsigned						eflag = msg->connect.eflag;
	unsigned						ftype = msg->connect.file_type;
	unsigned						len = strlen(name);
	
	_IO_SET_CONNECT_RET(ctp, _IO_CONNECT_RET_LINK);

//#define REDIRECT_PATH	"../../../../proc/"
#define REDIRECT_PATH	"/proc/"
	/* assuming max 10 digit pid */
	if (space < (sizeof(REDIRECT_PATH) + len))
		return ENAMETOOLONG;

	/* use relative path so that we redirect to the same node */
	memcpy(path, REDIRECT_PATH, sizeof(REDIRECT_PATH));
	STRLCPY(&path[sizeof(REDIRECT_PATH)-1], name, space - sizeof(REDIRECT_PATH));
	len += (sizeof(REDIRECT_PATH)-1);	// faster than strlen(path)
	path[len] = '\0';

	memset(linkp, 0x00, sizeof *linkp);
	linkp->file_type = ftype;
	linkp->eflag = eflag;
	linkp->nentries = 0;
	linkp->path_len = len + 1;

	return _RESMGR_PTR(ctp, msg, sizeof *linkp + linkp->path_len);
}



__SRCVERSION("$IQ: apsmgr_open.c,v 1.23 $");

