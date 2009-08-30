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


#ifndef _MEMCLASS_H_
#define _MEMCLASS_H_

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include <stdint.h>
#include <limits.h>
#include <signal.h>
#include <devctl.h>

#if defined(__GNUC__)
/*
 * these array element and structure field "safe" initializations are not
 * supported by the WATCOM compiler.
*/
	#define ARRAY_EL(e)		[(e)] =
	#define STRUCT_FLD(e)	.e =
#else
	#define ARRAY_EL(e)
	#define STRUCT_FLD(e)
#endif	/* __GNUC__ */

/*
 * memsize_t
 * 
 * This type is used to represent all size information for the memory class
 * and memory partitioning systems.
 * The API defines this type to be 64 bits regardless of the processor on which
 * code may be executing. Internally, if the processor does not support 64 bit
 * types, and any values exchanged between an application and the OS cannot be
 * represented, a EOVERFLOW error will be returned   
*/
#ifndef _MEMSIZE_T_
#define _MEMSIZE_T_		_Uint64t
typedef _MEMSIZE_T_		memsize_t;
#endif

/*
 * memsize_t_INFINITY
 * 
 * When creating a partition, the caller may wish to leave the maximum value
 * unspecified so that any limits will be governed by partitions higher up in
 * the hierarchy. This value should be used to represent such a choice.
*/
#define memsize_t_INFINITY	((_MEMSIZE_T_)ULLONG_MAX)

/*
 * memclass_id_t
 * 
 * Memory class identifier
 * This identifier is used to specify the class of memory for which the
 * configuration attributes and/or information sizes refer to.
*/
typedef _Uint32t	memclass_id_t;
#define memclass_id_t_INVALID	((memclass_id_t)0xFFFFFFFF)

/*
 * memclass_limits_t
 * 
 * This structure defines specific limits as related to the memory class and its
 * associated allocator. The values are considered read-only and are provided
 * to allow an application to make optimal decisions when interacting with the
 * allocator for the memory class.
 * 
 * For example, the allocation sub-structure indicates the smallest and largest
 * allocatable unit supported by the allocator for the class. 
*/
typedef struct memclass_limits_s
{
	struct {
		struct {
			_MEMSIZE_T_		min;		// minimum size of an allocation unit
			_MEMSIZE_T_		max;		// maximum size of an allocation unit
		} size;
	} alloc;

} memclass_limits_t;

/*
 * memclass_attr_t
 * 
 * This structure defines the attributes of a memory class
*/
typedef struct memclass_attr_s
{
	_MEMSIZE_T_			size;		// the amount of available memory
	memclass_limits_t	limits;		// limits as defined above
} memclass_attr_t;
/*
 * MEMCLASS_DEFAULT_ATTR
 * 
 * Default static memory class attribute initializer
*/
#define MEMCLASS_DEFAULT_ATTR \
		{ \
			STRUCT_FLD(size) 0, \
			STRUCT_FLD(limits) \
				{STRUCT_FLD(alloc) \
					{STRUCT_FLD(size) \
						{STRUCT_FLD(min) 0, STRUCT_FLD(max) 0}}}, \
		}

/*
 * memclass_sizeinfo_t
 * 
 * This structure defines the retrievable size information provided by the
 * allocator for the memory class. Memory class size information consists of
 * the amount of reserved memory (memory that has been set aside for partitions
 * created with a non-zero minimum value) and unreserved memory (memory which
 * is freely available to all partitions of the memory class in a discretionary
 * fashion.
 * The sum of all fields will always be equivalent to the size attribute
 * defined in memclass_attr_t. 
*/
typedef struct memclass_sizeinfo_s
{
	struct {
		_MEMSIZE_T_	free;
		_MEMSIZE_T_	used;
	} reserved;
	struct {
		_MEMSIZE_T_	free;
		_MEMSIZE_T_	used;
	} unreserved;
} memclass_sizeinfo_t;

/*
 * memclass_info_t
 * 
 * This structure defines the information that may be retrieved from the
 * allocator for a memory class.
*/
typedef struct memclass_info_s
{
	memclass_id_t		id;
	_Uint32t			reserved;
	memclass_attr_t		attr;
	memclass_sizeinfo_t	size;
} memclass_info_t;

/*
 * proc_memclass_sizeinfo_t
 * 
 * This structure defines the retrievable size information for each class of
 * memory a process is associated with.
 * 
 * When the caller provides the 'proc_memclass_sizeinfo_t' buffer, the num_entries
 * field can be set to any value >= 0 as long as enough storage has been provided
 * to accomodate a value > 0. If there are more entries to be returned than can
 * fit in the space specified by num_entries, then num_entries will be modified on
 * return to be the 2's compliment of the number of entries that would not fit.
 * Therefore, if you wish to first determine how many entries are required,
 * first pass in a mempart_olist_t buffer with num_entries set to zero. On
 * successful return, num_entries will be the 2's compliment (ie. negative) of
 * the num_entries that the caller should allocate space for (ie. num_entries = -10
 * indicates that the caller should allocate space for 10 entries).
 * 
 * num_entries >= 0 indicates the number of entries actually returned.
*/
typedef struct proc_memclass_sizeinfo_t
{
	_Int32t	num_entries;	// number of mempart_evtparam_t structures which follow
	_Uint32t	reserved[3];
	struct per_class_sizeinfo_s
	{
		memclass_id_t	mclass_id;	// memory class id to which the size pertains
		_MEMSIZE_T_	used;			// the amount of memory class 'mclass_id' in use by the process
		_MEMSIZE_T_ free;			// the amount of memory class 'mclass_id' available to the process
									// Note that the sum of 'used' + 'free' DOES NOT necessarily reflect
									// the size of the partition of 'mclass_id'. Check the respective
									// partition (of 'mclass_id') for that data
	} si[1];
} proc_memclass_sizeinfo_t;
#define PROC_MEMCLASS_SIZEINFO_T_SIZE(n) \
		((sizeof(proc_memclass_sizeinfo_t) - sizeof(struct per_class_sizeinfo_s)) + \
		((n) * sizeof(struct per_class_sizeinfo_s)))

/*
 * Memory Class devctl()'s (_DCMD_MEMCLASS)
 * 
 * MEMCLASS_GET_INFO
 * 	retrieve the memory class information
 * 	On success, EOK will be returned by devctl() and the caller provided
 * 	'memclass_info_t' structure will contain the applicable information.
 * 	If any error occurs, the caller provided memclass_info_t structure should
 * 	be considered to contain garbage.
 * 	The following errnos and their interpretation may be returned
 * 		ENOSYS - the memory partitioning module is not installed
 * 		EACESS - the caller does not have read permission on the memory class
 * 		EOVERFLOW - the caller has provided a buffer which is inconsistent with
 * 					the specified number of events being registered
 * 		EBADF - the MEMCLASS_GET_INFO devctl() has been issued on a name which
 * 				does not represent a memory class
 * 		ENOENT - there is no memory class information for the open()'d name
 * 		EIO - internal I/O error. There is no size information available or
 * 				the request to retrieve the information failed.
 * 
 * MEMCLASS_EVT_REG
 *	register to receive one or more of the defined memory class events
 * 	On success, EOK will be returned by devctl() and the caller can assume that
 * 	all events have been successfully registered.
 * 	If any error occurs, none of the requested events will be registered.
 * 	The following errnos and their interpretation may be returned
 * 		ENOSYS - the memory partitioning module is not installed
 * 		EACESS - the caller does not have read permission on the memory class
 * 		EOVERFLOW - the caller has not provided a buffer large enough for the
 * 					specified number of events
 * 				  - the caller has provided a event parameter value which could
 * 					not be internally represented. This can occur if the processor
 * 					does support a 64 bit types
 * 		EBADF - the MEMCLASS_EVT_REG devctl() has been issued on a name which
 * 				does not represent a memory class
 * 		EINVAL - at least 1 of the events to be registered contained invalid
 * 				 data
 * 		ENOMEM - the memory necessary to register the events could not be
 * 				allocated
 * 
 * 
*/
#define MEMCLASS_GET_INFO		__DIOF(_DCMD_MEMCLASS, 1, memclass_info_t)

/*
 * FIX ME
 * The following are accessible through the /proc/<pid> namespace. They should
 * be moved to an ap.h or apmgr.h public header file
 *
 * APMGR_GET_PROC_MEMINFO
 *
 * return meminfo about a process. Currently this is used to return the
 * amount of physical memory of each associated class that is in use by a
 * process
*/
#define APMGR_GET_PROC_MEMINFO   __DIOTF(_DCMD_MEMCLASS, 10, proc_memclass_sizeinfo_t)


#endif	/* _MEMCLASS_H_ */
