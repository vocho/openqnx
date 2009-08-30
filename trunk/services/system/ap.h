/* 
 * $QNXLicenseC:  
 * Copyright 2006, QNX Software Systems. All Rights Reserved.
 *
 * This source code may contain confidential information of QNX Software 
 * Systems (QSS) and its licensors.  Any use, reproduction, modification, 
 * disclosure, distribution or transfer of this software, or any software 
 * that includes or is based upon any of this code, is prohibited unless 
 * expressly authorized by QSS by written agreement.  For more information 
 * (including whether this source code file has been published) please
 * email licensing@qnx.com. $
*/

#ifndef _AP_H_
#define _AP_H_

#include <stdbool.h>

#ifdef __WATCOMC__
typedef char _Bool;	// not done in stdbool.h for watcom
#endif	/* __WATCOMC__ */ 

/*
 * this file contains common interfaces, types and definitions for the
 * partitioning modules.
*/
/*
 * ===========================================================================
 * 
 * 				Partitioning Compile time Behaviour Modification
 *  
 * ===========================================================================
*/
/*
 * USE_PROC_OBJ_LISTS
 * 
 * whether to use the process and object lists to keep track of process and
 * object associations to a memory partition.
 * 
 * Note that it is extremely unlikely at this point that this define will ever
 * be turned off however it is here because the code still contains the
 * conditional compilation control ... although I have not compiled with this
 * #define off in many moons so it may not even build without it anymore.
 * 
 * Just leave it alone ... go away ... skedaddle
*/
#define USE_PROC_OBJ_LISTS

/*
 * ext_lockfncs_t
 *  
 * This type is to allow me to pass partitioning specific lock/unlock functions
 * to the partitioning modules.
 * For example, for the process association/disassociation implementation in
 * mm_mempart.c.
*/
/*
 * part_evtflags_DEACTIVATE
 * 
 * This flag is used by event registration to mark an event as being deactivated.
 * This causes the event delivery code to move the event off of the active list
 * and on to the inactive list where it will eventually be deleted
*/
#define evtflags_DEACTIVATE		(0x80000000 & evtflags_RESERVED)

typedef struct ext_lockfncs_s
{
	int (*rdlock)(void *);
	int (*rdunlock)(void *);
	int (*wrlock)(void *);
	int (*wrunlock)(void *);
} ext_lockfncs_t;


typedef LINK4_NODE		part_qnode_t;
typedef LINK4_HDR		part_qnodehdr_t;
typedef struct
{
	part_qnodehdr_t		list;
	struct intrspin		lock;
} part_qnodehdr2_t;

/*
 * Internal event related types
 * 
 * evtdest_t
 * part_evtlist_t
*/
typedef struct
{
	int					rcvid;		// who to send the event to
	pid_t				pid;		// the process wishing to receive the events
	bool				in_progress;// break recursion for allocations during event delivery
} evtdest_t;

typedef struct
{
	part_qnodehdr2_t	active;
	part_qnodehdr2_t	inactive;
} part_evtlist_t;


#ifdef USE_PROC_OBJ_LISTS
/* prp_node_t */
typedef struct prp_node_s
{
	part_qnode_t	hdr;
	PROCESS *	prp;
} prp_node_t;
#endif	/* USE_PROC_OBJ_LISTS */


/*
 * LIST manipulation macros
*/
#ifdef __GNUC__
#define TYPEOF(t)	typeof(t)
#else	/* __GNUC__ */
#define TYPEOF(t)	// ok for now because the LINKx macros don't current use the type anyway
#endif	/* __GNUC__ */
#define LIST_INIT(_q)		LINK4_INIT((_q))
#define LIST_FIRST(_q)		(_q).head
#define LIST_LAST(_q)		(_q).tail
#define LIST_COUNT(_q)		(_q).count
#define LIST_ADD(_q, _n)	LINK4_END((_q), &(_n)->hdr, TYPEOF(_n))
#define LIST_DEL(_q, _n)	LINK4_REM((_q), &(_n)->hdr, TYPEOF(_n))
#define LIST_NEXT(_n)		((_n)->hdr.next)
#define LIST_PREV(_n)		((part_qnode_t *)((((char *)((_n)->hdr.prev))) - offsetof(part_qnode_t, next)))

#endif	/* _AP_H_ */

/* __SRCVERSION("$IQ: ap.h,v 1.91 $"); */
