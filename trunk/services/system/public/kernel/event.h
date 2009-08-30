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

#ifndef _EVENT_H_
#define _EVENT_H_

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include <stdint.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>

#include <kernel/mempart.h>
#include <kernel/schedpart.h>

/*
 * evttype_t
 * evtclass_t
*/
typedef _Int32t		evttype_t;
typedef _Int32t		evtclass_t;
typedef enum
{
evtclass_t_INVALID = -1,

	evtclass_t_MEMCLASS,
	evtclass_t_MEMPART,
	evtclass_t_SCHEDPART,

// (evtclass_t_last - evtclass_t_first) is always the number of event classes
evtclass_t_last,
evtclass_t_first = evtclass_t_MEMCLASS,

} evtclass_t_val;


/*
 * mempart_evttype_t
 * 
 * This structure defines the set of events that can be registered with a
 * memory partition and subsequently received by the caller
*/
typedef evttype_t	mempart_evttype_t;
typedef enum
{
mempart_evttype_t_INVALID = -1,

	mempart_evttype_t_THRESHOLD_CROSS_OVER,
	mempart_evttype_t_THRESHOLD_CROSS_UNDER,
	mempart_evttype_t_DELTA_INCR,
	mempart_evttype_t_DELTA_DECR,
	mempart_evttype_t_CONFIG_CHG_POLICY,
	mempart_evttype_t_CONFIG_CHG_ATTR_MIN,
	mempart_evttype_t_CONFIG_CHG_ATTR_MAX,
	mempart_evttype_t_PARTITION_CREATE,
	mempart_evttype_t_PARTITION_DESTROY,
	mempart_evttype_t_PROC_ASSOCIATE,
	mempart_evttype_t_PROC_DISASSOCIATE,

	// not sure what these next 2 could be used for other than debug
	mempart_evttype_t_OBJ_ASSOCIATE,
	mempart_evttype_t_OBJ_DISASSOCIATE,

// (mempart_evttype_t_last - mempart_evttype_t_first) is always the number of event types
mempart_evttype_t_last,	
mempart_evttype_t_first = mempart_evttype_t_THRESHOLD_CROSS_OVER,
} mempart_evttype_t_val;

/* convenience macros */
#define NUM_MEMPART_EVTTYPES		(mempart_evttype_t_last - mempart_evttype_t_first)
#define MEMPART_EVTYPE_IDX(etype)	((etype) - mempart_evttype_t_first)

/*
 * mempart_evtparam_t
 * 
 * This structure defines the parameters of an event that may be registered
 * with a memory partition.
 * For each event type, an optional value may be provided which acts as a filter
 * to the event delivery mechanism. For example, a threshold crossing event
 * requires an absolute value (between 0 and the maximum size attribute of the
 * memory partition) through which the memory partition current size value
 * (mempart_info_t.cur_size) must cross before the event is delivered.
*/
typedef struct mempart_evtparam_s
{
	mempart_evttype_t	type;	// event type
	_Uint32t			reserved[3];
	union {
		// event types
		//		mempart_evttype_t_THRESHOLD_CROSS_OVER
		//		mempart_evttype_t_THRESHOLD_CROSS_UNDER
		//
		// a notification event is generated when the partition crosses the
		// specified threshold in the specified direction. The new current size
		// of the partition is provided
		_MEMSIZE_T_	threshold_cross;

		// event types
		//		mempart_evttype_t_DELTA_INCR
		//		mempart_evttype_t_DELTA_DECR
		//
		// a notification event is generated when the partition changes size
		// by the specified delta in the specified direction from the last time
		// the event was received or when it is first registered.
		// The new current size of the partition is provided
		_MEMSIZE_T_	delta;

		// event types
		//		mempart_evttype_t_CONFIG_CHG_POLICY
		//		mempart_evttype_t_CONFIG_CHG_ATTR_MIN
		//		mempart_evttype_t_CONFIG_CHG_ATTR_MAX
		// a notification event is generated when the partition configuration
		// changes. The config change is separated into either a policy change
		// or an attribute change with each a separately configurable event.
		//
		// The policy changes are further subdivided into a bitmask for the boolen
		// policies (terminal, config lock and permanent) and an 8 bit field for
		// the allocation policy. Since the boolen policies can only ever transition
		// from FALSE to TRUE, the bit position represented by cfgchg_t_xxx will
		// indicate the current policy setting. The alloc policy will be provided
		// as the current value. Any change in any of the allocation policies
		// will cause an mempart_evttype_t_CONFIG_CHG_POLICY event to be sent.
		// 
		// Attribute changes are subdivided into either a change to the minimum
		// (reserved) value or a change to the maximum (restriction) value. A
		// mempart_evttype_t_CONFIG_CHG_ATTR_MIN or mempart_evttype_t_CONFIG_CHG_ATTR_MAX
		// event will be sent repectively.
		union mp_cfg_chg_s {
			struct {
				_Uint8t		b;		// bitset of the boolean policies
				_Uint8t		alloc;	// mempart_alloc_policy_t encoded into a _Uint8t
				_Uint8t		reserved[6];
			} policy;
			union {
				_MEMSIZE_T_	min;
				_MEMSIZE_T_	max;
			} attr;
		} cfg_chg;

		// event types
		//		mempart_evttype_t_PARTITION_CREATE
		//		mempart_evttype_t_PARTITION_DESTROY
		// A notification event is generated when a partition is created as the
		// child of the partition or memory class on which the caller is registering.
		// The partition identifier for the created partition is provided.
		//
		// A notification event is also generated for a partition destruction event
		// and the caller has the option of specifying an 'mpid' indicating interest
		// in the destruction of a specific existing child partition. A value of
		// 0 will cause an event for any child partition destruction.
		//
		// NOTE that in a partition hierarchy, only the immediate child partitions
		// of the partition or memory class to which the event has been registered
		// will cause and event to be generated.
		part_id_t	mpid;

		// event types
		//		mempart_evttype_t_PROC_ASSOCIATE
		//		mempart_evttype_t_PROC_DISASSOCIATE
		// a notification event is generated when a process associates with or
		// disassociates from the partition to which the event is registered.
		// The caller has the option of specifying a 'pid' indicating interest
		// in the disassociation of a specific existing process. A value of 0
		// will cause an event for any process disassociation from the partition
		// to which the event is registered.
		//
		// NOTE that in a partition hierarchy, only the processes associated with
		// or disassociated from the partition to which the event has been registered
		// will cause and event to be generated.
		pid_t	pid;

		// event types
		//		mempart_evttype_t_OBJ_ASSOCIATE
		//		mempart_evttype_t_OBJ_DISASSOCIATE
		// a notification event is generated when a process creates an object
		// which is associated with or an object disassociates from the partition
		// to which the event is registered.
		// The caller has the option of specifying a 'oid' indicating interest
		// in the disassociation of a specific existing associated object.
		// A value of 0 will cause an event for any object disassociation from
		// the partition to which the event is registered.
		//
		// NOTE that in a partition hierarchy, only the processes associated with
		// or disassociated from the partition to which the event has been registered
		// will cause and event to be generated.
		void *	oid;

	} val;
} mempart_evtparam_t;


/*
 * memclass_evttype_t
 * 
 * This structure defines the set of events that can be registered with a
 * memory class and subsequently received by the caller
*/
typedef evttype_t	memclass_evttype_t;
typedef enum
{
memclass_evttype_t_INVALID = -1,

	//	free and used crossings (total)
	memclass_evttype_t_THRESHOLD_CROSS_TF_OVER,
	memclass_evttype_t_THRESHOLD_CROSS_TF_UNDER,
	memclass_evttype_t_THRESHOLD_CROSS_TU_OVER,
	memclass_evttype_t_THRESHOLD_CROSS_TU_UNDER,
	//	free crossings (reserved and unreserved)
	memclass_evttype_t_THRESHOLD_CROSS_RF_OVER,
	memclass_evttype_t_THRESHOLD_CROSS_RF_UNDER,
	memclass_evttype_t_THRESHOLD_CROSS_UF_OVER,
	memclass_evttype_t_THRESHOLD_CROSS_UF_UNDER,
	//	used crossings	(reserved and unreserved)
	memclass_evttype_t_THRESHOLD_CROSS_RU_OVER,
	memclass_evttype_t_THRESHOLD_CROSS_RU_UNDER,
	memclass_evttype_t_THRESHOLD_CROSS_UU_OVER,
	memclass_evttype_t_THRESHOLD_CROSS_UU_UNDER,
	
	//	free and used deltas (total)
	memclass_evttype_t_DELTA_TF_INCR,
	memclass_evttype_t_DELTA_TF_DECR,
	memclass_evttype_t_DELTA_TU_INCR,
	memclass_evttype_t_DELTA_TU_DECR,
	//	free deltas (reserved and unreserved)
	memclass_evttype_t_DELTA_RF_INCR,
	memclass_evttype_t_DELTA_RF_DECR,
	memclass_evttype_t_DELTA_UF_INCR,
	memclass_evttype_t_DELTA_UF_DECR,
	//	used deltas (reserved and unreserved)
	memclass_evttype_t_DELTA_RU_INCR,
	memclass_evttype_t_DELTA_RU_DECR,
	memclass_evttype_t_DELTA_UU_INCR,
	memclass_evttype_t_DELTA_UU_DECR,
	
	// memory class partition creation/destruction
	memclass_evttype_t_PARTITION_CREATE,
	memclass_evttype_t_PARTITION_DESTROY,

// (memclass_evttype_t_last - memclass_evttype_t_first) is always the number of event types
memclass_evttype_t_last,
memclass_evttype_t_first = memclass_evttype_t_THRESHOLD_CROSS_TF_OVER,
} memclass_evttype_t_val;

/* convenience macros */
#define NUM_MEMCLASS_EVTTYPES			(memclass_evttype_t_last - memclass_evttype_t_first)
#define MEMCLASS_EVTYPE_IDX(etype)		((etype) - memclass_evttype_t_first)

/*
 * memclass_evtparam_t
 * 
 * This structure defines the parameters of an event that may be registered
 * with a memory class.
 * For each event type, an optional value may be provided which acts as a filter
 * to the event delivery mechanism. For example, a threshold crossing event
 * requires an absolute value (between 0 and the size attribute of the memory
 * class) through which the specified memory class size value (memclass_sizeinfo_t)
 * must cross before the event is delivered.
*/
typedef struct memclass_evtparam_s
{
	memclass_evttype_t	type;	// event type
	_Uint32t			reserved[3];
	union {
		// event types
		//		memclass_evttype_t_THRESHOLD_CROSS_TF_OVER
		//		memclass_evttype_t_THRESHOLD_CROSS_TF_UNDER
		//		memclass_evttype_t_THRESHOLD_CROSS_TU_OVER
		//		memclass_evttype_t_THRESHOLD_CROSS_TU_UNDER
		//	free crossings (reserved and unreserved)
		//		memclass_evttype_t_THRESHOLD_CROSS_RF_OVER
		//		memclass_evttype_t_THRESHOLD_CROSS_RF_UNDER
		//		memclass_evttype_t_THRESHOLD_CROSS_UF_OVER
		//		memclass_evttype_t_THRESHOLD_CROSS_UF_UNDER
		//	used crossings	(reserved and unreserved)
		//		memclass_evttype_t_THRESHOLD_CROSS_RU_OVER
		//		memclass_evttype_t_THRESHOLD_CROSS_RU_UNDER
		//		memclass_evttype_t_THRESHOLD_CROSS_UU_OVER
		//		memclass_evttype_t_THRESHOLD_CROSS_UU_UNDER
		//
		// a notification event is generated when the memory class crosses the
		// specified threshold in the specified direction. In order to provide
		// the most flexibility, a number of events are specified which specifically
		// allow events to be generated (in either direction) on either
		//		total free (reserved free + unreserved free)
		//		total used (reserved used + unreserved used)
		//		reserved free
		//		reserved used
		//		unreserved free
		//		unreserved used
		// The new memory class value (as it relates to the specific event) will
		// be provided
		_MEMSIZE_T_		threshold_cross;

		// event types
		//	free and used deltas (total)
		//		memclass_evttype_t_DELTA_TF_INCR
		//		memclass_evttype_t_DELTA_TF_DECR
		//		memclass_evttype_t_DELTA_TU_INCR
		//		memclass_evttype_t_DELTA_TU_DECR
		//	free deltas (reserved and unreserved)
		//		memclass_evttype_t_DELTA_RF_INCR
		//		memclass_evttype_t_DELTA_RF_DECR
		//		memclass_evttype_t_DELTA_UF_INCR
		//		memclass_evttype_t_DELTA_UF_DECR
		//	used deltas (reserved and unreserved)
		//		memclass_evttype_t_DELTA_RU_INCR
		//		memclass_evttype_t_DELTA_RU_DECR
		//		memclass_evttype_t_DELTA_UU_INCR
		//		memclass_evttype_t_DELTA_UU_DECR
		//
		// a notification event is generated when the memory class changes by an
		// amount >= the specified delta from the time the last event was
		// received or when it is first registered. In order to provide the most
		// flexibility, a number of events are defined which specifically allow
		// events to be generated (in either direction) on either ...
		//		total free (reserved free + unreserved free)
		//		total used (reserved used + unreserved used)
		//		reserved free
		//		reserved used
		//		unreserved free
		//		unreserved used
		// The new memory class value (as it relates to the specific event) will
		// be provided
		_MEMSIZE_T_		delta;

		// event types
		//		memclass_evttype_t_PARTITION_CREATE
		//		memclass_evttype_t_PARTITION_DESTROY
		//
		// A notification event is generated when a partition of the memory
		// class on which the caller is registering is either created or
		// destroyed.
		// The partition identifier for the created partition is provided.
		//
		// NOTE that any partition created or destroyed for the memory class
		// will cause an event. These events ARE NOT hierarchy sensitive as with
		// partition creation/destruction events.
		_Uint32t	mpid;	// can not bring in part_id_t (chicken and egg)

	} val;
} memclass_evtparam_t;



/*
 * schedpart_evttype_t
 * 
 * This structure defines the set of events that can be registered with a
 * scheduler partition and subsequently received by the caller
*/
typedef evttype_t	schedpart_evttype_t;
typedef enum
{
schedpart_evttype_t_INVALID = -1,

	schedpart_evttype_t_THRESHOLD_CROSS_OVER,
	schedpart_evttype_t_THRESHOLD_CROSS_UNDER,
	schedpart_evttype_t_DELTA_INCR,
	schedpart_evttype_t_DELTA_DECR,
	schedpart_evttype_t_CONFIG_CHG_POLICY,
	schedpart_evttype_t_CONFIG_CHG_ATTR_BUDGET,
	schedpart_evttype_t_CONFIG_CHG_ATTR_CRIT_BUDGET,
	schedpart_evttype_t_PARTITION_CREATE,
	schedpart_evttype_t_PARTITION_DESTROY,
	schedpart_evttype_t_PROC_ASSOCIATE,
	schedpart_evttype_t_PROC_DISASSOCIATE,

// (schedpart_evttype_t_last - schedpart_evttype_t_first) is always the number of event types
schedpart_evttype_t_last,	
schedpart_evttype_t_first = schedpart_evttype_t_THRESHOLD_CROSS_OVER,
} schedpart_evttype_t_val;

/* convenience macros */
#define NUM_SCHEDPART_EVTTYPES		(schedpart_evttype_t_last - schedpart_evttype_t_first)
#define SCHEDPART_EVTYPE_IDX(etype)	((etype) - schedpart_evttype_t_first)

/*
 * schedpart_evtparam_t
 * 
 * This structure defines the parameters of an event that may be registered
 * with a scheduler partition.
 * For each event type, an optional value may be provided which acts as a filter
 * to the event delivery mechanism. For example, a threshold crossing event
 * requires an absolute value (between 0 and the maximum size attribute of the
 * scheduler partition) through which the scheduler partition current size value
 * (schedpart_info_t.cur_size) must cross before the event is delivered.
*/
typedef struct schedpart_evtparam_s
{
	schedpart_evttype_t	type;	// event type
	_Uint32t			reserved[3];
	union {
		// event types
		//		schedpart_evttype_t_THRESHOLD_CROSS_OVER
		//		schedpart_evttype_t_THRESHOLD_CROSS_UNDER
		//
		// a notification event is generated when the partition crosses the
		// specified threshold in the specified direction. The new current size
		// of the partition is provided
		uint32_t	threshold_cross;

		// event types
		//		schedpart_evttype_t_DELTA_INCR
		//		schedpart_evttype_t_DELTA_DECR
		//
		// a notification event is generated when the partition changes size
		// by the specified delta in the specified direction from the last time
		// the event was received or when it is first registered.
		// The new current size of the partition is provided
		uint32_t	delta;

		// event types
		//		schedpart_evttype_t_CONFIG_CHG_POLICY
		//		schedpart_evttype_t_CONFIG_CHG_ATTR_BUDGET
		//		schedpart_evttype_t_CONFIG_CHG_ATTR_CRIT_BUDGET
		// a notification event is generated when the partition configuration
		// changes. The config change is separated into either a policy change
		// or an attribute change with each a separately configurable event.
		//
		// The policy changes are further subdivided into a bitmask for the boolen
		// policies (terminal, config lock and permanent). Since the boolen
		// policies can only ever transition from FALSE to TRUE, the bit position
		// represented by cfgchg_t_xxx will indicate the current policy setting.
		// Any change in any of the allocation policies will cause a
		// schedpart_evttype_t_CONFIG_CHG_POLICY event to be sent.
		// 
		// Attribute changes are subdivided into either a change to the budget
		// value or a change to the critical budget value. A
		// schedpart_evttype_t_CONFIG_CHG_ATTR_BUDGET or
		// mempart_evttype_t_CONFIG_CHG_ATTR_CRIT_BUDGET event will be sent repectively. 
		union sp_cfg_chg_s {
			struct {
				_Uint8t		b;		// bitset of the boolean policies
				_Uint8t		reserved[7];
			} policy;
			union {
				uint32_t	budget;
				uint32_t	crit_budget;
			} attr;
		} cfg_chg;

		// event types
		//		schedpart_evttype_t_PARTITION_CREATE
		//		schedpart_evttype_t_PARTITION_DESTROY
		// A notification event is generated when a partition is created as the
		// child of the partition or scheduler class on which the caller is registering.
		// The partition identifier for the created partition is provided.
		//
		// A notification event is also generated for a partition destruction event
		// and the caller has the option of specifying an 'mpid' indicating interest
		// in the destruction of a specific existing child partition. A value of
		// 0 will cause an event for any child partition destruction.
		//
		// NOTE that in a partition hierarchy, only the immediate child partitions
		// of the partition or scheduler class to which the event has been registered
		// will cause and event to be generated.
		part_id_t	spid;

		// event types
		//		schedpart_evttype_t_PROC_ASSOCIATE
		//		schedpart_evttype_t_PROC_DISASSOCIATE
		// a notification event is generated when a process associates with or
		// disassociates from the partition to which the event is registered.
		// The caller has the option of specifying a 'pid' indicating interest
		// in the disassociation of a specific existing process. A value of 0
		// will cause an event for any process disassociation from the partition
		// to which the event is registered.
		//
		// NOTE that in a partition hierarchy, only the processes associated with
		// or disassociated from the partition to which the event has been registered
		// will cause and event to be generated.
		pid_t	pid;

	} val;
} schedpart_evtparam_t;











/*
 * evtparam_t
*/
typedef union
{
	memclass_evtparam_t		mc;
	mempart_evtparam_t		mp;
	schedpart_evtparam_t	sp;
} evtparam_t;


/*
 * evtflags_t
 * 
 * This structure defines the attributes of events that may be registered on
 * a memory partition. Events may be global (they apply to all defined memory
 * partition events) or specific to an event type
*/
typedef _Uint32t	evtflags_t;
typedef enum
{
	evtflags_DISARM = 0x0,		// Default action
										// the event, once delivered is disarmed
										// and must be re-registered in order to
										// be received again.
	evtflags_REARM = 0x10,		// the event, once delivered will remain in effect
/* FIX ME - make this a flag like SIGEV_FLAG_CRITICAL in siginfo,h ? */
	evtflags_SIGEV_FLAG_SIGINFO = 0x200,

	/* top 4 bits are reserved. Do not use */
	evtflags_RESERVED = 0xF0000000,

} evtflags_t_val;


/*
 * evtreg_t
 * 
 * The structure defines the event registration structure. This structure is
 * a variable size structure which is used by applications to register to
 * receive 1 or more events of interest from partition of a memory class.
 * The same event may be registered multiple times with different parameter
 * values.
 * When registering an event, the caller is required to provide properly
 * initialized mempart_evtparam_t, sigevent and evtflags_t values.
 * 
 * The macros can be used to aid in the initialization of the evtreg_t structure. 
*/
typedef struct evtreg_s
{
	_Uint32t	num_entries;	// number of mempart_evtparam_t structures which follow
	_Uint32t	reserved[3];
	struct evt_info_s {
		evtclass_t		class;	// event class
		evtparam_t		info;	// the event to register for
		struct sigevent	sig;	// sigevent
		evtflags_t		flags;	// event specific flags
		_Uint32t		reserved;
	} evt[1];
} evtreg_t;
/*
 * EVTREG_T_SIZE
 * 
 * convenience macro to calculate the size (in bytes) of the resulting
 * 'evtreg_t' structure given 'n' where 'n' (typically) represents the
 * value of the 'evtreg_t.num_entries' field.
 * 
 * Note:
 * 		'n' must be >= 0
 * 		implementation is coded to ensure the proper result even for 'n' == 0 
*/
#define EVTREG_T_SIZE(n) \
		((sizeof(evtreg_t) - sizeof(struct evt_info_s)) + \
		((n) * sizeof(struct evt_info_s)))




#endif	/* _MEMPART_H_ */
