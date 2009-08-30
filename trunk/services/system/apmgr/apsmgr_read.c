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
 * apsmgr_read
 * 
 * Provide resource manager read() processing for the memory partitioning module
 * 
*/

#include "apsmgr.h"


static int _apsmgr_readdir(PROCESS *prp, apsmgr_attr_t *attr, off_t *offset,
							void *buf, size_t size);
//static bool_t spart_in_list(part_list_t *spart_list, schedpart_t *schedpart);
static bool node_is_in_hierarchy(apsmgr_attr_t *node, part_list_t *spart_list);

/*******************************************************************************
 * apsmgr_read
 * 
 * This routine wraps _apsmgr_read() and provides the resource manager
 * read implementation
 * 
*/
int apsmgr_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	apsmgr_attr_t  *mp = (apsmgr_attr_t *)GET_PART_ATTR(ocb);
	int  r;
	void *reply_msg;

	if (msg->i.type != _IO_READ)
		return EBADF;
	else if ((r = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK)
		return r;
	else if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
		return ENOSYS;
	else if (mp == NULL)
		return ENOENT;
	else
	{
// FIX ME - why can't we use 'msg' as 'reply_msg' ?
		if ((reply_msg = calloc(1, msg->i.nbytes)) == NULL)
			return ENOMEM;
		
		if ((r = PART_ATTR_LOCK(mp)) != EOK)
		{
			free(reply_msg);	// FIX ME - won't be required as per above
			return r;
		}
		if (S_ISDIR(mp->attr.mode))
		{
			if ((r = _apsmgr_readdir(NULL, mp, (off_t *)&ocb->offset, reply_msg, msg->i.nbytes)) >= 0)
			{
				MsgReply(ctp->rcvid, r, reply_msg, r);
				r = EOK;
			}
		}
		else
			r = EBADF;

		PART_ATTR_UNLOCK(mp);
		free(reply_msg);
		return (r == EOK) ? _RESMGR_NOREPLY : -r;
	}
}

/*******************************************************************************
 * spmgr_read
 *
 * Used by the generic partition manager to perform memory partitioning specific
 * reads
*/
int spmgr_read(PROCESS *prp, apsmgr_attr_t *attr, off_t *offset,
							void *buf, size_t *size)
{
	if (attr == NULL)
		return EINVAL;
	else
	{
		int  r;
		
		if ((r = PART_ATTR_LOCK(attr)) != EOK)
			return r;
		r = _apsmgr_readdir(prp, attr, offset, buf, *size);
		PART_ATTR_UNLOCK(attr);

		if (r >= 0)
		{
			*size = r;
			return EOK;
		}
		else
			return -r;
	}
}

/*******************************************************************************
 * _apsmgr_readdir
 * 
 * This routine does the actual work of reading the contents of the entry
 * identified by <attr>. The read is performed starting at the offset pointed
 * to by <offset> the contents are placed into buffer <buf>. The size of <buf>
 * is pointed to by <size>. If <prp> == NULL, no filtering will be done.
 * <offset> is adjusted acordingly.
 * 
 * Returns: the number of bytes placed into <buf> (never more than <size>) or
 * 			a negative errno.
 * 
*/
static int _apsmgr_readdir(PROCESS *prp, apsmgr_attr_t *attr, off_t *offset,
							void *buf, size_t size)
{
	size_t space_left;
	struct dirent *dir;
	unsigned dirent_max;
	part_list_t *spart_list = NULL;

	CRASHCHECK(attr == NULL);
	CRASHCHECK(offset == NULL);
	CRASHCHECK(buf == NULL);
	CRASHCHECK(!S_ISDIR(attr->attr.mode));
	
	dirent_max = LIST_COUNT(attr->children);
	if (attr->type == part_type_SCHEDPART_REAL)
	{
		schedpart_t *spart = SCHEDPART_ID_TO_T(attr->data.spid);
		CRASHCHECK(spart == NULL);
		dirent_max += LIST_COUNT(spart->prp_list);
	}

	if (*offset >= dirent_max)
		return EOK;

	if (prp != NULL)
	{
		int num_parts = SCHEDPART_GETLIST(prp, NULL, 0, schedpart_flags_t_GETLIST_ALL, NULL);
		if ((num_parts > 0) && ((spart_list = alloca(PART_LIST_T_SIZE(num_parts))) != NULL))
		{
#ifndef NDEBUG
			int n = SCHEDPART_GETLIST(prp, spart_list, num_parts, schedpart_flags_t_GETLIST_ALL, NULL);
			CRASHCHECK(n != 0);
#else	/* NDEBUG */
			(void)SCHEDPART_GETLIST(prp, spart_list, num_parts, schedpart_flags_t_GETLIST_ALL, NULL);
#endif	/* NDEBUG */
		}
	}

	dir = (struct dirent *)buf;
	space_left = size;

	if (*offset < LIST_COUNT(attr->children))
	{
		apsmgr_attr_t *sibling = (apsmgr_attr_t *)LIST_FIRST(attr->children);
		off_t  dir_offset = 0;

		/* add all of the child partitions */
		while (sibling != NULL)
		{
			bool exclude = (spart_list != NULL) &&
							 (((sibling->type == part_type_SCHEDPART_REAL) &&
//							 (spart_in_list(spart_list, sibling->spart) == bool_t_FALSE) &&
							 (node_is_in_hierarchy(sibling, spart_list) == bool_t_FALSE))
							 ||
							 ((sibling->type == part_type_GROUP) ||
							 	 (sibling->type == part_type_SCHEDPART_PSEUDO)));
			if (!exclude)
			{					
				if (dir_offset >= *offset)
				{
					if (space_left >= (sizeof(struct dirent) + strlen(sibling->name) + 1))
					{
						dir->d_ino = (int)sibling->data.spid;
						dir->d_offset = (*offset)++;
						STRLCPY(dir->d_name, sibling->name, NAME_MAX + 1);
						dir->d_namelen = strlen(dir->d_name);
						dir->d_reclen = (sizeof(*dir) + dir->d_namelen + 1 + 3) & ~3;

						space_left -= dir->d_reclen;
						dir = (struct dirent *)((unsigned int)dir + (unsigned int)dir->d_reclen);
					}
					else
						break;
				}
				++dir_offset;
			}
			sibling = (apsmgr_attr_t *)LIST_NEXT(sibling);
		}
	}
#ifdef USE_PROC_OBJ_LISTS
	if (prp == NULL)
	{
		if (attr->type == part_type_SCHEDPART_REAL) 
		{
			schedpart_t *spart = SCHEDPART_ID_TO_T(attr->data.spid);
			CRASHCHECK(spart == NULL);
			if (*offset < (LIST_COUNT(spart->prp_list) + LIST_COUNT(attr->children)))
			{
				prp_node_t  *prp_list = (prp_node_t *)LIST_FIRST(spart->prp_list);
				off_t  dir_offset = LIST_COUNT(attr->children);

				while (prp_list != NULL)
				{
					if (dir_offset >= *offset)
					{
						if (space_left >= (sizeof(struct dirent) + 10 + 1))	// UINT_MAX is 10 digits
						{
							dir->d_ino = PID_TO_INO(prp_list->prp->pid, 1);
							dir->d_offset = (*offset)++;
							ultoa(prp_list->prp->pid, dir->d_name, 10);
							dir->d_namelen = strlen(dir->d_name);
							dir->d_reclen = (sizeof(*dir) + dir->d_namelen + 1 + 3) & ~3;

							space_left -= dir->d_reclen;
							dir = (struct dirent *)((unsigned int)dir + (unsigned int)dir->d_reclen);
						}
						else
							break;
					}
					++dir_offset;
					prp_list = (prp_node_t *)LIST_NEXT(prp_list);
				}
			}
		}	
	}
#endif	/* USE_PROC_OBJ_LISTS */

	return size - space_left;
}


/*******************************************************************************
 * node_is_in_hierarchy
 * 
 * Determine whether the apsmgr_attr_t referred to by <node> is in the
 * partition hierarchy of any partition in <spart_list>
 * 
 * Example is if a process is associated with partition sysram/p0/p1/p2 and we
 * are filtering for that process, then as _apsmgr_readdir() is called we
 * will see first sysram. In this case, we cannot reject p0 because it is in the
 * hierarchy of the partition we are ultimately trying to resolve, ie p2.
 * 
 * The most efficient way to do this is to start with the partitions in
 * <spart_list> and follow the hierarchy up (since a partition can have at most
 * 1 parent) checking against node->spart along the way.
 * 
 * Returns: bool_t_TRUE if it is, bool_t_FALSE is it is not
*/
static bool node_is_in_hierarchy(apsmgr_attr_t *node, part_list_t *spart_list)
{
	unsigned i;

	CRASHCHECK(node->type != part_type_SCHEDPART_REAL);

	for (i=0; i<spart_list->num_entries; i++)
	{
		/* does the id match directly? If so we are done */
		if (spart_list->i[i].id == node->data.spid)
			return bool_t_TRUE;
		else
		{
			/* if the <node> in the hierarchy of id ? */
			schedpart_t *spart = SCHEDPART_ID_TO_T(spart_list->i[i].id);
			CRASHCHECK(spart == NULL);
			while ((spart = spart->parent) != NULL)
				if (SCHEDPART_T_TO_ID(spart) == node->data.spid)
					return bool_t_TRUE;
		}
	}
	return bool_t_FALSE;
}


__SRCVERSION("$IQ: apsmgr_read.c,v 1.23 $");

