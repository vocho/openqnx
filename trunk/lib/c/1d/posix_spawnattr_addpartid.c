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

#include <unistd.h>		// determines whether _POSIX_SPAWN is defined or not
#ifdef _POSIX_SPAWN

#include "posix_spawnattr.h"


/*
 * pathtype_t
 * 
 * This type is used by the posix_spawnattr_addpartition() function when
 * parsing the partition path name. See pathtype()
*/
typedef enum
{
	pathtype_t_MEM = 1,
	pathtype_t_SCHED,
	pathtype_t_NAME,
	pathtype_t_PROC_PID,
	pathtype_t_UNKNOWN

} pathtype_t;


static part_id_t get_part_id(const char *part_name);
static int pathtype(const char *fullpath, pathtype_t *type, char **name);


/*******************************************************************************
 * addpartid
 * 
 * Specify a resource partition that the spawned process should be associated
 * with by its partition identifier. If an ID is specified for a non-existent
 * partition, it will not be known until the posix_spawn() call is made.
 * 
 * Note that 2 additional forms for this call (addmempartid and addschedpartid)
 * are implemented as macros in spawn.h
 * 
 * Note that partition association only takes effect if the corresponding
 * POSIX_SPAWN_SETMPART or POSIX_SPAWN_SETSPART flag is set. Setting one of
 * these flags without providing a partition of that type may give unintended
 * results. Please consult the Adaptive Partitioning Guide for the inheritance
 * rules as they apply to partition association.
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL for any invalid parameter
 * 		ENOMEM if the partition id could not be added to attributes object
 * 
 *		Note that if this function fails and the <partition_name> provided is a
 *		group name, then it is unspecified how many of the pseudo partitions
 * 		within the group name are added to the attributes object pointed to by
 * 		<attrp>. It is suggested that the attributes object be destroyed and a
 * 		new attributes object be initialized in this situation
 * 
 * 		Also note that some partition association errors may not be reported
 * 		until the posix_spawn() call is made.
 * 
 * Add the partition identified by <part_id> 'posix_spawnattr_t' object <attrp>.
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL for any invalid parameter
 * 		ENOMEM if the partition id could not be added to the attributes object
*/
int posix_spawnattr_addpartid(posix_spawnattr_t *_Restrict attrp, part_id_t part_id, part_dcmd_flags_t part_flags)
{
	switch (PART_TYPE(part_id))
	{
		case parttype_MEM:
		case parttype_SCHED: break;
		default: return EINVAL;
	}
	if ((attrp == NULL) || (part_id == part_id_t_INVALID)) {
		return EINVAL;
	} else {
		/* need to realloc the attribute structure. Let's find out by how much */
		_posix_spawnattr_t *_attrp = GET_ATTRP(attrp);
		size_t attr_t_size = sizeof(*_attrp) - sizeof(_attrp->partition);
		size_t partition_t_size = 0;
		_posix_spawnattr_partition_t *partition_attr;
		unsigned init_mem = 0;

		if ((_attrp == NULL) || (_attrp->partition == NULL)) {
			partition_t_size = sizeof(*_attrp->partition);
			/* the first time that partition information is added, a realloc()
			 * will be done. This could leave garbage at _attrp->partition and
			 * lead to invalid partition_attr values and the placement of the
			 * new entry at the wrong location. After the first entry is added,
			 * values like partition_attr->mem.list.num_entries will remain
			 * valid and so don't have to worry */
			init_mem = 1;
		} else {
			partition_attr = (_posix_spawnattr_partition_t *)&_attrp->partition;
			/*
			 * have some partition id's already. New size is existing size
			 * plus space for one more 'id'
			*/
			partition_t_size = (partition_attr->size + sizeof(partition_attr->part.i));
		}

		if ((_attrp = _posix_spawnattr_t_realloc(_attrp, attr_t_size + partition_t_size)) == NULL) {
			return ENOMEM;
		}
		/* reset 'partition_attr' to the location of the possibly relocated '_attrp' */
		partition_attr = (_posix_spawnattr_partition_t *)&_attrp->partition;
		/* initialize a first time entry as required */
		if (init_mem) memset(partition_attr, 0, partition_t_size);
		/* update the size of the _posix_spawnattr_partition_t structure that has been allocated */
		partition_attr->size = partition_t_size;
		/* update the partition structure with the new entry */
		partition_attr->part.i[partition_attr->part.num_entries].id = part_id;
		partition_attr->part.i[partition_attr->part.num_entries].flags = part_flags;
		++partition_attr->part.num_entries;
	
		/* reset posix_spawnattr_t object pointer */
		SET_ATTRP(attrp, _attrp);
		return EOK;
	}
}

/*******************************************************************************
 * addpartition
 * 
 * Specify a resource partition that the spawned process should be associated
 * with. This is the most convenient form of adding partition associations as it
 * accepts a partition name rather than an ID. Partition names can either be
 * group names, pseudo partitions or real partitions and will always be resolved
 * to a correct 'real' partition otherwise an error will be returned.
 * 
 * Note that if <partition_path> does not refer to a real partition then the
 * <part_flags> argument will apply to all resolved 'real' partitions refered to
 * by the group name or pseudo partition. This may result in an error if the flag
 * does not apply to the partition type (as reported by the PART_TYPE() macro
 * defined in part.h).
 * 
 * Note that partition association only takes effect if the corresponding
 * POSIX_SPAWN_SETMPART or POSIX_SPAWN_SETSPART flag is set. Setting one of
 * these flags without providing a partition of that type may give unintended
 * results. Please consult the Adaptive Partitioning Guide for the inheritance
 * rules as they apply to partition association.
 * 
 * Returns:
 * 		EOK on success
 * 		EINVAL for any invalid parameter
 * 		ENOMEM if the partition id could not be added to attributes object
 * 
 *		Note that if this function fails and the <partition_name> provided is a
 *		group name, then it is unspecified how many of the pseudo partitions
 * 		within the group name are added to the attributes object pointed to by
 * 		<attrp>. It is suggested that the attributes object be destroyed and a
 * 		new attributes object be initialized in this situation
 * 
 * 		Also note that some partition association errors may not be reported
 * 		until the posix_spawn() call is made.
*/
int posix_spawnattr_addpartition(posix_spawnattr_t *_Restrict attrp, const char *partition_path, part_dcmd_flags_t part_flags)
{
	char *dont_care;
	pathtype_t  type;
	int r;

	if (!valid_attrp(attrp)) return EINVAL;
	if ((partition_path == NULL) || (*partition_path == '\0')) return EINVAL;
	if ((r = pathtype((char *)partition_path, &type, &dont_care)) != EOK) return r;
	
	switch(type)
	{
		case pathtype_t_MEM:	// a memory partition
		case pathtype_t_SCHED:	// a scheduling partition
		{
			part_id_t part_id = get_part_id(partition_path);
			if (part_id == part_id_t_INVALID) return EINVAL;
			return posix_spawnattr_addpartid(attrp, part_id, part_flags);
		}

		case pathtype_t_NAME:	// a pseudo partition or group name
		{
			struct stat  s;

			if ((stat(partition_path, &s) != 0) || ((s.st_mode & (S_IFLNK|S_IFDIR)) == 0)) {
				return EINVAL;
			}

			switch (s.st_mode & S_IFMT)
			{
				case S_IFLNK:	// pseudo partition
				{
					char r_path[PATH_MAX+1];
					if (readlink(partition_path, r_path, sizeof(r_path)-1) == -1) {
						return errno;
					} else {
						return posix_spawnattr_addpartition(attrp, r_path, part_flags);
					}
				}
				case S_IFDIR:	// group name
				{
					DIR  *dirp;
					struct dirent dent[sizeof(struct dirent) + NAME_MAX + 1];
					struct dirent* direntp;
					r = EINVAL;

					if ((dirp = opendir(partition_path)) == NULL) return errno;

					while (((r = readdir_r(dirp, dent, &direntp)) == EOK) && (direntp != NULL))
					{
						char r_path[PATH_MAX+1];
						snprintf(r_path, sizeof(r_path), "%s/%s", partition_path, direntp->d_name);
						if (readlink(r_path, r_path, sizeof(r_path)-1) > 0) {
							if ((r = posix_spawnattr_addpartition(attrp, r_path, part_flags)) != EOK) {
								break;
							}
							r = EOK;	// at least 1 successfully registered name
						}
					}
					closedir(dirp);
					return r;
				}
				default:
					return EINVAL;
			}
		}
		default:
			return EINVAL;
	}
}

/*
 * =============================================================================
 * 
 * 							Internal Support Routines
 * 
 * =============================================================================
*/

/*******************************************************************************
 * get_part_id
 * 
 * Return the partition identifier for the named partition
 * Returns the part_id_t or part_id_t_INVALID if unsuccessful.
*/
static part_id_t get_part_id(const char *part_name)
{
	int  fd, r = EOK;
	int  status;
	part_id_t  part_id;
	char *path_name = (char *)part_name;

//printf("open '%s'\n", path_name);
	if (((fd = open(path_name, O_RDONLY)) == -1) ||
		((r = devctl(fd, PART_GET_ID, &part_id, sizeof(part_id), &status)) != EOK))
	{
		r = (r == EOK) ? errno : r;
		part_id = part_id_t_INVALID;
	}

	close(fd);
	errno = r;
	return part_id;
}


/*******************************************************************************
 * pathtype
 * 
 * Given a full partition path, determine the path type (memory/sched/etc) and
 * return a pointer to the name suffix (ie. the portion of the name after
 * "/partition/mem/" or "/partition/sched/") if possible
*/
static int pathtype(const char *fullpath, pathtype_t *type, char **name)
{
	int r = EINVAL;

	if ((*name = (char *)strstr(fullpath, "/partition")) != NULL)
	{
		part_id_t  id = get_part_id(fullpath);
		
		switch(PART_TYPE(id))
		{
			case parttype_MEM:
				*type = pathtype_t_MEM;
				*name += (sizeof("/partition/mem") - 1);
				break;
			case parttype_SCHED:
				*type = pathtype_t_SCHED;
				*name += (sizeof("/partition/sched") - 1);
				break;
			default:
				*type = pathtype_t_NAME;
				*name += (sizeof("/partition") - 1);
		}
		if (**name == '/') ++(*name);
		r = EOK;
	}
	else if ((*name =(char *)strstr(fullpath, "/proc")) != NULL)
	{
		*type = pathtype_t_PROC_PID;
		if (**name == '/') ++(*name);
		r = EOK;
	}
	else
	{
		*type = pathtype_t_UNKNOWN;
		*name = NULL;
	}
	return r;
}

#endif	/* _POSIX_SPAWN */

