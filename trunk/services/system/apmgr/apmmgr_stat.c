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
 * apmmgr_stat
 * 
 * Provide resource manager stat() processing for the memory partitioning module
 * 
*/

#include "apmmgr.h"


static int _apmmgr_st_size(apmmgr_attr_t *attr, struct stat *st);


/*******************************************************************************
 * apmmgr_stat
 * 
 * Resource manager interface for stat() processing
 * 
 * Returns: EOK on success, otherwise an errno
*/
int apmmgr_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb)
{
	apmmgr_attr_t  *mp = (apmmgr_attr_t *)GET_PART_ATTR(ocb);
	int r = iofunc_stat(ctp, &mp->attr, &msg->o);
	
	if (r == EOK)
		r = _apmmgr_st_size(mp, &msg->o);

	return (r != EOK) ? r : _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

/*******************************************************************************
 * mpmgr_get_st_size
 *
 * Partition manager interface to obtain the 'st_size' field of a 'struct stat'
 * for entries managed by the memory partitioning resource manager 
 * 
 * Returns: EOK on success, otherwise an errno
*/
int mpmgr_get_st_size(apmmgr_attr_t *attr, struct stat *st)
{
	if (attr == NULL)
		return EINVAL;
	else
		return _apmmgr_st_size(attr, st);
}

/*******************************************************************************
 * _apmmgr_st_size
 * 
 * Do the work of filling in the struct stat structure st_size field.
 * 
 * Note that this routine assumes that iofunc_stat() (or equivalent) has been
 * called to do most of the work of filling in the io_stat_t.stat message. Only
 * the st_size field will be filled in here.
 * 
 * Returns: EOK on success, otherwise an errno
*/
static int _apmmgr_st_size(apmmgr_attr_t *attr, struct stat *st)
{
	int  r = EOK;

	CRASHCHECK(attr == NULL);
	CRASHCHECK(st == NULL);

	if ((r = PART_ATTR_LOCK(attr)) != EOK)
		return r;

	switch (attr->type)
	{
		case part_type_ROOT:
		case part_type_GROUP:
		{
			st->st_size = LIST_COUNT(attr->children);
#if !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
			st->st_size_hi = 0;
#endif
			break;
		}

		case part_type_MEMPART_REAL:
		{
			mempart_info_t  _meminfo, *meminfo = &_meminfo;
			apmmgr_attr_t *parent = attr->parent;
			memclass_info_t  _si, *si = memclass_info(mempart_get_classid(attr->data.mpid), &_si);
			memsize_t smallest_size = (memsize_t)memsize_t_INFINITY;
			memsize_t partition_free;

			/*
			 * go up the hierarchy (if one exists) and find the partition with the
			 * least available memory. This search will include the memory class itself
			*/
			while (parent != NULL)
			{
				memsize_t partition_free_space;
				memsize_t class_free_space;
				memsize_t free_space;

				if (parent->type == part_type_MEMCLASS) {
					break;	// root of the hierarchy
				}
#ifndef NDEBUG
				CRASHCHECK(parent->data.mpid == part_id_t_INVALID);
				CRASHCHECK((meminfo = MEMPART_GETINFO(parent->data.mpid, meminfo)) == NULL);
#else	/* NDEBUG */
				meminfo = MEMPART_GETINFO(parent->data.mpid, meminfo);
#endif	/* NDEBUG */
				
				/* check the available space in the partition against the memory class */
				partition_free_space = meminfo->cur_cfg.attr.size.max - meminfo->cur_size;
				class_free_space = si->size.unreserved.free;
	
				/*
				 * if the partition has a reservation that is not used up, then the unused
				 * reserved amount must be added to the class_free_space
				*/
				if (meminfo->cur_size < meminfo->cur_cfg.attr.size.min)
					class_free_space += (meminfo->cur_cfg.attr.size.min - meminfo->cur_size);

				free_space = min(class_free_space, partition_free_space);

				/* new smallest ? */
				if (free_space < smallest_size) {
					smallest_size = free_space;
				}
				if (meminfo->cur_cfg.policy.alloc != mempart_alloc_policy_t_HIERARCHICAL) {
					break;
				}
				parent = parent->parent;
			}
			if (smallest_size == (memsize_t)memsize_t_INFINITY)
				smallest_size = si->size.unreserved.free;

			meminfo = MEMPART_GETINFO(attr->data.mpid, meminfo);
			CRASHCHECK(meminfo == NULL);

			if (meminfo->cur_size < meminfo->cur_cfg.attr.size.min)
			{
				memsize_t discretionary = meminfo->cur_cfg.attr.size.max - meminfo->cur_cfg.attr.size.min;
				partition_free = min(discretionary, smallest_size);
				partition_free += (meminfo->cur_cfg.attr.size.min - meminfo->cur_size);
				
			}
			else
			{
				memsize_t discretionary = meminfo->cur_cfg.attr.size.max - meminfo->cur_size;
				partition_free = min(discretionary, smallest_size);
			}

			st->st_size = (off_t)partition_free;
#if !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
			/* will be 0 if memsize_t is 32 bit */
			st->st_size_hi = (off_t)((_MEMSIZE_T_)partition_free >> 32);
#endif
			break;
		}

		case part_type_MEMCLASS:
		{
			memclass_entry_t  *mclass = attr->data.mclass_entry;
			if (mclass == NULL)
			{
				r = ENOENT;
			}
			else
			{
				memclass_info_t  _si, *si = memclass_info(mclass->data.info.id, &_si);
				CRASHCHECK(si == NULL);
				st->st_size = (off_t)(si->size.reserved.free + si->size.unreserved.free);
#if !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
				st->st_size_hi = (off_t)((si->size.reserved.free + si->size.unreserved.free) >> 32);
#endif
			}
			break;
		}

		case part_type_MEMPART_PSEUDO:
		{
#ifndef NDEBUG
			CRASHCHECK(attr->data.symlink == NULL);
#endif	/* NDEBUG */
			st->st_size = strlen(attr->data.symlink);
#if !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
			st->st_size_hi = 0;
#endif
			break;
		}
		
		default:
			r = ENOSYS;
	}
	PART_ATTR_UNLOCK(attr);
	return r;
}


__SRCVERSION("$IQ: apmmgr_stat.c,v 1.23 $");

