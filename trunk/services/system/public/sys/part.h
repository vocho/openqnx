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

/*
 * =============================================================================
 * 
 * 					Common header for partitioning module
 * 								public interfaces
 * =============================================================================
*/

#ifndef _PART_H_
#define _PART_H_

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include <stdint.h>
#include <limits.h>
#include <signal.h>
#include <devctl.h>

/*
 * ap_bool_t
 * 
 * The ap_bool_t type is a logical type with the following additional properties
 * 
 * 		- its size is specified
 * 		- it has "write once" behaviour
 * 
 * The write once behaviour is defined by the variable of this type. For example,
 * the variable may be defined such that once set TRUE, it cannot be changed to
 * FALSE or vice versa.
 * This type is always represented by the value bool_t_FALSE or bool_t_TRUE.
 * See mempart.h for specific examples of where this type is used.
*/
typedef _Uint32t	ap_bool_t;
typedef enum {bool_t_FALSE = 0, bool_t_TRUE = !bool_t_FALSE} ap_bool_t_val;

/*
 * part_id_t
 * 
 * A unique partition identifier that can be retrieved with the appropriate
 * devctl() to the partition manager and subsequently passed in calls to
 * posix_spawn() thereby allowing a spawned process to be associated with the
 * specified memory and/or scheduler partitions identified by 'part_id_t'.
 * A 'part_id_t' is also provided in xxx_PARTITION_ASSOCIATION and
 * xxx_PARTITION_DISASSOCIATION events 
*/
typedef _Uint32t	part_id_t;
#define part_id_t_INVALID	((part_id_t)0xFFFFFFFF)

/*
 * PART_TYPE
 * 
 * This macro can be used to determine the type of partition (memory or scheduler)
 * from a valid partition id <_id_>.
 * We use the 3 MSb's (ie 0x20000000 to 0x80000000)
*/
typedef enum
{
	parttype_NONE		= (uint32_t)(0 << 29),
	parttype_MEM		= (uint32_t)(1 << 29),
	parttype_SCHED		= (uint32_t)(2 << 29),

	parttype_MASK		= (uint32_t)(7 << 29),	/* 8 possible types */
	parttype_UNKNOWN	= (uint32_t)(part_id_t_INVALID & parttype_MASK),
} parttype_t;

#define PART_TYPE(_id_)	((((_id_) & parttype_MASK) == parttype_MEM) ? parttype_MEM : \
							((((_id_) & parttype_MASK) == parttype_SCHED) ? parttype_SCHED : parttype_UNKNOWN))

/*
 * part_dcmd_flags_t
 * 
 * Per process, common partitioning specific flags which can be set and
 * retrieved with the PART_SETFLAGS and PART_GETFLAGS devctl()'s respectively.
 * 
 * The partition flags consist of 4 bytes. Common flags are defined in byte 0
 * (LSB), memory partition flags in byte 1, scheduler partition flags in byte 2
 * and currently byte 3 is undefined
 * 
*/
#define part_flags_COMMON_MASK	0x000000FFU
#define part_flags_MEM_MASK		0x0000FF00U
#define part_flags_SCHED_MASK	0x00FF0000U
typedef enum
{
	/* the following flags are common to all partition types */
	part_flags_NONE			= 0x00,
	part_flags_NO_INHERIT	= 0x01,
	
	/* memory partition specific flags are defined in mempart.h and use byte 1 */

	/* scheduler partition specific flags are defined in schedpart.h and use byte 2 */
} part_dcmd_flags_t_val;

typedef _Uint32t	part_dcmd_flags_t;


/*
 * part_plist_t
 * part_olist_t
 * 
 * structures used to retrieve the list of processes (pid_t) or objects
 * associated with a partition respectively
 * Note that objects only apply to memory partitions
*/
typedef struct part_plist_s
{
	_Int32t	num_entries;		// number of 'pid' information structures
	pid_t	pid[1];
} part_plist_t;
/*
 * PART_PLIST_T_SIZE
 * 
 * convenience macro to calculate the size (in bytes) of the resulting
 * 'part_plist_t' structure given 'n' where 'n' (typically) represents the
 * value of the 'part_plist_t.num_entries' field.
 * 
 * Note:
 * 		'n' must be >= 0
 * 		implementation is coded to ensure the proper result even for 'n' == 0
*/
#define PART_PLIST_T_SIZE(n) \
		((sizeof(part_plist_t) - sizeof(pid_t)) + ((n) * sizeof(pid_t)))

/*
 * partlist_info_t
 * part_list_t
 * 
 * This type is used to obtain a list of partition id's that a process is
 * associated with.
*/
typedef struct {					
	part_id_t			id;
	part_dcmd_flags_t	flags;			/* partition specific flags */
	_Uint32t			reserved[2];
} partlist_info_t;

typedef struct part_list_s
{
	_Int32t 			num_entries;
	_Uint32t			reserved;
	partlist_info_t		i[1];
} part_list_t;

/*
 * PART_LIST_T_SIZE
 *
 * convenience macro to calculate the size (in bytes) of the resulting
 * 'part_list_t' structure given 'n' where 'n' (typically) represents the
 * value of the 'part_list_t.num_entries' field.
 * 
 * Note:
 * 		'n' must be >= 0
 * 		implementation is coded to ensure the proper result even for 'n' == 0
*/
#define PART_LIST_T_SIZE(n) \
		((sizeof(part_list_t) - sizeof(partlist_info_t)) + \
		((n) * sizeof(partlist_info_t)))

/*
 * cfgchg_t
 * cfgchg_t_val
*/
typedef _Uint32t	cfgchg_t;
typedef enum {
	cfgchg_t_NONE = 0,				

	cfgchg_t_ALLOC_POLICY = 0x1,	// the following flags when set indicate
	cfgchg_t_TERMINAL_POLICY = 0x2,	// a valid mempart_cfg_t value in val field.
	cfgchg_t_LOCK_POLICY = 0x4,		// An unset flag will cause the corresponding
	cfgchg_t_PERMANENT_POLICY = 0x8,// val field value to be left unchanged
	cfgchg_t_ATTR_MAX = 0x10,
	cfgchg_t_ATTR_CRIT_BUDGET = cfgchg_t_ATTR_MAX,
	cfgchg_t_ATTR_MIN = 0x20,
	cfgchg_t_ATTR_BUDGET = cfgchg_t_ATTR_MIN,

	cfgchg_t_ALL = 0xff,
} cfgchg_t_val;

/*
 * Common Partition devctl()'s (_DCMD_PARTITION)
 * 
 * PART_GET_ID
 * 	retrieve the partition identifier for the partition identified by 'fd'.
 * 	The partition identifier is also returned in partition association and
 * 	disassociation events.
 * 
 * 	On success, EOK will be returned by devctl() and the caller provided
 * 	'part_id_t' will contain the applicable information.
 * 	If any error occurs, the caller provided 'part_id_t' should be considered
 * 	to contain garbage.
 * 	The following errnos and their interpretation may be returned
 * 		ENOSYS - the partitioning module is not installed
 * 		EOVERFLOW - the caller has provided a buffer which is inconsistent with
 * 					the specified number of events being registered
 * 		EBADF - the PART_GET_ID devctl() has been issued on a name which
 * 				does not represent a valid partition name
 *
 *  
 * PART_EVT_REG
 * 	register the caller to receive asynchronous notification of specified number
 *  of partition events
 *  On success, EOK will be returned by devctl() and the caller can assume that
 * 	all events have been successfully registered.
 * 	If any error occurs, none of the requested events will be registered.
 * 	The following errnos and their interpretation may be returned
 * 		ENOSYS - the partitioning module is not installed
 * 		EACESS - the caller does not have read permission on the partition name
 * 		EOVERFLOW - the caller has not provided a buffer large enough for the
 * 					specified number of events
 * 				  - the caller has provided a event parameter value which could
 * 					not be internally represented. This can occur if the processor
 * 					does support a 64 bit types
 * 		EBADF - the PART_EVT_REG devctl() has been issued on a name which
 * 				does not represent a valid partition name
 * 		EINVAL - at least 1 of the events to be registered contained invalid
 * 				 data
 * 		ENOMEM - the memory necessary to register the events could not be
 * 				allocated
 * 
 * PART_GET_ASSOC_PLIST
 * 	return the list (part_plist_t) of processes associated with the partition
 * 	identifed by 'fd'.
 *  On success, EOK will be returned by devctl() and the caller provided
 * 	'part_plist_t' structure will contain some or all if the applicable
 * 	information (see @@ below)
 * 	If any error occurs, the caller provided part_plist_t structure should
 * 	be considered to contain garbage.
 * 	The following errnos and their interpretation may be returned
 * 		ENOSYS - the partitioning module is not installed
 * 		EOVERFLOW - the caller has provided a buffer which is inconsistent with
 * 					the amount of information being requested.
 * 		EBADF - the PART_GET_ASSOC_PLIST devctl() has been issued on a name
 * 				which does not represent a valid partition name
 *		EIO - internal I/O error. There is no information available or the
 * 				request to retrieve the information failed.
 * @@
 * When the caller provides the 'part_plist_t' buffer, the num_entries
 * field can be set to any value >= 0 as long as enough storage has been provided
 * to accomodate the value. If there are more entries to be returned than can fit
 * in the space specified by num_entries, then num_entries will be modified on
 * return to be the 2's compliment of the number of entries that would not fit.
 * Therefore, if you wish to first determine how many entries are required,
 * first pass in a part_plist_t buffer with num_entries set to zero. On
 * successful return, num_entries will be the 2's compliment (ie. negative) of
 * the num_entries that the caller should allocate space for (ie. num_entries = -10
 * indicates that the caller should allocate space for 10 entries).
 * If num_entries == 0 on successful return, all requested entries were returned.
 * 
 * PART_SETFLAGS
 * 
 * 	PART_SETFLAGS and PART_GETFLAGS operate on the /proc/<pid>/partition/
 * 	namespace and apply only to real partitions (only real partitions are visible
 * 	through this namespace). Suitable WRITE permissions must be present on the
 * 	partition path name in order to set the flags.
 * 
 * 	set the process specific partition flags for the process <pid> associated
 * 	with the partition identified by 'fd'. Note that partition flags are process
 * 	specific and the process must necessarily be associated with the partition in
 * 	order to set the flags (it won't be visible in the /proc/<pid>/partition/
 * 	namespace otherwise). A process can have its process specific partition flags
 * 	set prior to association by utilizing a properly initialized 'posix_spawnattr_t'
 * 	object prior to the process being posix_spawn()'d (see posix_spawnattr_addpartid()
 *  and posix_spawn_addpartition() for more details).
 * 
 * 	Process specific partition flags, if not set in the 'posix_spawnattr_t' object
 * 	prior to posix_spawn() will be inherited from the parent process for any
 * 	inherited partitions.
 * 
 * 	For example, suppose a parent process A has partition X and Y associations
 * 	without any flags set. If it spawns or forks a new process, the child process
 * 	B will also be associated with partitions X and Y. Now suppose that process B
 * 	sets the part_flags_NO_INHERIT flags for partition X. Now if process B spawns
 * 	or forks a child process C (grandchild of A), child process C will only inherit
 * 	an association with partition Y. A subsequent spawn or fork by process A again
 * 	will have the child D be associated with both partitions X and Y (demonstrating
 * 	the per process behavior of the partition flags).
 * 
 * PART_GETFLAGS
 * 	returns the per process partition flags for the process associated with the
 * 	partition identified by 'fd'. Suitable READ permissions must be present on the
 * 	partition path name in order to get the flags.
 *
*/
#define	PART_GET_ID				__DIOF(_DCMD_PARTITION, 1, part_id_t)
#define PART_GET_ASSOC_PLIST	__DIOTF(_DCMD_PARTITION, 2, part_plist_t)
#define EVENT_REG				__DIOT(_DCMD_PARTITION, 3, evtreg_t)
#define PART_SETFLAGS			__DIOT(_DCMD_PARTITION, 4, part_dcmd_flags_t)
#define PART_GETFLAGS			__DIOF(_DCMD_PARTITION, 5, part_dcmd_flags_t)

/* the following sub ranges are defined for memory and scheduler partitioning */
#define _DCMD_MEMPART_OFFSET			20
#define _DCMD_SCHEDPART_OFFSET			(_DCMD_MEMPART_OFFSET + 20)

#endif	/* _PART_H_ */
