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

#ifndef _MEMPART_H_
#define _MEMPART_H_

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include <stdint.h>
#include <limits.h>
#include <signal.h>
// FIX ME #include <sys/memclass.h>
#include <sys/part.h>
#include <kernel/memclass.h>


/*
 * mempart_attr_t
 * 
 * This type defines the attributes used in creating a memory partition
 * 
 * size.min	- A size.min > 0 will guarantee the specified amount of memory that
 * 			  can be allocated by processes associated with the partition but it
 * 			  does not guarantee that an allocation will be successful if the
 * 			  allocation fails for reasons other than for errno == ENOMEM.
 * 			  size.min can never be greater than size.max.
 * size.max	- size.max will limit the amount of memory that can be allocated
 * 			  by processes associated with the partition to the specified value.
 * 			  size.max can never be less than size.min.
 * 
 * When creating a partition with non-zero size.min and size.max other than
 * memsize_t_INFINITY, the values are constrained by the memclass_limits_t.alloc.min
 * setting for the memory class. That is, values must be chosen which are modulo
 * the memclass_limits_t.alloc.min
*/
typedef struct mempart_attr_s
{
	struct {
		_MEMSIZE_T_		min;
		_MEMSIZE_T_		max;
	} size;
} mempart_attr_t;


/*
 * mempart_alloc_policy_t
 * 
 * Valid memory partition allocation policies
*/
typedef _Uint32t	mempart_alloc_policy_t;
typedef enum
{
	/*
	 * mempart_alloc_policy_t_ROOT
	 *	  -	all allocations (including reservations) are autonomous and not
	 * 		accounted for in a partition hierarchy. The parent/child partition
	 * 		relationship exists to constrain partition creation. A hieracrhy may
	 * 		exist in the namespace, however child partitions operate independently
	 * 		for the purpose of accounting allocations
	*/
	mempart_alloc_policy_t_ROOT = 1,
	
	/*
	 * mempart_alloc_policy_t_HIERARCHICAL
	 * 	  -	all allocations (including reservations) are accounted for in the
	 * 		partition hierarchy. This allocation policy results in a completely
	 * 		hierarchical relationship in which memory allocated by a process
	 * 		associated with a given partition must be available in all partitions
	 * 		higher in the tree up to and including the root partition and 
	 * 		associated memory class allocator
	*/
	mempart_alloc_policy_t_HIERARCHICAL,

mempart_alloc_policy_t_last,
mempart_alloc_policy_t_first = mempart_alloc_policy_t_ROOT,
mempart_alloc_policy_t_DEFAULT = mempart_alloc_policy_t_HIERARCHICAL,
} mempart_alloc_policy_t_val;


/*
 * mempart_policy_t
 * 
 * This structure defines the set of policies by which a partition operates.
 * The allocation policies are described above.
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
typedef struct mempart_policy_s
{
	ap_bool_t				terminal;	// child partitions are permitted (T/F) ?
	ap_bool_t				config_lock;		// configuration is locked (T/F) ?
	ap_bool_t				permanent;			// partition is permanent (T/F) ?
	mempart_alloc_policy_t	alloc;				// the partition allocation policy

} mempart_policy_t;

#define MEMPART_DFLT_POLICY_INITIALIZER \
		{ \
			STRUCT_FLD(terminal) bool_t_FALSE, \
			STRUCT_FLD(config_lock) bool_t_FALSE, \
			STRUCT_FLD(permanent) bool_t_FALSE, \
			STRUCT_FLD(alloc) mempart_alloc_policy_t_HIERARCHICAL, \
		}

/*
 * mempart_cfg_t
 * 
 * This type defines the configuration attributes and policies for the partition.
 * This type is used both to return the configuration state of the partition as
 * well as to allow the configuration to be changed (see mempart_cfgchg_t)
*/
typedef struct mempart_cfg_s
{
	mempart_policy_t	policy;
	mempart_attr_t		attr;
} mempart_cfg_t;

/*
 * mempart_cfgchg_t
 * 
 * This type is used to change the configuration attributes and policies for the
 * partition. The 'valid' field allows the caller to specify which attributes
 * and/or polices should be changed without changing the other attributes and/or
 * policies and therefore does not require knowledge of the partition's current
 * configuration
*/
typedef struct mempart_cfgchg_s
{
	cfgchg_t		valid;
	mempart_cfg_t	val;
} mempart_cfgchg_t;

/*
 * mempart_meminfo_t
 *  
 * This type defines the information that can be obtained from a memory
 * partition. Its fields have the following meaning
 *
 * cfg		- partition creation attributes. This field contains the attributes
 * 			  that were specified when the partition was originally created. The
 * 			  values may or may not be the same as the current partition attributes.
 * 			  This value is provided as a record of the partition creation history
 * 			  only and is not otherwise used.
 * 
 * cur		- current partition attributes
 * 			  The cur.min_size and cur.max_size specify what the current min_size
 * 			  and max_size attributes are (configuration attributes may be altered
 * 			  with the MEMPART_CFG_CHG devctl()). At partition creation, 'cur' will
 * 			  be equal to 'cfg'.
 * 
 * cur_size - current partition size.
 * 			  This value indicates the amount of memory that has been successfully
 * 			  allocated by processes associated with the partition. Successful
 * 			  allocations include the min_size for any partition lower in the
 * 			  partition hierarchy. The difference between max_size and cur_size
 * 			  represents the amount of memory that 'may' be allocated by processes
 * 			  associated with the partition but does not necessarily guarantee
 *			  that the amount of memory is actually available.
 * 
 * hi_size	- indicates the maximum amount of memory that has ever been allocated
 * 			  by any process associated with the partition since the value was
 * 			  reset. The value will be reset to 0 upon partition creation or with
 * 			  the MEMPART_CNTR_RESET devctl(). hi_size could be greater than
 * 			  max_size if the partition attributes are changed such that cur.max_size
 * 			  is no longer equal to cfg.max_size.
 * 
 * id		- partition identifier
*/
typedef struct mempart_meminfo_s
{
	_MEMSIZE_T_		cur_size;		// current partition size
	_MEMSIZE_T_		hi_size;		// largest cur_size that this partition has ever been
	mempart_cfg_t	cre_cfg;		// configuration at partition creation
	mempart_cfg_t	cur_cfg;		// current configuration
	part_id_t	id;					// partition identifier
	_Uint32t		num_children;	// number of (immediate) children this partition has
									// Don't need 32 bits but want to use atomic_() operations
	_Uint32t		reserved[2];
} mempart_info_t;


/*
 * mempart_olist_t
 * 
 * structure used to retrieve the list of objects associated with a memory
 * partition
*/
typedef struct mempart_olist_s
{
	_Int32t		num_entries;		// number of 'obj' information structures
	_Uint32t	reserved;
	struct obj_info_s
	{
		_Uint8t					type;
		_Uint8t					size;
		_Uint16t				flags;
		part_id_t				mpid;
		_Uint32t				reserved[2];
	} obj[1];
} mempart_olist_t;
/*
 * MEMPART_OLIST_T_SIZE
 * 
 * convenience macro to calculate the size (in bytes) of the resulting
 * 'mempart_olist_t' structure given 'n' where 'n' (typically) represents the
 * value of the 'mempart_olist_t.num_entries' field.
 * 
 * Note:
 * 		'n' must be >= 0
 * 		implementation is coded to ensure the proper result even for 'n' == 0
*/
#define MEMPART_OLIST_T_SIZE(n) \
		((sizeof(mempart_olist_t) - sizeof(struct obj_info_s)) + ((n) * sizeof(struct obj_info_s)))

/*
 * mempart_dcmd_flags_t
 * 
 * Memory Partitioning specific flags which can be set and retrieved with the
 * DCMD_ALL_SETFLAGS and DCMD_ALL_GETFLAGS respectively
 * 
 * Some notes on the CREATE and SHARE flags
 * If the 'mempart_flags_HEAP_CREATE' flag is set, a kernel heap for the memory
 * class represented by the partition for which the flag is set, WILL be created.
 * If the 'mempart_flags_HEAP_SHARE' is not set then this will be a private
 * heap for the exclusive use of the caller. If 'mempart_flags_HEAP_SHARE' is
 * set as well, then the heap WILL be created (even if there are other shared
 * heaps from the same partition) but it will also then be shareable. This means
 * that if in the future, a process associates with the partition with only the
 * 'mempart_flags_HEAP_SHARE' flag set, then that process will use the same heap
 * as the first process it finds that is associated with the partition that also
 * has the 'mempart_flags_HEAP_SHARE' flag set. Since multiple processes could
 * associate with a partition with both the 'mempart_flags_HEAP_CREATE' and
 * 'mempart_flags_HEAP_SHARE' flags set, the actual heap to be shared will depend
 * on the chronological order in which the processes associate as well as whether
 * or not the process that created a shared heap is still present in the system.
 * 
 * If a process associates with a partition and only has the 'mempart_flags_HEAP_SHARE'
 * flag set and there are no other processes associated with the partition which
 * have the 'mempart_flags_HEAP_SHARE' flag set then the behaviour will be as if
 * both the 'mempart_flags_HEAP_CREATE' and 'mempart_flags_HEAP_SHARE' flags were
 * specified.
 * 
 * If a process associates with a partition and does not specify either the
 * 'mempart_flags_HEAP_CREATE' or 'mempart_flags_HEAP_SHARE' flags, then no
 * kernel heap will be created. The memory class represented by the partition
 * will still govern user memory allocations, however no memory of the class
 * will be available for internal memory allocations. This is not a problem as
 * most internal memory allocations utilize the 'sysram' memory class and when
 * a process is created, the system automatically ensures that it is associated
 * with a partition of the 'sysram' memory class either by inheritance or by
 * ensuring that the the appropriate paremeters to posize_spawn() are provided. 
 * 
*/
typedef part_dcmd_flags_t	mempart_dcmd_flags_t;
typedef enum
{
	/* memory partitioning flags are defined in byte 1 (see part.h) */

	mempart_flags_HEAP_CREATE	= (0x01 << 8) & part_flags_MEM_MASK,
	// create a process private kernel HEAP for the memory class represented by the partition

	mempart_flags_HEAP_SHARE	= (0x02 << 8) & part_flags_MEM_MASK,
	// find (or create) a common heap for the memory class represented by the partition
 
} mempart_dcmd_flags_t_val;

/*
 * Memory Partition specific devctl()'s in addition to those defined in
 * part.h
 * 
 * MEMPART_GET_INFO
 * 	return memory partition information (mempart_info_t) for the partition
 * 	identified by 'fd'.
 *  On success, EOK will be returned by devctl() and the caller provided
 * 	'mempart_info_t' structure will contain the applicable information.
 * 	If any error occurs, the caller provided mempart_info_t structure should
 * 	be considered to contain garbage.
 * 	The following errnos and their interpretation may be returned
 * 		ENOSYS - the memory partitioning module is not installed
 * 		EOVERFLOW - the caller has provided a buffer which is inconsistent with
 * 					the specified number of events being registered
 * 		EBADF - the MEMPART_GET_INFO devctl() has been issued on a name which
 * 				does not represent a memory partition
 *		EIO - internal I/O error. There is no information available or the
 * 				request to retrieve the information failed.
 * 
 * MEMPART_CFG_CHG
 * 	change the current configuration attributes for the partition identified by
 *  'fd' to those specified in the provided 'mempart_cfgchg_t'. The success of
 *  this operation depends on the current partition configuration, current
 *  parent partition configuration, allocation use and the amount of unallocated
 *  unreserved memory of the class the partition represents. The reconfiguration
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
 * 		ENOSYS - the memory partitioning module is not installed
 * 		EOVERFLOW - the caller has provided a buffer which is inconsistent with
 * 					the specified number of events being registered
 * 		EBADF - the MEMPART_CFG_CHG devctl() has been issued on a name which
 * 				does not represent a memory partition
 * 
 * MEMPART_CNTR_RESET
 * 	reset the the hi_size high water mark counter. You may want to use this to
 * 	reset the counters if the configuration parameters of the partition are
 *	modified. The hi water mark value at the time the reset occurs will be
 * 	returned in the caller provided 'memsize_t' buffer
 * 
 * MEMPART_GET_ASSOC_OLIST
 * 	return the list (mempart_olist_t) of objects associated with the
 * 	partition identifed by 'fd'.
 *  On success, EOK will be returned by devctl() and the caller provided
 * 	'mempart_olist_t' structure will contain some or all if the applicable
 * 	information (see @@ below)
 * 	If any error occurs, the caller provided mempart_olist_t structure should
 * 	be considered to contain garbage.
 * 	The following errnos and their interpretation may be returned
 * 		ENOSYS - the memory partitioning module is not installed
 * 		EOVERFLOW - the caller has provided a buffer which is inconsistent with
 * 					the amount of information being requested.
 * 		EBADF - the MEMPART_GET_ASSOC_OLIST devctl() has been issued on a name
 * 				which does not represent a memory partition
 *		EIO - internal I/O error. There is no information available or the
 * 				request to retrieve the information failed.
 * @@
 * When the caller provides the 'mempart_olist_t' buffer, the num_entries
 * field can be set to any value >= 0 as long as enough storage has been provided
 * to accomodate a value > 0. If there are more entries to be returned than can fit
 * in the space specified by num_entries, then num_entries will be modified on
 * return to be the 2's compliment of the number of entries that would not fit.
 * Therefore, if you wish to first determine how many entries are required,
 * first pass in a mempart_olist_t buffer with num_entries set to zero. On
 * successful return, num_entries will be the 2's compliment (ie. negative) of
 * the num_entries that the caller should allocate space for (ie. num_entries = -10
 * indicates that the caller should allocate space for 10 entries).
 * If num_entries == 0 on successful return, all requested entries were returned.
 *
*/
#define MEMPART_CFG_CHG			__DIOT(_DCMD_PARTITION, _DCMD_MEMPART_OFFSET + 0, mempart_cfgchg_t)
#define MEMPART_CNTR_RESET		__DIOF(_DCMD_PARTITION, _DCMD_MEMPART_OFFSET + 1, memsize_t)
#define MEMPART_GET_INFO		__DIOTF(_DCMD_PARTITION, _DCMD_MEMPART_OFFSET + 2, mempart_info_t)
#define MEMPART_GET_ASSOC_OLIST	__DIOTF(_DCMD_PARTITION, _DCMD_MEMPART_OFFSET + 3, mempart_olist_t)


#endif	/* _MEMPART_H_ */
