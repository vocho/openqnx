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
 * apmmgr_read
 * 
 * Provide resource manager read() processing for the memory partitioning module
 * 
*/

#include "apmmgr.h"


static int _apmmgr_readdir(PROCESS *prp, apmmgr_attr_t *attr, off_t *offset,
							void *buf, size_t size);
//static bool_t mpart_in_list(part_list_t *mpart_list, mempart_t *mempart);
static bool node_is_in_hierarchy(apmmgr_attr_t *node, part_list_t *mpart_list);

/*******************************************************************************
 * apmmgr_read
 * 
 * This routine wraps _apmmgr_read() and provides the resource manager
 * read implementation
 * 
*/
int apmmgr_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	apmmgr_attr_t  *mp = (apmmgr_attr_t *)GET_PART_ATTR(ocb);
	int  r;
	void *reply_msg;

	if (msg->i.type != _IO_READ) {
		return EBADF;
	} else if ((r = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK) {
		return r;
	} else if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
		return ENOSYS;
	} else if (mp == NULL) {
		return ENOENT;
	}
	else
	{
// FIX ME - why can't we use 'msg' as 'reply_msg' ?
		if ((reply_msg = calloc(1, msg->i.nbytes)) == NULL) {
			return ENOMEM;
		}
		
		if ((r = PART_ATTR_LOCK(mp)) != EOK)
		{
			free(reply_msg);	// FIX ME - won't be required as per above
			return r;
		}
		if (S_ISDIR(mp->attr.mode))
		{
			off_t  offset = (off_t)ocb->offset;
			if ((r = _apmmgr_readdir(NULL, mp, &offset, reply_msg, msg->i.nbytes)) >= 0)
			{
				ocb->offset = offset;
#if _IOFUNC_OFFSET_BITS - 0 == 32
				ocb->offset_hi = 0;
#endif
				MsgReply(ctp->rcvid, r, reply_msg, r);
				r = EOK;
			}
		}
		else
		{
			r = EBADF;
		}

		PART_ATTR_UNLOCK(mp);
		free(reply_msg);
		return (r == EOK) ? _RESMGR_NOREPLY : -r;
	}
}

/*******************************************************************************
 * mpmgr_read
 *
 * Used by the generic partition manager to perform memory partitioning specific
 * reads
*/
int mpmgr_read(PROCESS *prp, apmmgr_attr_t *attr, off_t *offset,
							void *buf, size_t *size)
{
	if (attr == NULL) {
		return EINVAL;
	}
	else
	{
		int  r;
		
		if ((r = PART_ATTR_LOCK(attr)) != EOK) {
			return r;
		}
		r = _apmmgr_readdir(prp, attr, offset, buf, *size);
		PART_ATTR_UNLOCK(attr);

		if (r >= 0) {
			*size = r;
			return EOK;
		} else {
			return -r;
		}
	}
}

/*******************************************************************************
 * _apmmgr_readdir
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
static int _apmmgr_readdir(PROCESS *prp, apmmgr_attr_t *attr, off_t *offset,
							void *buf, size_t size)
{
	size_t space_left;
	struct dirent *dir;
	unsigned dirent_max;
	part_list_t *mpart_list = NULL;

	CRASHCHECK(attr == NULL);
	CRASHCHECK(offset == NULL);
	CRASHCHECK(buf == NULL);
	CRASHCHECK(!S_ISDIR(attr->attr.mode));
	
	dirent_max = LIST_COUNT(attr->children);
	if (attr->type == part_type_MEMPART_REAL)
	{
		mempart_t *mpart = MEMPART_ID_TO_T(attr->data.mpid);
		CRASHCHECK(mpart == NULL);
		dirent_max += LIST_COUNT(mpart->prp_list);
	}

	if (*offset >= dirent_max) {
		return EOK;
	}

	if (prp != NULL)
	{
		int num_parts = MEMPART_GETLIST(prp, NULL, 0, mempart_flags_t_GETLIST_ALL, NULL);
		if ((num_parts > 0) && ((mpart_list = alloca(PART_LIST_T_SIZE(num_parts))) != NULL))
		{
#ifndef NDEBUG
			int n = MEMPART_GETLIST(prp, mpart_list, num_parts, mempart_flags_t_GETLIST_ALL, NULL);
			CRASHCHECK(n != 0);
#else	/* NDEBUG */
			(void)MEMPART_GETLIST(prp, mpart_list, num_parts, mempart_flags_t_GETLIST_ALL, NULL);
#endif	/* NDEBUG */
		}
	}

	dir = (struct dirent *)buf;
	space_left = size;

	if (*offset < LIST_COUNT(attr->children))
	{
		apmmgr_attr_t *sibling = (apmmgr_attr_t *)LIST_FIRST(attr->children);
		off_t  dir_offset = 0;

		/* add all of the child partitions */
		while (sibling != NULL)
		{
			bool exclude = (mpart_list != NULL) &&
							 (((sibling->type == part_type_MEMPART_REAL) &&
//							 (mpart_in_list(mpart_list, sibling->mpart) == bool_t_FALSE) &&
							 (node_is_in_hierarchy(sibling, mpart_list) == bool_t_FALSE))
							 ||
							 ((sibling->type == part_type_GROUP) ||
							 	 (sibling->type == part_type_MEMPART_PSEUDO)));
			if (!exclude)
			{					
				if (dir_offset >= *offset)
				{
					if (space_left >= (sizeof(struct dirent) + strlen(sibling->name) + 1))
					{
						dir->d_ino = (int)sibling->data.mpid;
						dir->d_offset = (*offset)++;
						STRLCPY(dir->d_name, sibling->name, NAME_MAX + 1);
						dir->d_namelen = strlen(dir->d_name);
						dir->d_reclen = (sizeof(*dir) + dir->d_namelen + 1 + 3) & ~3;

						space_left -= dir->d_reclen;
						dir = (struct dirent *)((unsigned int)dir + (unsigned int)dir->d_reclen);
					}
					else
					{
						break;
					}
				}
				++dir_offset;
			}
			sibling = (apmmgr_attr_t *)LIST_NEXT(sibling);
		}
	}
#ifdef USE_PROC_OBJ_LISTS
	if (prp == NULL)
	{
		if (attr->type == part_type_MEMPART_REAL) 
		{
			mempart_t *mpart = MEMPART_ID_TO_T(attr->data.mpid);
			CRASHCHECK(mpart == NULL);
			if (*offset < (LIST_COUNT(mpart->prp_list) + LIST_COUNT(attr->children)))
			{
				prp_node_t  *prp_list = (prp_node_t *)LIST_FIRST(mpart->prp_list);
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
						{
							break;
						}
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

#if 0	// FIX ME - no longer required, here for reference
/*******************************************************************************
 * mpart_in_list
 * 
 * Determine whether <mempart> is contained in <mpart_list>
 * 
 * Returns: bool_t_TRUE if it is, bool_t_FALSE is it is not
*/
static bool_t mpart_in_list(part_list_t *mpart_list, mempart_t *mempart)
{
	part_id_t  id_to_find = MEMPART_T_TO_ID(mempart);
	unsigned i;

	for (i=0; i<mpart_list->num_entries; i++) {
		if (mpart_list->i[i].t.id == id_to_find) {
			return bool_t_TRUE;
		}
	}
	return bool_t_FALSE;
}
#endif	/* 0 */

/*******************************************************************************
 * node_is_in_hierarchy
 * 
 * Determine whether the apmmgr_attr_t referred to by <node> is in the
 * partition hierarchy of any partition in <mpart_list>
 * 
 * Example is if a process is associated with partition sysram/p0/p1/p2 and we
 * are filtering for that process, then as _apmmgr_readdir() is called we
 * will see first sysram. In this case, we cannot reject p0 because it is in the
 * hierarchy of the partition we are ultimately trying to resolve, ie p2.
 * 
 * The most efficient way to do this is to start with the partitions in
 * <mpart_list> and follow the hierarchy up (since a partition can have at most
 * 1 parent) checking against node->mpart along the way.
 * 
 * Returns: bool_t_TRUE if it is, bool_t_FALSE is it is not
*/
static bool node_is_in_hierarchy(apmmgr_attr_t *node, part_list_t *mpart_list)
{
	unsigned i;

	CRASHCHECK(node->type != part_type_MEMPART_REAL);

	for (i=0; i<mpart_list->num_entries; i++)
	{
		/* does the id match directly? If so we are done */
		if (mpart_list->i[i].id == node->data.mpid) {
			return bool_t_TRUE;
		}
		else
		{
			/* if the <node> in the hierarchy of id ? */
			mempart_t *mpart = MEMPART_ID_TO_T(mpart_list->i[i].id);
			CRASHCHECK(mpart == NULL);
			while ((mpart = mpart->parent) != NULL) {
				if (MEMPART_T_TO_ID(mpart) == node->data.mpid) {
					return bool_t_TRUE;
				}
			}
		}
	}
	return bool_t_FALSE;
}


__SRCVERSION("$IQ: apmmgr_read.c,v 1.23 $");

