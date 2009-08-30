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
 * 
 * Security wise, the caller must have write permissions on the partition name
 * in order to rename it. No other security policies apply to renaming
 *
 */

/*==============================================================================
 * 
 * apsmgr_rename
 * 
 * Provide resource manager handling for the scheduler partitioning module.
 * This entry point allows the use of 'mv' for the renaming of an existing
 * real scheduler partition or group name.
 * 
 * Note that we do not support moving partition names, that concept does not
 * exist, this is not a filesystem. The only thing that can be done is to rename
 * an existing partition to a new name. The entire path prefix must be identical
*/

#include "apsmgr.h"

static pthread_mutex_t schedpart_rename_lock = PTHREAD_MUTEX_INITIALIZER;

/*******************************************************************************
 * apsmgr_rename
 * 
*/
int apsmgr_rename(resmgr_context_t *ctp, io_rename_t *msg, RESMGR_HANDLE_T *handle, io_rename_extra_t *extra)
{
	switch (msg->connect.subtype)
	{
		case _IO_CONNECT_RENAME:
		{
			struct _client_info  ci;
			int  r;
			apsmgr_attr_t  *mp;
			char *new_name = msg->connect.path;
			char *cur_name = extra->path;
			char *chk_name;
			char *nn_prefix_end;
			char *cn_prefix_end;

			CRASHCHECK(nametoolong(cur_name, PATH_MAX, (void *)apsmgr_devno));
			CRASHCHECK(nametoolong(cur_name, NAME_MAX, (void *)apsmgr_devno));

			if ((nametoolong(new_name, PATH_MAX, (void *)apsmgr_devno)) ||
				(nametoolong(new_name, NAME_MAX, (void *)apsmgr_devno))) {
				return ENAMETOOLONG;
			}

			nn_prefix_end = strrchr(new_name, '/');
			cn_prefix_end = strrchr(cur_name, '/');

			/* make sure that name prefixes match */
			if (!((nn_prefix_end == NULL) && (cn_prefix_end == NULL)))
			{
				/* non NULL prefixes, do some more checking */
				if (((nn_prefix_end == NULL) && (cn_prefix_end != NULL)) ||
					((nn_prefix_end != NULL) && (cn_prefix_end == NULL))) {
					return EINVAL;
				}
				CRASHCHECK(nn_prefix_end == NULL);
				CRASHCHECK(cn_prefix_end == NULL);
				
				/* prefix length check */ 
				if ((nn_prefix_end - new_name) != (cn_prefix_end - cur_name)) {
					return EINVAL;
				}
				if (memcmp(new_name, cur_name, nn_prefix_end - new_name) != 0) {
					return EINVAL;
				}
			}

			/* current partition must exist */
			if (memcmp(chk_name = cur_name, "/partition/sched/", sizeof("/partition/sched/")-1) == 0) {
				chk_name += sizeof("/partition/sched/") - 1;
			}
			if ((mp = spath_find((apsmgr_attr_t *)LIST_FIRST(root_spart->children), chk_name, NULL)) == NULL) {
				return ENOENT;
			}

			/*
			 * renaming to same name is effectively a no-op but we want to catch
			 * the situation of "mv /foo/bar/part /foo/bar/". In this case the
			 * new and current names will match but if we don't return EOK, then
			 * 'mv' will attempt an unlink and we don't want that.
			 * Note that this IS NOT the same situation as
			 * "mv /foo/bar/part /foo/bar/part" which would want to create
			 * "/foo/bar/part/part". This is illegal and will cause EINVAL to be
			 * returned
			*/
			if (nn_prefix_end != NULL)
			{
				/* we know the prefixes are the same so only compare the base names */
				unsigned len = max(strlen(nn_prefix_end), strlen(cn_prefix_end));
				if (memcmp(nn_prefix_end, cn_prefix_end, len) == 0) {
					return EOK;
				}
			}
			else
			{
				unsigned len = max(strlen(new_name), strlen(cur_name));
				if (memcmp(new_name, cur_name, len) == 0) {
					return EOK;
				}
			}

			/* new partition name partition must NOT exist */
			if (memcmp(chk_name = new_name, "/partition/sched/", sizeof("/partition/sched/")-1) == 0) {
				chk_name += sizeof("/partition/sched/") - 1;
			}
			if (spath_find((apsmgr_attr_t *)LIST_FIRST(root_spart->children), chk_name, NULL) != NULL) {
				return EEXIST;
			}

			CRASHCHECK(mp == NULL);

			if ((r = PART_ATTR_LOCK(mp)) != EOK) {
				return r;
			}

			/* only deal with real/pseudo partitions and group names */			
			if ((mp->type != part_type_SCHEDPART_REAL) &&
				(mp->type != part_type_SCHEDPART_PSEUDO) &&
				(mp->type != part_type_GROUP))
			{
				PART_ATTR_UNLOCK(mp);
				return EBADF;
			}

			/* check access permissions. Caller needs write permissions */
			if (((r = iofunc_client_info(ctp, msg->connect.ioflag, &ci)) != EOK) ||
				((r = check_access_perms(ctp, (apxmgr_attr_t *)mp, S_IWUSR|S_IWGRP|S_IWOTH, &ci, bool_t_FALSE)) != EOK))
			{
				PART_ATTR_UNLOCK(mp);
				return r;
			}

			/* ensure that the 2 different partitions cannot be given the same name */
			if ((r = pthread_mutex_lock(&schedpart_rename_lock)) == EOK)
			{
				/* new partition name partition must STILL NOT exist */
				if (memcmp(chk_name = new_name, "/partition/sched/", sizeof("/partition/sched/")-1) == 0) {
					chk_name += sizeof("/partition/sched/") - 1;
				}
				if (spath_find((apsmgr_attr_t *)LIST_FIRST(root_spart->children), chk_name, NULL) != NULL) {
					r = EEXIST;
				}
				else
				{
					free((char *)mp->name);
					if (nn_prefix_end != NULL) {
						new_name = nn_prefix_end + 1;	// skip over '/'
					}
					mp->name = strdup(new_name);
					r = EOK;
				}
				pthread_mutex_unlock(&schedpart_rename_lock);
			}
			PART_ATTR_UNLOCK(mp);
			return r;
		}

		default:
			break;
	}
	return ENOSYS;
}

__SRCVERSION("$IQ: apsmgr_mknod.c,v 1.23 $");

