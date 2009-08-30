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





#ifndef __SCHED_APS_H_INCLUDED
#define __SCHED_APS_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif
#ifndef _SCHED_H_INCLUDED
#include <sched.h>
#endif


__BEGIN_DECLS

/* Here are the defs for the parameters to SchedCtl for controlling the adaptive partitioning thread scheduler.
 * The ap scheduler is optional and is present only if something like [module=aps] appears in your buildfile.
 * 
 * See adaptive partitioning scheduler's user guide for more. 
 *
 * Example of a call to the AP scheduler: 
 *
 *     //declare parameter block 
 *     sched_aps_partition_info part_info; 
 *
 *     //initialize parameter block. Must be done.
 *     APS_INIT_DATA(&part_info); 
 *		
 *     //set input sub-paramters
 *     part_info.id = 2;
 *
 *     //invoke SchedCtl to perform the query partition function. 
 *     ret = SchedCtl(SCHED_APS_QUERY_PARTITION, &part_info, sizeof(part_info) );
 *     if (EOK!=ret) some_kind_of_error_handler(); 
 *
 *     //use output field 
 *     printf("&d",part_info.budget_percent); 
 *
 * 
 *
 * Use the SchedCtl kernel call to control APS: 
 * 
 *
 * int SchedCtl(int cmd, void *data, int length)  // kernel call 
 *
 * returns: EOK: 		success 
 *          ENOSYS:		APS scheduler not installed 
 *          EINVAL		size of parameter block does not match size structure expected
 *          EACCES		caller does not meet security options set (see SCHED_APS_ADD_SECURITY). Usually this means
 * 				you must be root. 
 *          EDOM		a reserved field is not zero. Probably becase APS_INIT_DATA() was not called on the data parameter 
 *                      
 *          others as noted below
 * 
 * cmd: a command. Is one of the SCHED_APS_... enums below 
 *
 * data: Must point to an instance of one of the parm stuctures following. Before passing data to SchedCtl you:
 *       1. must APS_INIT_DATA(&data); to init all reserved fields to zero.
 *       2. may seed to set some fields of data input values. 
 *
 * length: should be sizeof the data parm. 
 *
 * 
 * Each value of cmd has its own parameter structure. Some in-parms are optional as noted. Reserved fields are unused
 * but must be set to zero (by APS_INIT_DATA). 
 */




/* macro to prepare parameter blocks for use */
#define APS_INIT_DATA(parm_ptr) (memset((parm_ptr), 0, sizeof(*(parm_ptr))))


/* commands */ 
enum {
	SCHED_APS_QUERY_PARMS = SCHED_EXT_APS_CMD_BASE,
	SCHED_APS_SET_PARMS,
	SCHED_APS_CREATE_PARTITION,
	SCHED_APS_LOOKUP, 
	SCHED_APS_QUERY_PARTITION,
	SCHED_APS_JOIN_PARTITION,
	SCHED_APS_MODIFY_PARTITION,
	SCHED_APS_PARTITION_STATS,
	SCHED_APS_OVERALL_STATS,
	SCHED_APS_MARK_CRITICAL,
	SCHED_APS_CLEAR_CRITICAL,
	SCHED_APS_ATTACH_EVENTS,
	SCHED_APS_QUERY_THREAD,
	SCHED_APS_ADD_SECURITY,
	SCHED_APS_QUERY_PROCESS
};


/* SCHED_APS_QUERY_PARMS 
 *
 * Fills in following struct which describes overall parameters of the APS scheduler
 * 
 * returns:	EOK, ENOSYS, EINVAL, EACCES as described for SchedCtl 
 *
 */ 
typedef struct {
	_Uint64t		cycles_per_ms;		/* machine cycles in a millisecond. use to convert output of SCHED_APS_QUERY_PARTITION 
										to time units of your choice */
	_Uint64t		windowsize_cycles;	/* length of averaging window used for scheduling, in cpu cycles. By default 100ms */ 
	_Uint64t		windowsize2_cycles;	/* length of window 2, for reporting only, in cpu cyles. Typically 10 windowsizes*/
	_Uint64t		windowsize3_cycles;	/* length of window 3, for reporting only, in cpu cycles. Typcially 100 windowsizes*/ 
	_Uint32t		scheduling_policy_flags;/* or-ing of SCHED_APS_SHEDPOL_ bits */ 
	_Uint32t		sec_flags;		/* set of SCHED_APS_SEC_* flags describing security options */
	_Uint32t		bankruptcy_policy;	/* what to do if critical-time budgets are exceed, an oring of SCHED_APS_BNKR_* flags */
	_Uint16t		num_partitions;		/* number of currently defined partitions */ 
	_Uint16t		max_partitions;		/* maximum possible number of partitions */ 		 
	_Uint64t		reserved1;         
	_Uint64t		reserved2;         
} sched_aps_info;



/* SCHED_APS_SET_PARMS
 *
 * Set parameters for overall APS behavior.  
 *
 * returns:	EOK, ENOSYS, EINVAL described for SchedCtl 
 * 		EINVAL if windowsize if out of range (8ms to 400ms) or if windowsize/ClockPeriod is out of
 * 			range (2 to 400). 
 *		EACCESS: if SCHED_APS_SEC_PARTITIONS_LOCKED is set, or if you're not root running in the system partition
 *			when SCHED_APS_SEC_ROOT0_OVERALL is set.
 *		EINTR: if SchedCtl() was interrupted by two clock interrupts. (Very unlikely, unless you
 *			have overriden ClockPeriod() with a very small value.) 
 * 
 */ 
typedef struct { 
	_Int16t	windowsize_ms;	/*time over which scheduler will average cpu cycles and balance partitions to their
				* percentage budgets as specified by  APS_SCHED_CREATE_PARTITION. Set to -1 to skip. */
	_Int16t		reserved1;
	_Uint32t	*scheduling_policy_flagsp; /* points an or-ing of SCHED_APS_SCHED_SCHEDPOL_*i, described under "Scheduling Policy Flags" below. Set to NULL for no change */ 
    _Uint32t	*bankruptcy_policyp; /*points to an or-ing of SCHED_APS_BNKR_* flags, described under "handling bankruptcy" below. Set to NULL for no change*/	 
	_Int32t		reserved2;
	_Int64t		reserved3;
} sched_aps_parms;








/* SCHED_APS_CREATE_PARTITION
 *
 * Creates a new partition which is considered to be a child of the partition which calls SchedCtl. Budgets given to
 * the new partition are taken from the parent partition. Critical budgets do not affect the parent, but are 
 * automatically limited to be no bigger than the windowsize.
 *
 * The name parameter is optional. If not provided, Neutrino will assign a unique name in the range "Pa" to "Pz". 
 * The name may not contain a slash "/". 
 * 
 * Note: before creating zero-budget partitions, read cautions in user guide. 
 * 
 * returns:	
 * 	    EOK, ENOSYS, EINVAL as described for SchedCtl 
 *          EDQUOT: parent partition does not have enough budget 
 *          EINVAL: badly formed name, or budget out of range.
 *          ENAMETOOLONG: partition name > APS_PARTITION_NAME_LENGTH
 *          EEXIST: name already used by other partition
 *          ENOSPC: the maximum number of partitions already exist.
 *          EACCES: if SCHED_APS_SEC_PARTITIONS_LOCKED, or if any of these security conditions are set and not
 *          	satisfied:
 *          	SCHED_APS_SEC_ROOT_MAKES_PARTITIONS
 *          	SCHED_APS_SEC_SYS_MAKES_PARTITIONS
 *          	SCHED_APS_SEC_NONZERO_BUDGETS
 *          	SCHED_APS_SEC_ROOT_MAKES_CRITICAL
 *          	SCHED_APS_SEC_SYS_MAKES_CRITICAL
 *          	
 */ 
#define APS_SYSTEM_PARTITION_ID 0
#define APS_PARTITION_NAME_LENGTH 15 /*not including the trailing null */ 
#define APS_SYSTEM_PARTITION_NAME "System" /*Is the name of partition 0, created automatically by system. */

/* the following flags apply to the 'aps_create_flags' field */
#define APS_CREATE_FLAGS_USE_PARENT_ID	0x01	// if set 'parent_id' will be used, otherwise it is ignored

typedef struct {
	/* input parms */ 
	char		*name;		/* may be NULL ptr or "", both mean "Neutrino will assign a name" */ 
	_Uint16t	budget_percent;
	_Int16t		critical_budget_ms;	/*milliseconds. optional: set to -1 or 0 to skip */
	_Uint8t		aps_create_flags;		/* option partition creation flags */
	_Int8t		parent_id;	/* from which partition the budget should come. If -1, then calling thread */
	_Int16t		reserved1;
	_Uint64t 	reserved2;
	/* output parms */
	_Int16t		id;			/* created partition's id number 0 to max_partitions-1 */ 
	_Int16t		reserved3;
} sched_aps_create_parms;


/* SCHED_APS_QUERY_PARTITION
 * 
 * Fills in the following structure to describe one partition. To convert the budget_cycles field to something useful,
 * convert it with cycles_per_ms read with SCHED_APS_QUERY_PARMS. 
 * 
 * returns:		EOK, ENOSYS, EINVAL as described for SchedCtl 
 *
 */ 
typedef struct { 
	/* out parms */ 
	_Uint64t	budget_cycles;		
	_Uint64t	critical_budget_cycles; 
	char		name[APS_PARTITION_NAME_LENGTH+1]; 
	_Int16t		parent_id;  		/* is zero for the system partition. */
	_Uint16t	budget_percent;		
	_Int32t		notify_pid;		/* pid/tid of thread to be given overload and bankruptcy notifications, -1 if unset*/
	_Int32t		notify_tid;
	_Uint32t	pinfo_flags;		/* set of SCHED_APS_PINFO_* below */
	_Int32t		pid_at_last_bankruptcy; /*pid,tid at time of last bankruptcy, pid,tid= -1,-1 if none */
	_Int32t		tid_at_last_bankruptcy;
	_Int64t		reserved1;
	_Int64t		reserved2;
	/* input parm */
	_Int16t		id;  			/* partition number */
} sched_aps_partition_info; 

/* partition info flags */ 
#define SCHED_APS_PINFO_BANKRUPTCY_NOTIFY_ARMED		0x00000001   /*see SCHED_APS_ATTACH_EVENTS */ 
#define SCHED_APS_PINFO_OVERLOAD_NOTIFY_ARMED		0x00000002   /*see SCHED_APS_ATTACH_EVENTS */


/* SCHED_APS_LOOKUP 
 *
 * finds the partition id for a given partition name. Returns EINVAL if name not found. 
 *
 */
typedef struct {
		/* input parms */ 
		char	*name;
		_Int16t	reserved1;
		/* output parms */
		_Int16t		id;
} sched_aps_lookup_parms;



/* SCHED_APS_JOIN_PARTITION
 *
 * The thread specified by pid/tid becomes a member of the partition specified by the ID parameter. This partition
 * also becomes the thread's new home partition: where it returns after partition inheritance.
 * 
 * If pid/tid is zero, then the calling thread is joined to the specified
 * partition.
 *
 * If tid is -1, then the process specified by pid is joined to the
 * specified partition. Setting a process partition *doesn't* change the
 * partition of the threads within it, it just sets the partition these threads
 * will bill while they are handling a pulse.
 *
 * 
 * returns:
 * 			EOK, ENOSYS, EINVAL as described for SchedCtl 
 * 			EINVAL: also if an attempt is made to move the Idle thread (which must remain in System)
 * 			ESRCH: if pid/tid are invalid. 
 * 			EACCES: if the these security options are set but not satisfied:
 * 				SCHED_APS_SEC_ROOT_JOINS
 * 				SCHED_APS_SEC_SYS_JOINS
 * 				SCHED_APS_SEC_PARENT_JOINS
 * 				SCHED_APS_SEC_JOIN_SELF_ONLY
 *
 * */ 
typedef struct { 
		_Int16t		id; 
		_Int16t		reserved1;
		_Int32t		pid; 		/* zero means 'self' */ 
		_Int32t		tid;		/* zero means 'self, -1 means 'join process instead of thread' */
		_Int32t		reserved2;
} sched_aps_join_parms;


/* SCHED_APS_MODIFY_PARTITION
 *
 * Changes parameters of an existing partition. 
 *
 * If the new budget percent value is different from current, the difference is either taken from, or returned to, the 
 * budget of the parent partition. The critical time paramenter affects only the chosen partition, not its parent.
 *
 * To change just one of new budget or new critical time, set the other to -1. 
 *
 * Note, you cannot modify the budget of the system partition with this. To increase the size of the system partition,
 * modify one of it's child partitions to be smaller.
 *
 * Note: Reducing the size of partition may cause it to not run for the time of an averaging window as you may 
 *       caused it to become temporarily over-budget. However, reducing the critical time will not trigger the
 *       declaration of bankruptcy.
 *
 * returns:
 * 	EOK, ENOSYS, EINVAL as described for SchedCtl 
 *	EACCES: if SCHED_APS_SEC_PARTITIONS_LOCKED is set, or if these security options are set and not 
 *		satisfied:
 *			SCHED_APS_SEC_PARENT_MODIFIES
 *			SCHED_APS_SEC_ROOT_MAKES_PARTITIONS
 *			SCHED_APS_SEC_SYS_MAKES_PARTITIONS
 *			SCHED_APS_SEC_NONZERO_BUDGETS
 *			SCHED_APS_SEC_ROOT_MAKES_CRITICAL
 *			SCHED_APS_SEC_SYS_MAKES_CRITICAL
 *
 * */ 
typedef struct { 
		_Int16t		id;
		_Int16t		new_budget_percent;	/* optional, set to -1 to skip */ 
		_Int16t		new_critical_budget_ms;	/* optional, set to -1 to skip */ 
		_Int16t		reserved1;
		_Int64t		reserved2;
		_Int64t		reserved3;
} sched_aps_modify_parms; 



/*  SCHED_APS_PARTITION_STATS 
 *
 *  Returns the instantaneous values of cpu time accounting variables for a set of partitions
 *
 *  Data for more than one partition may be returned. To do so, pass an array of sched_aps_partition_stats 
 *  structures, with the 'id' field of the first element set to the id of the first partition for which
 *  you want data. SchedCtl will fill each of the sched_aps_partition_stats elements with stats for a different
 *  partition. 
 *  
 *  The sched_aps_partition_stats.id field is overwritten with the partition
 *  number for which data is being returned. -1 is written into the 'id' field of unused elements.  
 *
 *  To get a constant picture for the whole machine it's important to read data for all partitions in one call, 
 *  since sequential calls to SCHED_APS_PARTITION_STATS may come from separate averaging windows. 
 *
 *  
 *  Note use cycles_per_ms as returned by APS_QUERY_PARMS to convert run_time_cycles and critical_time_cyles 
 *  to a standard time unit. 
 *
 * returns:
 * 	EOK, ENOSYS, EINVAL as described for SchedCtl 
 * 	EINVAL if size is not a multiple of size(sched_aps_partition_stats) 
 * 	EINTR if SchedCtl() was interrupted by two clock interrupts (very unlikely unless you have overridden
 * 		ClockPeriod() with an unusually small value)
 * 		
 */ 
typedef struct { 
	/* out parms */ 
	_Uint64t		run_time_cycles;	/* cpu exceution time during last scheduling window */
	_Uint64t		critical_time_cycles;	/* time spent running critical during last scheduling window */ 
	_Uint64t		run_time_cycles_w2;	/* cpu time spent, during last windowsize2_cycles. Nominally 1sec*/
	_Uint64t		critical_time_cycles_w2;/* time spent running critical during last windowsize2_cycles.Nomially 1sec */ 
	_Uint64t		run_time_cycles_w3;	/* cpu time spent, during last windowsize3_cycles. Nominally 10 sec*/
	_Uint64t		critical_time_cycles_w3;/* time spent running critical during last windowsize3_cycles.Nomially 10 sec */ 
	_Uint32t		stats_flags;		/* set of SCHED_APS_PSTATS_* flags below  */
	_Uint32t		reserved1;
	_Uint64t		reserved2;
	_Uint64t		reserved3;
	/* in parm */
	_Int16t		id;  				/* actually an in and out parm */ 
} sched_aps_partition_stats;
#define SCHED_APS_PSTATS_IS_BANKRUPT_NOW 0x00000001 /* critical time used > critical budget at the time 
													   SCHED_APS_PARTITION_STATS is called */ 
#define SCHED_APS_PSTATS_WAS_BANKRUPT	0x00000002 /* the partition was declared to be bankrupt sometime since the last restart*/




/*	SCHED_APS_OVERALL_STATS 
 * 
 * Returns intantaneous values of overall cpu usage variables and other dynamic scheduler states. 
 *
 * returns:		EOK, ENOSYS, EINVAL as described for SchedCtl 
 */ 

typedef struct { 
		_Uint64t	idle_cycles;	/* Time during last scheduling window where nothing (other than idle ran 
									 * onvert to % idle time by 100*idle_cycles/windowxsize_cycles */
		_Uint64t	idle_cycles_w2;	/* Time spent running idle during last windowsize2_cycles. Nominally 1 sec */ 
		_Uint64t	idle_cycles_w3;	/* Time spent running idle during last windowsize3_cycles. Nominally 10 sec */ 
		_Int16t		id_at_last_bankruptcy; 	/* id of last bankrupt partition. -1 if none */ 
		_Int16t		reserved1; 	 
		_Int32t		pid_at_last_bankruptcy; /* pid,tid at last bankruptcy, pid,tid= -1,-1 if none */ 	
		_Int32t		tid_at_last_bankruptcy;
		_Uint32t	reserved2;
		_Uint32t 	reserved3;
		_Uint64t	reserved4;
} sched_aps_overall_stats;

/* SCHED_APS_MARK_CRITICAL
 *
 * Sets one thread in your process to run critical whenever it runs.
 * Use thread id of zero to set the calling thread to critical. 
 *
 *
 * In general, it's more useful to send a critical sigevent to a thread to make it run critical.
 * 
 * returns:
 * 	EOK, ENOSYS, EINVAL as described for SchedCtl 
 *	ESRCH, thread not found
 */
typedef struct {
		_Int32t		pid; 		/* 0 means 'self' */
		_Int32t		tid;		/* 0 means 'self' */ 
		_Int32t		reserved1;
} sched_aps_mark_crit_parms; 
/* see also siginfo.h  for setting sigevents to run their receiving threads critical */



/* SCHED_APS_CLEAR_CRITICAL 
 *
 * Clears the "always run critical" state set by SCHED_APS_MARK_CRITICAL. Then the thread will only run critical
 * when it inherits that state from another thread (on receipt of a message). 
 *
 *
 * returns:
 * 	EOK, ENOSYS, EINVAL as described for SchedCtl 
 *	ESRCH, thread not found
 */ 
typedef struct {
		_Int32t		pid; 		/* 0 means 'self' */
		_Int32t		tid;		/* 0 means 'self' */  
		_Int32t		reserved1;
} sched_aps_clear_crit_parms; 


/* SCHED_APS_QUERY_THREAD 
 *
 * Returns the partition of the given thread plus whether the given thread in your process is marked to run
 * critical. Use a thread id of zero to mean the calling thread. 
 *
 * !perm_critical & running_critical means the thread has temporarily inherited the critical state.
 * running_critical & !billed_as_critical means that the thread is running critical but is not depleting its
 * partition's critical-time budget (i.e. running for free). 
 *
 * returns:
 * 	EOK, ENOSYS, EINVAL as described for SchedCtl 
 *	ESRCH, thread not found
 */
typedef struct { 
		_Int32t		pid; 		/* 0 means 'self' */
		_Int32t		tid; 		/* 0 means 'self' */ 
		/* out parms: */
		_Int16t		id;					/* partition originally joined */ 
		_Int16t		inherited_id;		/* current partition, may be from inheritance */ 
		_Uint32t	crit_state_flags;	/* see aps_qcrit_* below */
		_Int32t		reserved1;
		_Int32t		reserved2;
} sched_aps_query_thread_parms;
#define APS_QCRIT_PERM_CRITICAL		0x00000001
#define APS_QCRIT_RUNNING_CRITICAL	0x00000002
#define APS_QCRIT_BILL_AS_CRITICAL	0x00000004


/* SCHED_APS_QUERY_PROCESS 
 *
 * Returns the partition of the given process. Use a process id of zero to mean
 * the calling process. The partition of a process is billed while one of the
 * threads in a process handles a pulse. The individual threads in a process may
 * all be in different partitions than the process.
 *
 *
 * returns:
 * 	EOK, ENOSYS, EINVAL as described for SchedCtl 
 *	ESRCH, process not found
 */
typedef struct { 
		_Int32t		pid; 		/* 0 means 'self' */
		/* out parms: */
		_Int16t		id;		/* partition of process */
		_Int16t		reserved1;
		_Int32t		reserved2;
		_Int32t		reserved3;
		_Int32t		reserved4;
} sched_aps_query_process_parms;


/* SCHED_APS_ATTACH_EVENTS 
 *
 * Defines sigevents that the scheduler will return the calling thread when the scheduler detects:
 *
 * 1. a given partition becoming bankrupt. 
 * 2. the whole system becoming overloaded.
 *
 * Calling SCHED_APS_ATTACH events arms notification once. After you receive the notification, you must call
 * SCHED_APS_ATTACH_EVENTS again to receive a subsequent notification. This is to ensure that the system does
 * not send you notifications faster than you can handle them. The pinfo_flags field of sched_aps_partition_stats
 * indicates if these events are armed. 
 *
 * Note that only one pair of sigevents, bankrupcty and overload, may be registered per partition and to one
 * receiving thread. The thread notified is the calling thread. Attaching events a second time overwrites the first.
 * Passing NULL pointers means "no changes in notification". To turn off notification, set the appropriate sigevent
 * to SIGEV_NONE with SIGEV_NONE_INIT.
 *
 * Note: See the "Handling Bankruptcy" to configure additional actions the system will perform upon bankruptcy. 
 *
 * Note: !!! Overload notification is not implemented in this release. !!!
 * returns:	
 * 	EOK, ENOSYS, EINVAL as described for SchedCtl 
 *	ESRCH, thread not found
 * 	EACCES: if you do not have the right to modify the partition, i.e if these security modes are set and
 * 		not satisfied:  
 *			SCHED_APS_SEC_PARENT_MODIFIES
 *			SCHED_APS_SEC_ROOT_MAKES_PARTITIONS
 *			SCHED_APS_SEC_SYS_MAKES_PARTITIONS
 *
 */
typedef struct { 
		const struct sigevent	*bankruptcy_notification;	/* may be NULL */
		const struct sigevent	*overload_notification;		/* may be NULL */ /* !!! not implemented in this release !!!*/		
		/* each partition gets a different set of sigevents */ 
		_Int16t					id; 						/* in-out parm. updated to partition used. */
															/* set to -1 to mean "partition of calling thread" */
		_Int16t					reserved1;
		_Int32t					reserved2;
		_Int64t					reserved3;
} sched_aps_events_parm;
/* APS @@@ errr, should we allow separate threads for bankruptcy and overload indication? */ 





/* Handling Bankruptcy  
 * ===================
 *
 * Bankruptcy is when critical cpu time billed to a partition exceeds it's critical budget. Bankruptcy is
 * always considered to be a design error on the part of the application, but the system's response is configurable. 
 *
 * 
 * If the system is not declaring bankruptcy when you expect it, note that bankruptcy can only be declared if critical
 * time is billed to your partition. Critical time is billed on those timeslices when these four conditions are 
 * all met:
 * 	1. the running partition has a critical budget greater than zero
 * 	2. the top thread in the partition is marked as running critical, or has received the critical state from
 * 	   receiving a SIG_INTR, a sigevent marked as critical, or has just received a message from a critical thread.
 * 	3. the running partition must be out of percentage-cpu budget 
 * 	4. there be at least one other partition that is competing for cpu time. 
 *
 * 	And then only if the billed critical time exceeds a partitions critical budget will the system declare bankrupcty.
 *
 * 
 * When the system detects bankruptcy it will always: 
 *
 * 1. cause that partition to be out of budget for the remainder of the current scheduling window. 
 * 2. If the user has set a sigevent for notify_bankrupcty with SCHED_APS_ATTACH_EVENTS, deliver the event. 
 *    This occurs at most once per calling SCHED_APS_ATTACH_EVENTS. 
 *
 * In addition the following responses are configurable. QNX recommends using SCHED_APS_BNKR_RECOMMENDED.
 */ 

#define SCHED_APS_BNKR_BASIC		0x0000000
/* This causes delivery of bankruptcy notification events and makes the partition out-of-budget for the rest of
 * the scheduling window (nominally 100ms). This is the default.  
 */ 

#define SCHED_APS_BNKR_CANCEL_BUDGET	0x00000001
/* Causes the system to set the offending partition's critical budget to zero, which forces the partition to be 
 * be scheduled by it's percentage cpu budget only. That also means that a second bankruptcy cannot occur.
 * This persists until a restart or SCHED_APS_MODIFY_PARTITION is called to set a new critical budget. 
 */

#define SCHED_APS_BNKR_LOG		0x000000002
/* Causes the system to log the occurrence of bankruptcy. To prevent causing a flood of logs, contiguous bankruptcies
 * which occur while the same process is running will be logged once. 
 * 
 * NOTE: This is not implemented in the current release. Output to slogger is scheduled for a later 
 * release.
 */ 

#define SCHED_APS_BNKR_REBOOT		0X000000004
/* The most severe response, suggested for use while testing a product to make sure bankruptcies will never be
 * ingored. Causes the system to crash with a brief message identifying the offending partition. Not recommended
 * for field use.
 */ 

#define SCHED_APS_BNKR_RECOMMENDED  (SCHED_APS_BNKR_CANCEL_BUDGET | SCHED_APS_BNKR_LOG) 
/* Most users should use this combination of bankruptcy-handling options */ 

/* to set a choice of bankruptcy handling options, create a or-ing of the above SCHED_APS_BNKR_* flags and pass pointer to it 
 * as the bankruptcy_policyp field of sched_aps_parms when calling SCHED_APS_SET_PARMS
 *
 */ 



/* Security 
 * ========
 * 
 * The APSched implementation permits dynamic creation and modification of partitions. However, QNX recommmends that
 * users setup their partition environment at restart time, and then lock all parameters, using
 * SCHED_APS_SEC_LOCK_PARTITIONS. However some customers may need to modify a partition at runtime. For those users
 * several security options are available. 
 *
 * When neutrino restarts, it will set APSched security to APS_SCHED_SEC_OFF. QNX recommends immediately
 * setting SCHED_APS_SEC_RECOMMENDED. One way to do so is 
 * 
 * 		sched_aps_security_parms p; 
 * 		p.sec_flags = SCHED_APS_SEC_RECOMMENDED; SchedCtl(SCHED_APS_ADD_SECURITY,&p, sizeof(p)); 
 * 		@@@ APS add comment about how to do this in build files. 
 *
 * 		
 * These are the security options: 
 *
 *
 * SCHED_APS_SEC_RECOMMENDED 
 * -------------------------
 * Allows only root from the system partition may create partitions or change parameters.
 * Arranges for a 2 level hierarchy of partitions: the system partition and its children. Only root, 
 * running in the system partition, may join it's own thread to partitions. Percentage budgets must not be zero.  
 * 
 * SCHED_APS_SEC_FLEXIBLE
 * ----------------------
 * Allows only root in the system partition to change scheduling parameters or to change critical budgets. But root 
 * running in any partition may create sub partitions, join threads into its own sub-partitions and modify 
 * sub_partitions. This permits applications to create their own local sub-partitions out of their own budgets. 
 * Percentage budgets must not be zero.
 *
 * SCHED_APS_SEC_BASIC
 * -------------------
 * Unless you are testing partitioning and want to change all parms without needing to restart, at least
 * BASIC security should be set. Only root in system may change overall scheduling parameters and set critical budgets.
 * 
 * In general, RECOMMENDED > FLEXIBLE > BASIC. Note all three allow partitions to be created and modified. After
 * seting up partitions use SCHED_APS_SEC_LOCK_PARTITIONS to prevent further unauthorized changes. Example:
 *
 * 		sched_aps_security_parms p; 
 * 		p.sec_flags = SCHED_APS_SEC_LOCK_PARTITIONS; SchedCtl(SCHED_APS_ADD_SECURITY,&p, sizeof(p)); 
 *
 * 
 * RECOMMENDED, FLEXIBLE, and BASIC options are composed of the flags defined below, but it's preferrable to use
 * the compound options decribed above. 
 */ 

#define SCHED_APS_SEC_ROOT0_OVERALL			0x00000001			
/* You must be root running in the system partition to change overall scheduling parameters, such as the averaging
 * windowize.  
 */

#define SCHED_APS_SEC_ROOT_MAKES_PARTITIONS		0x00000002
/* You must be you must be root to create or modify partitions. Applies to:	SCHED_APS_CREATE_PARTITION,
 * SCHED_APS_MODIFY_PARTITION and SCHED_APS_ATTACH_EVENTS.
 */
 
#define SCHED_APS_SEC_SYS_MAKES_PARTITIONS		0x00000004 
/* You must be running in the system partition to create or modify partitions. Applies to same commands as 
 * ROOT_MAKES_PARTITIONS. Attaching events, with SCHED_APS_ATTACH_EVENTS, is considered to be modifying the partition.
 */

#define SCHED_APS_SEC_PARENT_MODIFIES			0x00000008
/* Allows partitions to be modified (SCHED_APS_MODIFY_PARTITION), but the caller must be running in the parent partition
 * of the partition being modified. 'Modify' means change a partition's percent or critical budget, or attach events
 * with SCHED_APS_ATTACH_EVENTS.
 */

#define SCHED_APS_SEC_NONZERO_BUDGETS			0x00000010 
/* A partition may not be created with, or modified to have, a zero budget. Unless you know your partition only
 * needs to run in reponse to client requests, i.e. receipt of messages, this option should be set. 
 */ 

#define SCHED_APS_SEC_ROOT_MAKES_CRITICAL		0x00000020
/* Root is required to create a non-zero critical budget, or change and existing critical budget*/
  
#define SCHED_APS_SEC_SYS_MAKES_CRITICAL		0x00000040
/* You must be running in the system partition to create a non-zero critical budget or change an existing critical budget.*/ 
   
#define SCHED_APS_SEC_ROOT_JOINS			0x00000080 
/* You must be root to join a thread to a partition */ 

#define SCHED_APS_SEC_SYS_JOINS				0x00000100
/* You must be running in the system partition to join a thread */

#define SCHED_APS_SEC_PARENT_JOINS			0x00000200
/* You must be running in the parent partition of the partition you wish to join to. */

#define SCHED_APS_SEC_JOIN_SELF_ONLY			0X00000400
/* The caller of SCHED_APS_JOIN_PARTITION must specify 0/0 for pid/tid, in other words, a process may only join itself
 * to a partition
 */ 

#define SCHED_APS_SEC_PARTITIONS_LOCKED			0x00000800
/* Prevents further changes to any partition budget, or overall scheduling parameters, like windowsize. Set this after
 * you have set up a set of partitions. Once locked, SCHED_APS_JOIN_PARTITION and SCHED_APS_ATTACH_EVENTS may still be used.
 */



#define SCHED_APS_SEC_BASIC	(SCHED_APS_SEC_ROOT0_OVERALL | SCHED_APS_SEC_ROOT_MAKES_CRITICAL)

#define SCHED_APS_SEC_FLEXIBLE (SCHED_APS_SEC_BASIC | SCHED_APS_SEC_NONZERO_BUDGETS | SCHED_APS_SEC_ROOT_MAKES_PARTITIONS |\
		SCHED_APS_SEC_PARENT_JOINS | SCHED_APS_SEC_PARENT_MODIFIES ) 

#define SCHED_APS_SEC_RECOMMENDED (SCHED_APS_SEC_FLEXIBLE | SCHED_APS_SEC_SYS_MAKES_PARTITIONS | SCHED_APS_SEC_SYS_JOINS\
		| SCHED_APS_SEC_JOIN_SELF_ONLY)

#define SCHED_APS_SEC_OFF				0x00000000

/* SCHED_APS_ADD_SECURITY,
 * 
 * Sets security options. A bit set turns the corresponding security option on. Successive calls add to the existing
 * set of security options. Security options can only be cleared by a restart. 
 *
 * sec_flags is updated with the current set of options. So calling with sec_flags =0 reads the security settings.
 *
 * You must be root running in partition zero to call this, even if all security options are off. 
 *
 * returns:		EOK, ENOSYS, EINVAL, EACCES as described for SchedCtl 
 *
 */

typedef struct {
		_Uint32t		sec_flags; /* set of SCHED_APS_SEC_* flags, both in and out parm, set to 0 to read sec_flags */
		_Uint32t		reserved1;
		_Uint32t		reserved2;
} sched_aps_security_parms;




/* Scheduling Policy Flags * 
 * -----------------------
 *  
 * They set options on details of the adaptive-partition scheduling algorithm. 
 *
 * To set, pass a pointer to an OR-ing of these flags with the SCHED_APS_SET_PARMS call to SchedCtl.. 
 *
 *
 * FREETIME_BY_RATIO 
 *
 * "Free time" is when at least one partition is not running. Its time becomes free to other partitions who
 * may then run over their budgets. 
 * 
 * By default, the scheduler hands out "free time" to the partition with the highest-priority running thread. 
 * That guarantees real-time scheduling behavior (i.e. scheduling strictly by priority) to partitions anytime they
 * are not being limited by some other partition's right to its guaranteed minimum budget. But it also means that 
 * one partition is allowed to grab all the free time.
 *
 * When FREETIME_BY_RATIO is set, the running partitions share free time by the ratios of their
 * budgets. So, one partition can no longer grab all the free time. However, when FREETIME_BY_RATIO is set, partitions
 * will see strict priorty-scheduling between partitions only when they are consuming less than their cpu budgets. 
 *
 * Scheduling within a partition always is strictly by priority -- independant of the FREETIME_BY_RATIO
 * setting.
 * 
 */
#define SCHED_APS_SCHEDPOL_FREETIME_BY_RATIO    0x0001

/* BMP_SAFETY
 *
 * Strict priority scheduling between partitions, with some combinations of partition budgets, and some cominations
 * of runmasks (i.e. Bound Multi-Processing) can require the AP scheduler to not meet minimum cpu budgets. When
 * BMP_SAFETY is set, the scheduler use a more restictive algorithm that guarantees minimum cpu budgets but
 * gives priority-based scheduling between partitions only when when partitions are consuming less than their budgets.
 * When BMP_SAFETY is set, neutrino will automatically set FREETIME_BY_RATIO. 
 * 
 * Scheduling within a partition always is strictly by priority -- independant of the BMP_SAFETY
 * setting.
 *
 * See the Adpative Partitioning Scheduler user's guide for more on APS with Bound MultiProcessing (BMP) */
#define SCHED_APS_SCHEDPOL_BMP_SAFETY		0x0002 


#define SCHED_APS_SCHEDPOL_DEFAULT              0X0000
/* Means not FREETIME_BY_RATIO and not BMP_SAFETY. Neutrino sets this at startup.  
 */ 





/* Overlays "debug_process_t.extsched" from <sys/debug.h> */
struct extsched_aps_dbg_process {
	unsigned char	id;
	char			reserved[7];
};
/* Overlays "debug_thread_t.extsched" from <sys/debug.h> */
struct extsched_aps_dbg_thread {
	unsigned char	id;
	char			reserved[7];
};

__END_DECLS

#endif


/* __SRCVERSION("sched_aps.h $Rev: 168521 $"); */
