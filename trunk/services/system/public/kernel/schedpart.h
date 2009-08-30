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

#ifndef _SCHEDPART_H_
#define _SCHEDPART_H_

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include <stdint.h>
#include <limits.h>
#include <signal.h>
#include <sys/part.h>
#include <sys/sched_aps.h>

/*
 * schedpart_attr_t
 * 
*/
typedef struct schedpart_attr_s
{
	_Uint16t	budget_percent;
	_Int16t		critical_budget_ms;	/*milliseconds. optional: set to -1 or 0 to skip */

} schedpart_attr_t;


/*
 * schedpart_policy_t
 * 
 * This structure defines the set of policies by which a partition operates.
 * The boolean policies are write once in that once set TRUE, they can never be
 * changed. The purpose of this is to ensure a partition topology is not
 * compromised. These boolean policies are independent of each other and of
 * posix name space permissions (however write persmissions are required to
 * change a policy) and operate as follows ...
 * 		terminal - if TRUE, no child partitions may be created
 * 		config_lock - if TRUE, the partition attributes and allocation policy
 * 					  cannot be changed
 * 		permanent - if TRUE, the partition cannot be destroyed
*/
typedef struct schedpart_policy_s
{
	ap_bool_t				terminal;		// child partitions are permitted (T/F) ?
	ap_bool_t				config_lock;	// configuration is locked (T/F) ?
	ap_bool_t				permanent;		// partition is permanent (T/F) ?

} schedpart_policy_t;

#define SCHEDPART_DFLT_POLICY_INITIALIZER \
		{ \
			STRUCT_FLD(terminal) bool_t_FALSE, \
			STRUCT_FLD(config_lock) bool_t_FALSE, \
			STRUCT_FLD(permanent) bool_t_TRUE, /* scheduler partitions are inherently permanent for now */\
		}

/*
 * schedpart_cfg_t
 * 
 * This type defines the configuration attributes and policies for the partition.
 * This type is used both to return the configuration state of the partition as
 * well as to allow the configuration to be changed (see schedpart_cfgchg_t)
*/
typedef struct schedpart_cfg_s
{
	schedpart_policy_t	policy;
	schedpart_attr_t	attr;
	_Uint32t			pinfo_flags;
} schedpart_cfg_t;

/*
 * schedpart_cfgchg_t
 * 
 * This type is used to change the configuration attributes and policies for the
 * partition. The 'valid' field allows the caller to specify which attributes
 * and/or polices should be changed without changing the other attributes and/or
 * policies and therefore does not require knowledge of the partition's current
 * configuration
*/
typedef struct schedpart_cfgchg_s
{
	_Uint32t		valid;
	schedpart_cfg_t	val;
} schedpart_cfgchg_t;

/*
 * schedpart_info_t
 *  
*/
typedef struct schedpart_info_s
{
	sched_aps_partition_info	info;	// FIX ME - currently just same as defined in sched_aps.h even though redundant
	sched_aps_partition_stats	stats;
	schedpart_cfg_t				cre_cfg;// configuration at partition creation
	schedpart_cfg_t				cur_cfg;// configuration at partition creation
	part_id_t					id;		// partition identifier
	_Uint32t					num_children;	// number of (immediate) children this partition has
												// Don't need 32 bits but want to use atomic_() operations
	_Uint32t					reserved[2];
} schedpart_info_t;


/*
 * schedpart_dcmd_flags_t
*/
typedef part_dcmd_flags_t	schedpart_dcmd_flags_t;
#if 0	// no scheduler specific partitioning flags yet
typedef enum
{
	/* scheduler partitioning flags are defined in byte 2 (see part.h) */

} schedpart_dcmd_flags_t_val;
#endif


/*
 * Scheduler Partition specific devctl()'s in addition to those defined in
 * part.h
 * 
 * SCHEDPART_GET_INFO
 * 	return scheduler partition information (schedpart_info_t) for the partition
 * 	identified by 'fd'.
 *  On success, EOK will be returned by devctl() and the caller provided
 * 	'schedpart_info_t' structure will contain the applicable information.
 * 	If any error occurs, the caller provided schedpart_info_t structure should
 * 	be considered to contain garbage.
 * 	The following errnos and their interpretation may be returned
 * 		ENOSYS - the scheduler partitioning module is not installed
 * 		EOVERFLOW - the caller has provided a buffer which is inconsistent with
 * 					the specified number of events being registered
 * 		EBADF - the SCHEDPART_GET_INFO devctl() has been issued on a name which
 * 				does not represent a scheduler partition
 *		EIO - internal I/O error. There is no information available or the
 * 				request to retrieve the information failed.
 * 
 * SCHEDPART_CFG_CHG
 * 	change the current configuration attributes for the partition identified by
 *  'fd' to those specified in the provided 'schedpart_cfgchg_t'. The success of
 *  this operation depends on the current partition configuration, current
 *  parent partition configuration, allocation use and the amount of unallocated
 *  unreserved scheduler of the class the partition represents. The reconfiguration
 *  will fail if ...
 * 		- size.max attribute is specified to be less than the current allocation
 * 		  size (ie. cur_size)
 * 		- the specified size.min or size.max is greater than the size.max of the
 * 		  parent partition (FIX ME - Peter V change)
 * 		- a size.min (ie reservation) is specified which cannot be satisfied
 * 		- the attribute values are invalid (ex. size.max is less than size.min)
 *  On success, EOK will be returned by devctl() and the caller can assume the
 * 	partition configuration has been successfully changed to that those provided.
 * 	If any error occurs, the caller can assume that no partition changes were
 * 	made.
 * 	The following errnos and their interpretation may be returned
 * 		ENOSYS - the scheduler partitioning module is not installed
 * 		EOVERFLOW - the caller has provided a buffer which is inconsistent with
 * 					the specified number of events being registered
 * 		EBADF - the SCHEDPART_CFG_CHG devctl() has been issued on a name which
 * 				does not represent a scheduler partition
 * 
*/
#define SCHEDPART_CFG_CHG			__DIOT(_DCMD_PARTITION, _DCMD_SCHEDPART_OFFSET + 0, schedpart_cfgchg_t)
#define SCHEDPART_GET_INFO			__DIOTF(_DCMD_PARTITION, _DCMD_SCHEDPART_OFFSET + 1, schedpart_info_t)


#endif	/* _SCHEDPART_H_ */
