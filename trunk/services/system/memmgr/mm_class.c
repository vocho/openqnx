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

/*******************************************************************************
 * mm_class.c
 * 
 * This file contains code pertaining to the maintenance and manipulation of
 * memory classes within the system. It is theorectically separate from memory
 * partitioning but is required by the memory partitioning implementation hence
 * why it has its own file. It is bundles with the memory partitioning module
 * for now
 * 
*/

#include "vmm.h"

/*
 * memclass_vector
 * 
 * Pointer to the memory class vector
 * 
 * A memory class must be added to the 'memclass_vector' in order for it to be
 * partitioned.
*/
static VECTOR memclass_vector;

static pthread_mutex_t		memclass_list_lock = PTHREAD_MUTEX_INITIALIZER;

/* default allocator access functions */
static void memclass_resv_adjust(memsize_t size, int sign, memclass_info_t *info);
static memsize_t memclass_unreserve(memsize_t size, memsize_t adjust, int adjust_dir, memclass_info_t *info);
static memsize_t memclass_reserve(memsize_t size, memsize_t adjust, int adjust_dir, memclass_info_t *info);
static memclass_sizeinfo_t *memclass_get_sizeinfo(memclass_sizeinfo_t *s, memclass_info_t *info);

static allocator_accessfncs_t  _dflt_memclass_fncs =
{
	STRUCT_FLD(reserve) memclass_reserve,
	STRUCT_FLD(unreserve) memclass_unreserve,
	STRUCT_FLD(size_info) memclass_get_sizeinfo,
	STRUCT_FLD(resv_adjust) memclass_resv_adjust,
};
allocator_accessfncs_t *dflt_memclass_fncs = &_dflt_memclass_fncs;

/*
 * MUTEX_INIT
 * MUTEX_DESTROY
 * MUTEX_LOCK
 * MUTEX_UNLOCK
*/
#define MUTEX_INIT(p, m) \
		do { \
			/* use a recursive mutex */ \
			const pthread_mutex_t  tmp = PTHREAD_RMUTEX_INITIALIZER; \
			(m)->mutex = tmp; \
			(m)->spin.value = 0; \
		} while(0)

#define MUTEX_DESTROY(m)		pthread_mutex_destroy((m));

#define MUTEX_LOCK(m) \
		do { \
			int r = EOK; \
/*			CRASHCHECK(KerextAmInKernel()); */\
			if (!KerextAmInKernel()) \
				r = pthread_mutex_lock((m)); \
			CRASHCHECK(r != EOK); \
		} while(0)

#define MUTEX_UNLOCK(m) \
		do { \
			int r = EOK; \
/*			CRASHCHECK(KerextAmInKernel()); */\
			if (!KerextAmInKernel()) \
				r = pthread_mutex_unlock((m)); \
			CRASHCHECK(r != EOK); \
		} while(0)

/*
 * memclass_event
 * 
 * This function pointer will be initialized when the memory partitioning resource
 * manager module is installed.
*/

void (*memclass_event)(struct apmmgr_attr_s *attr, memclass_evttype_t evtype,
						memclass_sizeinfo_t *cur, memclass_sizeinfo_t *prev) = NULL;

/*******************************************************************************
 * memclass_find
 * 
 * This function will locate the memory class specified either by name or
 * memory class identifier and return a pointer to the memory class entry.
 * One of either the class name or identifier must be specified.
 * This function allows a specific entry to be obtained as follows. 
 * 	if <id> != memclass_id_t_INVALID, it will be used to search for a match and
 *  <name> will be ignored
 * 	if <id> == memclass_id_t_INVALID and <name != NULL>, <name> will be used to
 *  search for a match otherwise, NULL will be returned
 * 
 * Returns: (memclass_entry_t *) or NULL if not found or invalid arguments 
 * 
*/
memclass_entry_t *memclass_find(const char *name, memclass_id_t id)
{
	memclass_entry_t *entry = NULL;

	MUTEX_LOCK(&memclass_list_lock);
	if (id != memclass_id_t_INVALID) {
		entry = vector_lookup(&memclass_vector, id);
	}
	else if ((name != NULL) && (*name != '\0'))
	{
		/* search */
		unsigned i;
		for (i=0; i<memclass_vector.nentries; i++)
		{
			entry = vector_lookup(&memclass_vector, i);
			if ((entry != NULL) && (strcmp(name, entry->name) == 0)) break;
		}		
	}
	MUTEX_UNLOCK(&memclass_list_lock);
	CRASHCHECK((entry != NULL) && (entry->data.signature != MEMCLASS_SIGNATURE));
	return entry;
}


/*******************************************************************************
 * memclass_add
 * 
 * add the memory class identified by <memclass_name> to the list of known
 * memory types and return a memory class identifier for the new memory. The
 * attributes of the memory class are specified in <attr> and a set of allocator
 * functions are specified in <f>
 * 
 * If unsuccessful, memclass_id_t_INVALID will be returned.
 * 
 * If the memory class is already known, then the 'memclass_id_t' for the
 * previously added memory type is returned.
 * 
 * IMPORTANT
 * The memory for the class structures always comes from the internal heap
 * since it may need to live beyond the existence of the creating process
*/
memclass_id_t memclass_add(const char *memclass_name, memclass_attr_t *attr,
									allocator_accessfncs_t *f)
{
	memclass_entry_t  *mclass_entry;
	int					id;

	/* first make sure that the memory class has not already been added */
	if ((mclass_entry = memclass_find(memclass_name, memclass_id_t_INVALID)) != NULL)
	{
#ifndef NDEBUG
		if (ker_verbose > 1)
		{
			kprintf("memory class '%s' with class id 0x%x already present\n",
					mclass_entry->name, mclass_entry->data.info.id);
		}
#endif	/* NDEBUG */
	}
	/* not found, add a new entry */
	else if ((mclass_entry = calloc(1, sizeof(*mclass_entry))) == NULL)
		return memclass_id_t_INVALID;
	else
	{
		mclass_entry->data.signature = MEMCLASS_SIGNATURE;
		STRLCPY(mclass_entry->name, memclass_name, sizeof(mclass_entry->name));
		MUTEX_INIT(NULL, &mclass_entry->lock);

		if (attr)
			mclass_entry->data.info.attr = *attr;
		else
		{
			memclass_attr_t  a = MEMCLASS_DEFAULT_ATTR;
			mclass_entry->data.info.attr = a;
		}
		if (f != NULL) {
			mclass_entry->data.allocator = *f;
		}
		else {
			mclass_entry->data.allocator = _dflt_memclass_fncs;
		}

		memset(&mclass_entry->data.info.size, 0, sizeof(mclass_entry->data.info.size));
		mclass_entry->data.info.size.unreserved.free = mclass_entry->data.info.attr.size;

#ifndef NDEBUG
		if (ker_verbose > 1)
		{
			kprintf("added memory class '%s' with class id 0x%x, sz = %P\n",
						mclass_entry->name, mclass_entry->data.info.id,
						(paddr_t)mclass_entry->data.info.size.unreserved.free);
		}
#endif	/* NDEBUG */

		/* add the entry to the list */
		MUTEX_LOCK(&memclass_list_lock);
		id = vector_add(&memclass_vector, mclass_entry, 0);
		if(id < 0) {
			MUTEX_UNLOCK(&memclass_list_lock);
			free(mclass_entry);
			return memclass_id_t_INVALID;
		}
		mclass_entry->data.info.id = id;
		MUTEX_UNLOCK(&memclass_list_lock);
	}
	return mclass_entry->data.info.id;
}


/*******************************************************************************
 * memclass_del
 * 
 *
 * This function will remove the memory class specified by either name or
 * memclass identifier.
 * 
 * Note that although memory class id's are used to index the prp->mpart_list,
 * it is impossible to delete a memory class which has been partitioned.
 * At this point, all partitions of the memory class are gone and so any
 * reference to the corresponding memclass_id_t will also be gone. 
 * 
 * Returns: EOK on success, otherwise an errno
 * 
*/
int memclass_delete(const char *name, memclass_id_t id)
{
	memclass_entry_t  *entry;

	if ((name == NULL) && (id == memclass_id_t_INVALID))
		return EINVAL;
	
	if ((entry = memclass_find(name, id)) == NULL)
		return ENOENT;
	
	MUTEX_LOCK(&memclass_list_lock);
	vector_rem(&memclass_vector, entry->data.info.id);
	MUTEX_UNLOCK(&memclass_list_lock);

	MUTEX_DESTROY(&entry->lock.mutex);
	free(entry);

	return EOK;
}

/*******************************************************************************
 * memclass_info
 * 
 * 
*/
memclass_info_t *memclass_info(memclass_id_t id, memclass_info_t *info_buf)
{
	memclass_entry_t *e = memclass_find(NULL, id);

	if (e == NULL) return NULL;
	CRASHCHECK(info_buf == NULL);
	(void)e->data.allocator.size_info(&e->data.info.size, &e->data.info);
	*info_buf = e->data.info;
	return info_buf;
}

/*
 * ===========================================================================
 * 
 * 						Memory Class support routines
 *  
 * ===========================================================================
*/

/*
 * memclass_get_sizeinfo
 * 
 * default access function to obtain the size information for the memory class
*/
static memclass_sizeinfo_t *memclass_get_sizeinfo(memclass_sizeinfo_t *s, memclass_info_t *info)
{
	CRASHCHECK(s == NULL);
	CRASHCHECK(info == NULL);

	*s = info->size;
	return s;
}

/*
 * memclass_reserve
 * 
 * default access function to reserve memory in the allocator
 * 
 * When we reserve memory, we are increasing the amount of reserved memory
 * and correspondingly decreasing the amount of unreserved memory. This is
 * accomplished by increasing the amount of reserved free memory and decreasing
 * the amount of unreserved free memory. There is no adjustment to either the
 * reserved or unreserved used memory (not withstanding an <adjust> > 0 as
 * described below) hence why the the total increases/decreases respectively.
 * We are in effect moving available memory from the unreserved pool to the
 * reserved pool. The amount of memory being reserved is specified in the
 * <size> parameter.
 * 
 * A reservation increase can be accompanied by a reserved used adjustment. This
 * can occur for example, when a partition which has consumed more than its
 * reservation (ie has obtained discretionary (unreserved) memory), and has its
 * configuration modified such that its min size becomes greater. In this case,
 * the memory which was previously accounted against unreserved memory must be
 * adjusted such that it is accounted against reserved memory. In order to ensure
 * that we are mathematically correct, this adjustment is accomplished at the same
 * time as the reservation.
 * Also, because sizes are unsigned values, we also require whether the adjustment
 * is to increase (<adjust_dir> == +1) reserved usage (and consequently decrease
 * unreserved usage) or to decrease (<adjust_dir> == -1) reserved usage (and
 * consequently increase unreserved usage).
 * 
 * The unreserve and the reserve adjustment can be thought of logically as 2
 * separate operations, however they are performed as 1 single operation for
 * the reasons described below.
 * 
*/
static memsize_t memclass_reserve(memsize_t size, memsize_t adjust, int adjust_dir, memclass_info_t *info)
{
	memclass_t *mclass;
	memclass_sizeinfo_t cur, prev = info->size;
	memclass_evttype_t  evt1 = memclass_evttype_t_INVALID;
	memclass_evttype_t  evt2 = memclass_evttype_t_INVALID;

	CRASHCHECK((size == 0) && (adjust == 0));	// what are we doing here ?

	switch(adjust_dir)
	{
		case +1:
			if (size > adjust)
			{
				/* a net reservation increase */
				evt1 = memclass_evttype_t_DELTA_RF_INCR;
				CRASHCHECK((size - adjust) > info->size.unreserved.free);
				info->size.unreserved.free -= (size - adjust);
				info->size.reserved.free += (size - adjust);
			}
			else
			{
				/* a net reservation decrease */
				evt1 = memclass_evttype_t_DELTA_RF_DECR;
				CRASHCHECK((adjust - size) > info->size.reserved.free);
				info->size.unreserved.free += (adjust - size);
				info->size.reserved.free -= (adjust - size);
			}
			CRASHCHECK(adjust > info->size.unreserved.used);
			evt2 = memclass_evttype_t_DELTA_RU_INCR;
			info->size.reserved.used += adjust;
			info->size.unreserved.used -= adjust;
			break;

		case -1:
			/* a net reservation decrease */
			evt1 = memclass_evttype_t_DELTA_RF_INCR;
			evt2 = memclass_evttype_t_DELTA_RU_DECR;

			CRASHCHECK((size + adjust) > info->size.unreserved.free);
			CRASHCHECK(adjust > info->size.reserved.used);
			info->size.unreserved.free -= (size + adjust);
			info->size.reserved.free += (size + adjust);
			info->size.reserved.used -= adjust;
			info->size.unreserved.used += adjust;
			break;
		
		default: crash();
	}
	cur = info->size;
	mclass = (memclass_t *)((unsigned int)info - offsetof(memclass_t, info));
	MEMCLASSMGR_EVENT(mclass, evt1, &cur, &prev);
	if (evt2 != memclass_evttype_t_INVALID) {
		MEMCLASSMGR_EVENT(mclass, evt2, &cur, &prev);
	}
	return size;
}

/*
 * memclass_unreserve
 * 
 * default access function to unreserve memory in the allocator
 * 
 * When we unreserve memory, we are decreasing the amount of reserved memory
 * and correspondingly increasing the amount of unreserved memory. This is
 * accomplished by decreasing the amount of reserved free memory and increasing
 * the amount of unreserved free memory. There is no adjustment to either the
 * reserved or unreserved used memory (not withstanding an <adjust> > 0 as
 * described below) hence why the the total decreases/increases respectively.
 * We are in effect moving available memory from the reserved pool to the
 * unreserved pool. The amount of memory being unreserved is specified in the
 * <size> parameter.
 * 
 * A reservation decrease can be accompanied by a reserved used adjustment. This
 * can occur for example, when a partition which has consumed some or all of its
 * reservation, has its configuration modified such that its min size becomes
 * less than the amount of memory allocated. In this case, some or all of the
 * memory which was previously accounted against reserved memory must be adjusted
 * such that it is accounted against unreserved memory. In order to ensure that
 * we are mathematically correct, this adjustment is accomplished at the same
 * time as the unreservation.
 * Also, because sizes are unsigned values, we also require whether the adjustment
 * is to increase (<adjust_dir> == +1) reserved usage (and consequently decrease
 * unreserved usage) or to decrease (<adjust_dir> == -1) reserved usage (and
 * consequently increase unreserved usage).
 * 
 * The unreserve and the reserve adjustment can be thought of logically as 2
 * separate operations, however they are performed as 1 single operation for
 * the reasons described below.
 * 
*/
static memsize_t memclass_unreserve(memsize_t size, memsize_t adjust, int adjust_dir, memclass_info_t *info)
{
	memclass_t *mclass;
	memclass_sizeinfo_t cur, prev = info->size;
	memclass_evttype_t  evt1 = memclass_evttype_t_INVALID;
	memclass_evttype_t  evt2 = memclass_evttype_t_INVALID;

	CRASHCHECK((size == 0) && (adjust == 0));	// what are we doing here ?

	switch(adjust_dir)
	{
		case -1:
			if (size > adjust)
			{
				/* a net reservation decrease */
				evt1 = memclass_evttype_t_DELTA_RF_DECR;
				CRASHCHECK((size - adjust) > info->size.reserved.free);
				info->size.unreserved.free += (size - adjust);
				info->size.reserved.free -= (size - adjust);
			}
			else
			{
				/* a net reservation increase */
				evt1 = memclass_evttype_t_DELTA_RF_INCR;
				CRASHCHECK((adjust - size) > info->size.unreserved.free);
				info->size.unreserved.free -= (adjust - size);
				info->size.reserved.free += (adjust - size);
			}
			CRASHCHECK(adjust > info->size.reserved.used);
			evt2 = memclass_evttype_t_DELTA_RU_DECR;
			info->size.reserved.used -= adjust;
			info->size.unreserved.used += adjust;
			break;

		case +1:
			/* a net reservation decrease */
			evt1 = memclass_evttype_t_DELTA_RF_DECR;
			evt2 = memclass_evttype_t_DELTA_RU_DECR;

			CRASHCHECK((size + adjust) > info->size.reserved.free);
			CRASHCHECK(adjust > info->size.unreserved.used);
			info->size.unreserved.free += (size + adjust);
			info->size.reserved.free -= (size + adjust);
			info->size.reserved.used += adjust;
			info->size.unreserved.used -= adjust;
			break;
		
		default: crash();
	}
	cur = info->size;
	mclass = (memclass_t *)((unsigned int)info - offsetof(memclass_t, info));
	MEMCLASSMGR_EVENT(mclass, evt1, &cur, &prev);
	if (evt2 != memclass_evttype_t_INVALID) {
		MEMCLASSMGR_EVENT(mclass, evt2, &cur, &prev);
	}
	return size;
}

/*
 * memclass_resv_adjust
 * 
 * default function to make adjustments to the amount of memory accounted as
 * reserved
 * 
 * This function is called to re-account allocated memory from either reserved
 * to unreserved (-1) or unreserved to reserved (+1) 
*/
static void memclass_resv_adjust(memsize_t size, int sign, memclass_info_t *info)
{
	memclass_t *mclass;
	memclass_sizeinfo_t cur, prev = info->size;
	memclass_evttype_t evt = memclass_evttype_t_INVALID;

	if (size == 0)
		return;
	else if (sign == +1) {
		/* transfer used unreserved to used reserved (while keeping totals constant) */
		info->size.unreserved.used -= size;
		info->size.reserved.used += size;

		/* ... and fix up free's to keep config constant */
		info->size.unreserved.free += size;
		info->size.reserved.free -= size;
		evt = memclass_evttype_t_DELTA_RU_DECR;
	}
	else if (sign == -1) {
		/* transfer used reserved to used unreserved (while keeping totals constant) */
		info->size.reserved.used -= size;
		info->size.unreserved.used += size;

		/* ... and fix up free's to keep config constant */
		info->size.reserved.free += size;
		info->size.unreserved.free -= size;
		evt = memclass_evttype_t_DELTA_RU_INCR;
	}
#ifndef NDEBUG
	else crash();
#endif	/* NDEBUG */

	cur = info->size;
	mclass = (memclass_t *)((unsigned int)info - offsetof(memclass_t, info));
	MEMCLASSMGR_EVENT(mclass, evt, &cur, &prev);
}

#ifndef NDEBUG
/* implement real, gdb breakable functions */
void _memclass_pid_use(PROCESS *prp, memclass_id_t mclass_id, memsize_t size)
{
	_MEMCLASS_PID_USE(prp, mclass_id, size);
}

void _memclass_pid_free(PROCESS *prp, memclass_id_t mclass_id, memsize_t size)
{
	_MEMCLASS_PID_FREE(prp, mclass_id, size);
}
#endif	/* NDEBUG */


__SRCVERSION("mm_class.c $Rev$");
