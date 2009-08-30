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

#ifndef _MCLASS_H_
#define _MCLASS_H_

// FIX ME #include <sys/memclass.h>
#include "kernel/memclass.h"
#include "kernel/event.h"


#ifndef EXTERN
	#define EXTERN	extern
#endif

/*
 * allocator_accessfncs_t
 * 
 * This type defines the interface to the allocator for a memory class. By
 * utilizing a 'memclass_info_t *', the same allocator can be used for multiple
 * memory classes
*/
typedef struct allocator_accessfncs_s
{
	memsize_t				(*reserve)(memsize_t s, memsize_t adjust, int adjust_dir, memclass_info_t *i);
	memsize_t				(*unreserve)(memsize_t s, memsize_t adjust, int adjust_dir, memclass_info_t *i);
	memclass_sizeinfo_t *	(*size_info)(memclass_sizeinfo_t *si, memclass_info_t *i);
	void					(*resv_adjust)(memsize_t adjust, int sign, memclass_info_t *i);
} allocator_accessfncs_t;

extern allocator_accessfncs_t *dflt_memclass_fncs;

/*
 * memclass_t
 * 
 * This is the internal memory class structure. It is created when a memory class
 * is added to the system (typically through the resource manager).
 * 
 * This type is used in combination with a 'name' to form an entry in the memory
 * class list.
*/
#define MEMCLASS_SIGNATURE		(~0x13991187U)
typedef struct memclass_s
{
	_Uint32t		signature;		// used to help ensure not looking at a bogus memclass_t

	memclass_info_t		info;		// (public) accounting and configuration data
	allocator_accessfncs_t	allocator;

	void 			*rmgr_attr_p;	// back pointer to resource manager structure
} memclass_t;


/*
 * memclass_entry_t
 * 
 * A 'memclass_entry_t' is created for each of the memory classes which are
 * added to the system. The entries are added/removed to/from as physical memory
 * classes are added/removed to/from the system (note that adding a physical
 * chunk of memory does not "necessarily" add a new memory class unless that new
 * chunk of physical memory is desired to be partitioned independently. It can
 * otherwise simply be added to the pool of other chunks of physical memory
 * belonging to an existing memory class).
 * 
 * The current implementation requires that the name specified when adding a
 * memory class correspond to a syspage memory class name. This is because
 * what is otherwise currently known to the system as separate classes of
 * memory resides in the system page. Without a lot of rewrite is was decided
 * that this nehaviour would be enforced. The memory classes are therefore
 * added in startup and made available to be partitioned by add the class using
 * the syspage name
 *
*/
typedef struct memclass_entry_s
{
	kerproc_lock_t	lock;
	memclass_t		data;
	char			name[NAME_MAX+1];	// the syspage name by which the meory is known
} memclass_entry_t;

struct apmmgr_attr_s;
extern void (*memclass_event)(struct apmmgr_attr_s *attr, memclass_evttype_t evtype,
						memclass_sizeinfo_t *cur, memclass_sizeinfo_t *prev);


/*==============================================================================
 * 
 * 				public interfaces for memory class management
 * 
*/

/*
 * MEMCLASS_PID_USE
 * MEMCLASS_PID_FREE
 * 
 * These macros will perform per process, per memory class accounting of
 * physical memory allocations and deallocations respectively.
 * Parameters are PROCESS * 'p', memclass_id_t 'mc' and allocation size 'sz'
 * See kernel/types.h for reason why #if defined(__PPC__) is included in the
 * conditional compilation check
*/
#if (_PADDR_BITS == 64) || defined(__PPC__)
#define MEMCLASS_ATOMIC_MEM_USED_ADD(mp, y) \
		do { \
			extern char alives[]; /* this check prevents int lock/unlock during initialization */\
			if (alives[0] != 0) {INTR_LOCK(&(mp)->lock);} \
			(mp)->mem_used += (y); \
			if (alives[0] != 0) {INTR_UNLOCK(&(mp)->lock);} \
		} while(0)
#define MEMCLASS_ATOMIC_MEM_USED_SUB(mp, y) \
		do { \
			extern char alives[]; /* this check prevents int lock/unlock during initialization */\
			if (alives[0] != 0) {INTR_LOCK(&(mp)->lock);} \
			(mp)->mem_used -= (y); \
			if (alives[0] != 0) {INTR_UNLOCK(&(mp)->lock);} \
		} while(0)
#else	/* (_PADDR_BITS == 64)  || defined(__PPC__) */
#define MEMCLASS_ATOMIC_MEM_USED_ADD(mp, y)	atomic_add((volatile unsigned *)(&(mp)->mem_used), (y))
#define MEMCLASS_ATOMIC_MEM_USED_SUB(mp, y)	atomic_sub((volatile unsigned *)(&(mp)->mem_used), (y))
#endif	/* (_PADDR_BITS == 64)  || defined(__PPC__) */

#define _MEMCLASS_PID_USE(p, mc, sz) \
		do { \
			mempart_node_t *mp_node; \
			CRASHCHECK((p) == NULL); \
			mp_node = MEMPART_NODEGET((p), (mc)); \
			CRASHCHECK(mp_node == NULL); \
			MEMCLASS_ATOMIC_MEM_USED_ADD(mp_node, (sz)); \
		} while(0)

#define _MEMCLASS_PID_FREE(p, mc, sz) \
		do { \
			mempart_node_t *mp_node; \
			CRASHCHECK((p) == NULL); \
			mp_node = MEMPART_NODEGET((p), (mc)); \
			CRASHCHECK(mp_node == NULL); \
			MEMCLASS_ATOMIC_MEM_USED_SUB(mp_node, (sz)); \
		} while(0)

#if ENABLE_MEMCLASS_PID_USE
#ifndef NDEBUG
extern void _memclass_pid_use(PROCESS *prp, memclass_id_t mclass_id, memsize_t size);
extern void _memclass_pid_free(PROCESS *prp, memclass_id_t mclass_id, memsize_t size);
#define MEMCLASS_PID_USE(p, mc, sz) \
		do { \
			PROCESS *_prp = (p); \
			memsize_t _size = (sz); \
			if ((_prp != NULL) && (_size > 0)) {_memclass_pid_use(_prp, (mc), _size);} \
		} while(0)
#define MEMCLASS_PID_FREE(p, mc, sz) \
		do { \
			PROCESS *_prp = (p); \
			memsize_t _size = (sz); \
			if ((_prp != NULL) && (_size > 0)) {_memclass_pid_free(_prp, (mc), _size);} \
		} while(0)
#else	/* NDEBUG */
#define MEMCLASS_PID_USE(p, mc, sz) \
		do { \
			PROCESS *_prp = (p); \
			memsize_t _size = (sz); \
			if ((_prp != NULL) && (_size > 0)) {_MEMCLASS_PID_USE(_prp, (mc), _size);} \
		} while(0)
#define MEMCLASS_PID_FREE(p, mc, sz) \
		do { \
			PROCESS *_prp = (p); \
			memsize_t _size = (sz); \
			if ((_prp != NULL) && (_size > 0)) {_MEMCLASS_PID_FREE(_prp, (mc), _size);} \
		} while(0)
#endif	/* NDEBUG */
#else
#define MEMCLASS_PID_USE(p, mc, sz)
#define MEMCLASS_PID_FREE(p, mc, sz)
#endif

/*
 * MEMCLASS_PID_USE_INKER
 * MEMCLASS_PID_FREE_INKER
 * 
 * special (limited use versions) when we are required to enter the kernel in order
 * to satisfy locking requirements around the use of the PROCESS *
 * 
 * We pass in the part_id_t instead of the memclass_id_t so that we can obtain
 * the class_id for the partition AFTER entering the kernel as well. This is to
 * avoid entering the kernel first to obtain the class id in preparation for the
 * call to MemclassPidUse/Free()
*/
#if ENABLE_MEMCLASS_PID_USE
#define MEMCLASS_PID_USE_INKER(_pid, _mpid, sz)		if ((sz) > 0) {MemclassPidUse((_pid), (_mpid), (sz));}
#define MEMCLASS_PID_FREE_INKER(_pid, _mpid, sz)	if ((sz) > 0) {MemclassPidFree((_pid), (_mpid), (sz));}
#else
#define MEMCLASS_PID_USE_INKER(_pid, _mpid, sz)
#define MEMCLASS_PID_FREE_INKER(_pid, _mpid, sz)
#endif


/*
 * MEMCLASS_RUSED_ADJ_UP
 * MEMCLASS_RUSED_ADJ_DN
 * 
 * Adjust the amount of memory accounted as reserved by the allocator for the
 * memory class for which <mp> is a partition of. The reserved memory in use is
 * either increased (MEMCLASS_RUSED_ADJ_UP) or descreased (MEMCLASS_RUSED_ADJ_DN).
 * The adjustment does not affect the amount of memory configured as reserved
 * but rather the amount of reserved memory that is currently accounted as in use.
 * 
 * The result (amount reserved + amount unreserved) will remain constant. That is,
 * an increase in the reserved in use will result in a equal descrease in the
 * unreserved in use and vice versa.
 * 
 * Returns: nothing
*/
#define MEMCLASS_RUSED_ADJ_UP(p, s) \
		do { \
			if ((p)->memclass->data.allocator.resv_adjust != NULL) \
				((p)->memclass->data.allocator.resv_adjust)((s), +1, &((p)->memclass->data.info)); \
		} while(0)

#define MEMCLASS_RUSED_ADJ_DN(p, s) \
		do { \
			if ((p)->memclass->data.allocator.resv_adjust != NULL) \
				((p)->memclass->data.allocator.resv_adjust)((s), -1, &((p)->memclass->data.info)); \
		} while(0)


/*
 * MEMCLASSMGR_EVENT
 * 
 * This API is used to send event notifications to any registered process. Event
 * delivery is handled entirely from the resource manager.
 * 
 * Mandatory arguments include the mempart_t.resmgr_attr_p and the event type
 * (mempart_evttype_t). Depending on the event type (etype), additional event
 * specific arguments may be provided.
 * 
 * If the resource manager has elected not to register an event handler, this
 * API is a no-op.
 * 
 * Returns: nothing 
*/
#ifdef MEMPART_NO_EVENTS
#define MEMCLASSMGR_EVENT(mc, etype, c, p)	{}
#else	/* MEMPART_NO_EVENTS */
#define MEMCLASSMGR_EVENT(mc, etype, c, p) \
		do { \
			if ((memclass_event != NULL) && ((mc) != NULL)) \
				memclass_event((mc)->rmgr_attr_p, (etype) , (c), (p)); \
		} while(0)
#endif	/* MEMPART_NO_EVENTS */
		
#endif	/* _MCLASS_H_ */

/* __SRCVERSION("$IQ: memclass.h,v 1.91 $"); */
