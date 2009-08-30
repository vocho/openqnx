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
 * apsmgr_stat
 * 
 * Provide resource manager stat() processing for the scheduler partitioning module
 * 
*/

#include "apsmgr.h"


static int _apsmgr_st_size(apsmgr_attr_t *attr, struct stat *st);


/*******************************************************************************
 * apsmgr_stat
 * 
 * Resource manager interface for stat() processing
 * 
 * Returns: EOK on success, otherwise an errno
*/
int apsmgr_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb)
{
	apsmgr_attr_t  *mp = (apsmgr_attr_t *)GET_PART_ATTR(ocb);
	int r = iofunc_stat(ctp, &mp->attr, &msg->o);
	
	if (r == EOK)
		r = _apsmgr_st_size(mp, &msg->o);

	return (r != EOK) ? r : _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

/*******************************************************************************
 * spmgr_get_st_size
 *
 * Partition manager interface to obtain the 'st_size' field of a 'struct stat'
 * for entries managed by the scheduler partitioning resource manager 
 * 
 * Returns: EOK on success, otherwise an errno
*/
int spmgr_get_st_size(apsmgr_attr_t *attr, struct stat *st)
{
	if (attr == NULL)
		return EINVAL;
	else
		return _apsmgr_st_size(attr, st);
}

/*******************************************************************************
 * _apsmgr_st_size
 * 
 * Do the work of filling in the struct stat structure st_size field.
 * 
 * Note that this routine assumes that iofunc_stat() (or equivalent) has been
 * called to do most of the work of filling in the io_stat_t.stat message. Only
 * the st_size field will be filled in here.
 * 
 * Returns: EOK on success, otherwise an errno
*/
static int _apsmgr_st_size(apsmgr_attr_t *attr, struct stat *st)
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
			break;
		}

		case part_type_SCHEDPART_REAL:
		{
			// FIX ME - get budget free
			st->st_size = 0;
			break;
		}

		case part_type_SCHEDPART_PSEUDO:
		{
#ifndef NDEBUG
			CRASHCHECK(attr->data.symlink == NULL);
#endif	/* NDEBUG */
			st->st_size = strlen(attr->data.symlink);
			break;
		}
		
		default:
			r = ENOSYS;
	}
	PART_ATTR_UNLOCK(attr);
	return r;
}


__SRCVERSION("$IQ: apsmgr_stat.c,v 1.23 $");

