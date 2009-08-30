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

/*==============================================================================
 * 
 * apmmgr_event
 * 
 * When installed, this routine is called into from the memory partition
 * module to deliver events to any registered process.
 * This routine will be called with the partition that the event is associated
 * with <mp>, the event type (evtype> and (depending on the event type) optional
 * event specific data.
 * 
 * Event processing is kept out of the memory partitioning module and is
 * implemented entirely within the resource manager.
 * 
 * Returns: nothing
*/

#include "apmmgr.h"
#include <kernel/event.h>

//#define DISPLAY_EVENT_MSG

#if !defined(NDEBUG) || defined(__WATCOMC__)
#define INLINE
#else	/* NDEBUG */
#undef DISPLAY_EVENT_MSG
#define INLINE	__inline__
#endif	/* NDEBUG */

#if !defined(NDEBUG) && defined(DISPLAY_EVENT_MSG)
static char *mclass_evt_string(memclass_evttype_t evt);
#endif	/* NDEBUG */

#define ORPHAN_RETRY_COUNT	5

/*
 * =============================================================================
 * 
 * 						S U P P O R T   R O U T I N E S
 * 
 * 				These routines are first to allow them to be inlined
 * 
 * =============================================================================
*/

#ifndef NDEBUG
#define MATCH_FUNC_ASSERT(v1, v2) \
		do { \
			if (v1 == NULL) crash(); \
			if (v2 == NULL) crash(); \
		} while(0)
#else	/* NDEBUG */
#define MATCH_FUNC_ASSERT(v1, v2)
#endif	/* NDEBUG */

#define MATCH_FUNC(val1, val2, type)	(*((type *)val1) == *((type *)val2))

/*
 * pid_match
 * 
 * compare 2 values as process identifier (pid_t) types and return whether
 * they are equivalent
*/
static INLINE bool pid_match(void **match, void **val)
{
	pid_t null_pid = 0;
	MATCH_FUNC_ASSERT(match, val);
	/* if the caller provided a pid=0, this means match any process id */
	return (MATCH_FUNC(match, &null_pid, pid_t) || MATCH_FUNC(match, val, pid_t));
}

/*
 * mpid_match
 * 
 * compare 2 values as memory partition identifier (part_id_t) types and
 * return whether they are equivalent
*/
static INLINE bool mpid_match(void **match, void **val)
{
	part_id_t null_mpid = 0;
	MATCH_FUNC_ASSERT(match, val);
	/* if the caller provided an mpid=0, this means match any partition id */
	return (MATCH_FUNC(match, &null_mpid, part_id_t) || MATCH_FUNC(match, val, part_id_t));
}

/*
 * oid_match
 * 
 * compare 2 values as object identifier (void *) types and return whether they
 * are equivalent
*/
static INLINE bool oid_match(void **match, void **val)
{
	void *null_oid = 0;
	MATCH_FUNC_ASSERT(match, val);
	/* if the caller provided an oid=0, this menas match any object id */
	return (MATCH_FUNC(match, &null_oid, void *) || MATCH_FUNC(match, val, void *));
}

/*
 * delta_decr_check
 * 
 * perform the applicable delta decrement calculation and return TRUE if the
 * event send conditions are satisfied, otherwise return FALSE
*/
static INLINE bool delta_decr_check(memsize_t delta, memsize_t last_sz, memsize_t cur_sz)
{
	return (cur_sz < last_sz) && ((last_sz - cur_sz) >= delta);
}

/*
 * delta_incr_check
 * 
 * perform the applicable delta increment calculation and return TRUE if the
 * event send conditions are satisfied, otherwise return FALSE
*/
static INLINE bool delta_incr_check(memsize_t delta, memsize_t last_sz, memsize_t cur_sz)
{
	return (cur_sz > last_sz) && ((cur_sz - last_sz) >= delta);
}

/*
 * threshold_X_over_check
 * 
 * perform the applicable threshold cross over calculation and return TRUE if the
 * event send conditions are satisfied, otherwise return FALSE
*/
static INLINE bool threshold_X_over_check(memsize_t threshold, memsize_t cur_sz, memsize_t prev_sz)
{
	return (threshold > prev_sz) && (threshold <= cur_sz);
}

/*
 * threshold_X_under_check
 * 
 * perform the applicable threshold cross over calculation and return TRUE if the
 * event send conditions are satisfied, otherwise return FALSE
*/
static INLINE bool threshold_X_under_check(memsize_t threshold, memsize_t cur_sz, memsize_t prev_sz)
{
	return (threshold < prev_sz) && (threshold >= cur_sz);
}

/*
 * initial_delta_conditions_check
 * 
 * initialization routine for delta events.
 * This routine will update the event->evt_data.size value so that the delivery
 * conditions for a delta change event can be properly evaluated. 
 * A value of 'memsize_t_INFINITY' for the event->evt_data.size is used to
 * trigger the update when a delta event is initially registered.
*/
static INLINE void initial_delta_conditions_check(mempart_evt_t *event, memsize_t prev_size, void *cmp)
{
	if (cmp == (void *)delta_incr_check)
	{
		/*
		 * If 'prev_size' is less than the currently recorded size since the
		 * last event, we know we have decremented and we must reset the start point
		 * If event->evt_data.size == memsize_t_INFINITY from initial event
		 * registration, give it a real value
		*/
		if ((prev_size < event->evt_data.size) || (event->evt_data.size == (memsize_t)memsize_t_INFINITY)) {
			event->evt_data.size = prev_size;
		}
	}
	else if (cmp == (void *)delta_decr_check)
	{
		/*
		 * If 'prev_size' is less than the currently recorded size since the
		 * last event, we know we have decremented and we must reset the start point
		 * If event->evt_data.size == memsize_t_INFINITY from initial event
		 * registration, give it a real value
		*/
		if ((prev_size > event->evt_data.size) || (event->evt_data.size == (memsize_t)memsize_t_INFINITY)) {
			event->evt_data.size = prev_size;
		}
	}
}

#define SEND_MC_DELTA_INCR_EVENTS(evl, l_sz, c_sz) send_delta_events((evl), (l_sz), (c_sz), delta_incr_check)
#define SEND_MC_DELTA_DECR_EVENTS(evl, l_sz, c_sz) send_delta_events((evl), (l_sz), (c_sz), delta_decr_check)
#define SEND_MP_DELTA_INCR_EVENTS(evl, l_sz, c_sz) send_delta_events((evl), (l_sz), (c_sz), delta_incr_check)
#define SEND_MP_DELTA_DECR_EVENTS(evl, l_sz, c_sz) send_delta_events((evl), (l_sz), (c_sz), delta_decr_check)

#define SEND_MC_THRESHOLD_X_OVER_EVENTS(evl, c_sz, p_sz) send_threshold_X_events((evl), (c_sz), (p_sz), threshold_X_over_check)
#define SEND_MC_THRESHOLD_X_UNDER_EVENTS(evl, c_sz, p_sz) send_threshold_X_events((evl), (c_sz), (p_sz), threshold_X_under_check)
#define SEND_MP_THRESHOLD_X_OVER_EVENTS(evl, c_sz, p_sz) send_threshold_X_events((evl), (c_sz), (p_sz), threshold_X_over_check)
#define SEND_MP_THRESHOLD_X_UNDER_EVENTS(evl, c_sz, p_sz) send_threshold_X_events((evl), (c_sz), (p_sz), threshold_X_under_check)

#define SEND_MP_EVENT(evl, v, f) send_event((evl), (v), (f))
#define SEND_MC_EVENT(evl, v) send_event((evl), (v), NULL)

typedef struct
{
	mempart_evt_t **prev_active;
	mempart_evt_t  *prev_event;
} elistdata_t;
/*
 * get_first_event
 * 
 * Safely get the first event to send from 'evlist'. This function handles the
 * case where an event has transitioned from the active to the inactive list and
 * continues to process all events in the active list
*/
static INLINE mempart_evt_t *get_first_event(part_evtlist_t *evlist, elistdata_t *eld_p)
{
	mempart_evt_t *event;

	CRASHCHECK(eld_p == NULL);

	INTR_LOCK(&evlist->active.lock);
	event = (mempart_evt_t *)LIST_FIRST(evlist->active.list);
	eld_p->prev_active = (mempart_evt_t **)&evlist->active.list.head;
	eld_p->prev_event = NULL;
	if (event != NULL) ++event->inuse;
	INTR_UNLOCK(&evlist->active.lock);
	return event;
}

/*
 * get_next_event
 * 
 * Safely get the next event to send from 'evlist'. This function handles the
 * case where an event has transitioned from the active to the inactive list and
 * continues to process all events in the active list
*/
static INLINE mempart_evt_t *get_next_event(part_evtlist_t *evlist, mempart_evt_t *event, elistdata_t *eld_p)
{
	INTR_LOCK(&evlist->active.lock);

	CRASHCHECK(event->inuse == 0);

	/* Done with both the current and previous nodes */
	--event->inuse;
	if (eld_p->prev_event != NULL) --eld_p->prev_event->inuse;

	if (event->onlist == &evlist->active)
	{
		/* just get the next event on the active list */
		eld_p->prev_event = event;
		event = (mempart_evt_t *)LIST_NEXT(event);
		if (event != NULL)
		{
			++eld_p->prev_event->inuse;
			++event->inuse;
			eld_p->prev_active = (mempart_evt_t **)LIST_PREV(event);
		}
	}
	else
	{
		/*
		 * here we handle the case where 'event' was moved from the active
		 * list to the inactive list during delivery. If the event is not on
		 * the active list, this is what happened and so we look at the
		 * 'eld_p->prev_active' (which was on the active list, it may not be now)
		 * and possibly use what it points to for the next 'event'
		 * NOTE: since 'event' is no longer on the active list, **prev_active
		 * 		 points to 'event->next' iff (**prev_active) is still on the
		 * 		 active list
		*/
		if ((event = (*eld_p->prev_active)) != NULL)
		{
			/* if our previous active transitioned off the active list there is
			 * no way to continue with the active list, so return NULL */
			if (event->onlist != &evlist->active) event = NULL;
			else
			{
				eld_p->prev_active = (mempart_evt_t **)LIST_PREV(event);
				if (eld_p->prev_active == (mempart_evt_t **)&evlist->active.list.head)
					eld_p->prev_event = NULL;
				else
					eld_p->prev_event = *((mempart_evt_t **)LIST_PREV(event));
				++event->inuse;
				if (eld_p->prev_event != NULL) ++eld_p->prev_event->inuse;
			}
		}
	}
	INTR_UNLOCK(&evlist->active.lock);
	return event;
}

/*
 * send_threshold_X_events
 * 
 * Send an event to all listeners in list <evlist> for which
 * <ev>->_type.threshold_cross is either > or < the registered 'threshold' value
 * 
 * Note that in order to meet the conditions for a threshold crossing OVER, the
 * registered event crossing 'over' value must be greater than <prev_size> and
 * less than <ev>->_type.threshold_cross (ie. it must be between the
 * previous and the current values). If the previous current value was already
 * over the registered 'over' value, the threshold is not crossed and no event
 * will be sent.
 * 
 * Similarly, in order to meet the conditions for a threshold crossing UNDER,
 * the registered event crossing 'under' value must be less than <prev_size> and
 * greater than or equal to <ev>->_type.threshold_cross (ie it must be
 * between the previous and the current values). If the previous current value
 * was already under the registered 'under' value, the threshold is not crossed
 * and no event will be sent.
 * 
 * By default, events are removed from the event list once they have been
 * delivered. This behavour is controlled with the xxxx_evtflags_t_REARM flag.
 * If set, the event is not removed from the event list and will be sent
 * whenever the event delivery condition is met.
 * 
*/
#define GET_THRESHOLD_X_P(_ei_) \
		(((_ei_)->class == evtclass_t_MEMCLASS) ? &(_ei_)->info.mc.val.threshold_cross : \
			((_ei_)->class == evtclass_t_MEMPART) ? &(_ei_)->info.mp.val.threshold_cross : NULL)

static INLINE void
send_threshold_X_events(part_evtlist_t *evlist, memsize_t cur_size, memsize_t prev_size,
						bool (*compare_func)(memsize_t threshold, memsize_t cur_sz, memsize_t prev_sz))
{
	elistdata_t  eld_p;
	mempart_evt_t  *reg_event = get_first_event(evlist, &eld_p);

	while (reg_event != NULL)
	{
		struct evt_info_s *evt = &reg_event->evt_reg;

		if (reg_event->onlist == &evlist->active)
		{
			memsize_t *threshold_x_p = GET_THRESHOLD_X_P(evt);
			CRASHCHECK(threshold_x_p == NULL);

			/* if this event has been deactivated, don't send it */
			if (evt->flags & evtflags_DEACTIVATE) {
				deactivate_event(evlist, (part_evt_t *)reg_event);
			}
			/* check for event send conditions */
			else if ((reg_event->evt_dest.rcvid > 0) &&
					 (compare_func != NULL) &&
					 (compare_func(*threshold_x_p, cur_size, prev_size) == bool_t_TRUE))
			{
				int r;
				struct sigevent se = evt->sig;		// make a local copy
				/*
				 * if the caller has set evtflags_SIGEV_FLAG_SIGINFO, we
				 * can return some useful information
				*/
				if (evt->flags & evtflags_SIGEV_FLAG_SIGINFO)
				{
					/*
					 * if the 'cur_size' is too large to fit in the sival_int field
					 * then send back UINT_MAX. This otherwise illegal size value
					 * (because UINT_MAX & (__PAGESIZE - 1) != 0) will be an indication
					 * to the event receiver that o information was provided and
					 * that they need to request the current size if they need it
					*/
					if (cur_size > UINT_MAX) {
						se.sigev_value.sival_int = UINT_MAX;
					} else {
						se.sigev_value.sival_int = (_Uint32t)cur_size;
					}
				}
				r = apm_deliver_event(&reg_event->evt_dest, &se);

				if (r == EOK) reg_event->undeliverable_count = 0;
				else if (r == ESRCH) ++reg_event->undeliverable_count;

				if ((reg_event->undeliverable_count > ORPHAN_RETRY_COUNT) ||
					(!(evt->flags & evtflags_REARM)))
				{
					/* remove the event from the list */
					deactivate_event(evlist, (part_evt_t *)reg_event);
				}
			}
		}
		reg_event = get_next_event(evlist, reg_event, &eld_p);
	}
}

/*
 * send_delta_events
 * 
 * Send an event to all listeners in list <evlist> for which the delta between
 * the current size value and the size at the time the last event was sent
 * is >= the registered 'delta' value on increment or decrement
 * 
 * By default, events are removed from the event list once they have been
 * delivered. This behavour is controlled with the xxxx_evtflags_t_REARM flag.
 * If set, the event is not removed from the event list and will be sent
 * whenever the event delivery condition is met.
*/
#define GET_DELTA_P(_ei_) \
		(((_ei_)->class == evtclass_t_MEMCLASS) ? &(_ei_)->info.mc.val.delta : \
			((_ei_)->class == evtclass_t_MEMPART) ? &(_ei_)->info.mp.val.delta : NULL)

static INLINE void
send_delta_events(part_evtlist_t *evlist, memsize_t cur_size, memsize_t prev_size,
					bool (*compare_func)(memsize_t delta, memsize_t last_sz, memsize_t cur_sz))
{
	elistdata_t  eld_p;
	mempart_evt_t  *reg_event = get_first_event(evlist, &eld_p);

	while (reg_event != NULL)
	{
		struct evt_info_s *evt = &reg_event->evt_reg;

		if (reg_event->onlist == &evlist->active)
		{
			memsize_t *delta_p = GET_DELTA_P(evt);
			CRASHCHECK(delta_p == NULL);

			initial_delta_conditions_check(reg_event, prev_size, (void *)compare_func);

			/* if this event has been deactivated, don't send it */
			if (evt->flags & evtflags_DEACTIVATE) {
				deactivate_event(evlist, (part_evt_t *)reg_event);
			}
			/* check for event send conditions */
			else if ((reg_event->evt_dest.rcvid > 0) &&
					 ((compare_func == NULL) ||
					  (compare_func(*delta_p, reg_event->evt_data.size, cur_size) == bool_t_TRUE)))
			{
				int r;
				struct sigevent se = evt->sig;		// make a local copy
				/*
				 * if the caller has set evtflags_SIGEV_FLAG_SIGINFO, we
				 * can return some useful information
				*/
				if (evt->flags & evtflags_SIGEV_FLAG_SIGINFO)
				{
					/*
					 * if the 'cur_size' is too large to fit in the sival_int field
					 * then send back UINT_MAX. This otherwise illegal size value
					 * (because UINT_MAX & (__PAGESIZE - 1) != 0) will be an indication
					 * to the event receiver that o information was provided and
					 * that they need to request the current size if they need it
					*/
					if (cur_size > UINT_MAX) {
						se.sigev_value.sival_int = UINT_MAX;
					} else {
						se.sigev_value.sival_int = (_Uint32t)cur_size;
					}
				}
				r = apm_deliver_event(&reg_event->evt_dest, &se);

				if (r == EOK) reg_event->undeliverable_count = 0;
				else if (r == ESRCH) ++reg_event->undeliverable_count;

				if ((reg_event->undeliverable_count > ORPHAN_RETRY_COUNT) ||
					(!(evt->flags & evtflags_REARM)))
				{
					/* remove the event from the list */
					deactivate_event(evlist, (part_evt_t *)reg_event);
				}
				else {
					reg_event->evt_data.size = cur_size;
				}
			}
		}
		reg_event = get_next_event(evlist, reg_event, &eld_p);
	}
}

/*
 * send_event
 * 
 * Unconditionally send event data <ev> to all listeners in list <evlist>.
 * 
 * By default, events are removed from the event list once they have been
 * delivered. This behavour is controlled with the xxxx_evtflags_t_REARM flag.
 * If set, the event is not removed from the event list and will be sent
 * whenever the event delivery condition is met.
*/
#define GET_COMPARE_VAL_P(_ei_) \
		(((_ei_)->class == evtclass_t_MEMCLASS) ? (void **)&(_ei_)->info.mc.val : \
			((_ei_)->class == evtclass_t_MEMPART) ? (void **)&(_ei_)->info.mp.val : NULL)

static INLINE void send_event(part_evtlist_t *evlist, void **val, bool (*match_func)(void **a, void **b))
{
	elistdata_t  eld_p;
	mempart_evt_t  *reg_event = get_first_event(evlist, &eld_p);

	while (reg_event != NULL)
	{
		struct evt_info_s *evt = &reg_event->evt_reg;

		if (reg_event->onlist == &evlist->active)
		{
			/* if this event has been deactivated, don't send it */
			if (evt->flags & evtflags_DEACTIVATE) {
				deactivate_event(evlist, (part_evt_t *)reg_event);
			}
			/* check for event send conditions */
			else if (reg_event->evt_dest.rcvid > 0)
			{
				void **compare_val_p = GET_COMPARE_VAL_P(evt);
				CRASHCHECK(compare_val_p == NULL);

				/*
				 * if the caller did not provide a match function, or a match
				 * function was provided and it returns TRUE, deliver the event
				*/
				if ((match_func == NULL) ||
					(match_func(compare_val_p, val) == bool_t_TRUE))
				{
					int r;
					struct sigevent se = evt->sig;		// make a local copy
					/*
					 * if the caller has set evtflags_SIGEV_FLAG_SIGINFO, we
					 * can return some useful information
					*/
					if (evt->flags & evtflags_SIGEV_FLAG_SIGINFO) {
						se.sigev_value.sival_ptr = *val;
					}
					r = apm_deliver_event(&reg_event->evt_dest, &se);

					if (r == EOK) reg_event->undeliverable_count = 0;
					else if (r == ESRCH) ++reg_event->undeliverable_count;

					if ((reg_event->undeliverable_count > ORPHAN_RETRY_COUNT) ||
						(!(evt->flags & evtflags_REARM)))
					{
						/* remove the event from the list */
						deactivate_event(evlist, (part_evt_t *)reg_event);
					}
				}
			}
		}
		reg_event = get_next_event(evlist, reg_event, &eld_p);
	}
}


/*
 * _apmmgr_event_trigger
 * 
 * This is the routine which is made available externally to the memory
 * partitioning module and performs initial event processing
*/
static void _apmmgr_event_trigger(apmmgr_attr_t *mpart, memclass_evttype_t evtype,
										memsize_t cur_size, memsize_t prev_size)
{
	part_evtlist_t  *evt_list;

	/*
	 * it is possible that there is no apmmgr_attr_t yet even though there
	 * is an mempart_t (ie. mempart_t.resmgr_attr_p == NULL)
	*/
	if (mpart == NULL)
		return;

	/* check to see if there are any events of this type registered */
	evt_list = &mpart->event_list[MEMPART_EVTYPE_IDX(evtype)];
	if (LIST_COUNT(evt_list->active.list) == 0) return;

	switch (evtype)
	{
		case mempart_evttype_t_THRESHOLD_CROSS_OVER:
		case mempart_evttype_t_THRESHOLD_CROSS_UNDER:
		{
#ifdef DISPLAY_EVENT_MSG
			kprintf("THRESHOLD_CROSS %s: mp %p (%s) from %P to %P\n",
					(cur_size > prev_size) ? "over" : ((cur_size < prev_size) ? "under" : "?"),
					mpart, mpart->name, (paddr_t)prev_size, (paddr_t)cur_size);
#endif	/* DISPLAY_EVENT_MSG */
			if (cur_size > prev_size) {
				SEND_MP_THRESHOLD_X_OVER_EVENTS(evt_list, cur_size, prev_size);
			} else if (cur_size < prev_size) {
				SEND_MP_THRESHOLD_X_UNDER_EVENTS(evt_list, cur_size, prev_size);
			}
#ifndef NDEBUG
			else crash();
#endif	/* NDEBUG */
			break;
		}

		case mempart_evttype_t_DELTA_INCR:
		case mempart_evttype_t_DELTA_DECR:
		{
#ifdef DISPLAY_EVENT_MSG
			kprintf("DELTA %s: mp %p (%s) from %P to %P\n",
					(cur_size > prev_size) ? "incr" : ((cur_size < prev_size) ? "decr" : "?"),
					mpart, mpart->name, (paddr_t)prev_size, (paddr_t)cur_size);
#endif	/* DISPLAY_EVENT_MSG */
			if (cur_size > prev_size) {
				SEND_MP_DELTA_INCR_EVENTS(evt_list, cur_size, prev_size);
			} else if (cur_size < prev_size) {
				SEND_MP_DELTA_DECR_EVENTS(evt_list, cur_size, prev_size);
			}
#ifndef NDEBUG
			else crash();
#endif	/* NDEBUG */
			break;
		}

		default:
#ifdef DISPLAY_EVENT_MSG
			kprintf("Unknown event: %d\n", evtype);
#endif	/* DISPLAY_EVENT_MSG */
			break;
	}
}

/*
 * __apmmgr_event_trigger2
 * 
 * This routine will handle all of the possible memory class DELTA and
 * THRESHOLD_CROSS events. It is only called from _apmmgr_event_trigger2()
*/
static INLINE void __apmmgr_event_trigger2(apmmgr_attr_t *mpart, memclass_evttype_t evtype,
										memsize_t cur_size, memsize_t prev_size)
{
	part_evtlist_t  *evt_list;

	/* check to see if there are any events of this type registered */
	evt_list = &mpart->event_list[MEMCLASS_EVTYPE_IDX(evtype)];
	if (LIST_COUNT(evt_list->active.list) == 0) return;

	switch (evtype)
	{
		/* handle all the deltas */
		case memclass_evttype_t_DELTA_TF_INCR:
		case memclass_evttype_t_DELTA_TF_DECR:
		case memclass_evttype_t_DELTA_RF_INCR:
		case memclass_evttype_t_DELTA_RF_DECR:
		case memclass_evttype_t_DELTA_UF_INCR:
		case memclass_evttype_t_DELTA_UF_DECR:
		case memclass_evttype_t_DELTA_TU_INCR:
		case memclass_evttype_t_DELTA_TU_DECR:
		case memclass_evttype_t_DELTA_RU_INCR:
		case memclass_evttype_t_DELTA_RU_DECR:
		case memclass_evttype_t_DELTA_UU_INCR:
		case memclass_evttype_t_DELTA_UU_DECR:
		{

#ifdef DISPLAY_EVENT_MSG
			kprintf("%s: mp %p (%s) from %P to %P\n", mclass_evt_string(evtype),
					mpart, mpart->name, (paddr_t)prev_size, (paddr_t)cur_size);
#endif	/* DISPLAY_EVENT_MSG */

			if (cur_size > prev_size) {
				SEND_MC_DELTA_INCR_EVENTS(evt_list, cur_size, prev_size);
			} else if (cur_size < prev_size) {
				SEND_MC_DELTA_DECR_EVENTS(evt_list, cur_size, prev_size);
			}
#ifndef NDEBUG
			else crash();
#endif	/* NDEBUG */
			break;
		}

		case memclass_evttype_t_THRESHOLD_CROSS_TF_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_TF_UNDER:
		case memclass_evttype_t_THRESHOLD_CROSS_TU_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_TU_UNDER:
		case memclass_evttype_t_THRESHOLD_CROSS_RF_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_RF_UNDER:
		case memclass_evttype_t_THRESHOLD_CROSS_UF_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_UF_UNDER:
		case memclass_evttype_t_THRESHOLD_CROSS_RU_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_RU_UNDER:
		case memclass_evttype_t_THRESHOLD_CROSS_UU_OVER:
		case memclass_evttype_t_THRESHOLD_CROSS_UU_UNDER:
		{

#ifdef DISPLAY_EVENT_MSG
			kprintf("%s: mp %p (%s) from %P to %P\n", mclass_evt_string(evtype),
					mpart, mpart->name, (paddr_t)prev_size, (paddr_t)cur_size);
#endif	/* DISPLAY_EVENT_MSG */

			if (cur_size > prev_size) {
				SEND_MC_THRESHOLD_X_OVER_EVENTS(evt_list, cur_size, prev_size);
			} else if (cur_size < prev_size) {
				SEND_MC_THRESHOLD_X_UNDER_EVENTS(evt_list, cur_size, prev_size);
			}
#ifndef NDEBUG
			else crash();
#endif	/* NDEBUG */
			break;
		}

		default:
#ifdef DISPLAY_EVENT_MSG
			kprintf("Unknown event: %d\n", evtype);
#endif	/* DISPLAY_EVENT_MSG */
			break;
	}
}

/*
 * _apmmgr_event_trigger2
 * 
 * This routine will take the individual memory class DELTA events and generate
 * the applicable THRESHOLD_CROSS events. __apmmgr_event_trigger2() is
 * called to actually deliver the event.
 * This routne is only called from the main apmmgr_event_trigger2() function.
 * The only events processed by _apmmgr_event_trigger2() are DELTA events.
*/
static INLINE void _apmmgr_event_trigger2(apmmgr_attr_t *mpart, memclass_evttype_t evtype,
										memsize_t cur_size, memsize_t prev_size)
{
	/* send the delta event */
	__apmmgr_event_trigger2(mpart, evtype, cur_size, prev_size);

	switch (evtype)
	{
		/* handle all the deltas */
		case memclass_evttype_t_DELTA_TF_INCR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_TF_OVER;
			break;
		case memclass_evttype_t_DELTA_TF_DECR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_TF_UNDER;
			break;
		case memclass_evttype_t_DELTA_RF_INCR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_RF_OVER;
			break;
		case memclass_evttype_t_DELTA_RF_DECR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_RF_UNDER;
			break;
		case memclass_evttype_t_DELTA_UF_INCR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_UF_OVER;
			break;
		case memclass_evttype_t_DELTA_UF_DECR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_UF_UNDER;
			break;
		case memclass_evttype_t_DELTA_TU_INCR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_TU_OVER;
			break;
		case memclass_evttype_t_DELTA_TU_DECR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_TU_UNDER;
			break;
		case memclass_evttype_t_DELTA_RU_INCR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_RU_OVER;
			break;
		case memclass_evttype_t_DELTA_RU_DECR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_RU_UNDER;
			break;		
		case memclass_evttype_t_DELTA_UU_INCR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_UU_OVER;
			break;
		case memclass_evttype_t_DELTA_UU_DECR:
			evtype = memclass_evttype_t_THRESHOLD_CROSS_UU_UNDER;
			break;
		default:
#ifndef NDEBUG
			crash();
#endif	/* NDEBUG */
			evtype = memclass_evttype_t_INVALID;
			break;
	}
	
	if (evtype != memclass_evttype_t_INVALID) {
		__apmmgr_event_trigger2(mpart, evtype, cur_size, prev_size);
	}
}

/*
 * =============================================================================
 * 
 * 						E V E N T   A P I   R O U T I N E S
 * 
 * =============================================================================
*/

/*******************************************************************************
 * apmmgr_event_trigger
 * 
 * This is the routine which is made available externally to the memory
 * partitioning module and performs initial event processing
*/
void apmmgr_event_trigger(apmmgr_attr_t *mpart, mempart_evttype_t evtype, ...)
{
	part_evtlist_t  *evt_list;
	va_list  ap;
	va_start(ap, evtype);

// kprintf("%s: mp = %p, evt = %d\n", __func__, mpart, evtype);

	/*
	 * it is possible that there is no apmmgr_attr_t yet even though there
	 * is an mempart_t (ie. mempart_t.resmgr_attr_p == NULL)
	*/
	if (mpart == NULL) return;

	switch (evtype)
	{
		case mempart_evttype_t_DELTA_INCR:
		case mempart_evttype_t_DELTA_DECR:
		{
			/*
			 * additional arguments:
			 * arg1 - current partition size
			 * arg2 - previous partition size (before the increment/decrement
			*/
			memsize_t  cur_size = va_arg(ap, memsize_t);
			memsize_t  prev_size = va_arg(ap, memsize_t);

			/*
			 * Since multiple events may be triggered from the DELTA_INCR/DECR,
			 * the check for registered events will be done in _apmmgr_event_trigger()
			*/
			_apmmgr_event_trigger(mpart, evtype, cur_size, prev_size);
			if (cur_size > prev_size) {
				_apmmgr_event_trigger(mpart, mempart_evttype_t_THRESHOLD_CROSS_OVER, cur_size, prev_size);
			} else if (cur_size < prev_size) {
				_apmmgr_event_trigger(mpart, mempart_evttype_t_THRESHOLD_CROSS_UNDER, cur_size, prev_size);
			}
#ifndef NDEBUG
			else crash();
#endif	/* NDEBUG */
			break;
		}

		case mempart_evttype_t_PROC_ASSOCIATE:
		case mempart_evttype_t_PROC_DISASSOCIATE:
		{
			/* check to see if there are any events of this type registered */
			evt_list = &mpart->event_list[MEMPART_EVTYPE_IDX(evtype)];
			if (LIST_COUNT(evt_list->active.list) > 0)
			{
				/*
				 * addition arguments:
				 * arg1 - pid_t of the associated/disassociated process
				*/
				pid_t  pid = va_arg(ap, pid_t);

#ifdef DISPLAY_EVENT_MSG
			kprintf("PROC_%sASSOCIATE: pid %d %s associated with mp %p (%s)\n",
						(evtype == mempart_evttype_t_PROC_DISASSOCIATE) ? "DIS" : "", pid,
						(evtype == mempart_evttype_t_PROC_DISASSOCIATE) ? "no longer" : "now",
						mpart, mpart->name);
#endif	/* DISPLAY_EVENT_MSG */

				/*
				 * for disassociation events, we optionally check whether a
				 * specific process id of interest has been provided
				*/
				if (evtype == mempart_evttype_t_PROC_DISASSOCIATE) {
					SEND_MP_EVENT(evt_list, (void *)&pid, pid_match);
				} else {
					SEND_MP_EVENT(evt_list, (void *)&pid, NULL);
				}
			}
			break;
		}

		case mempart_evttype_t_CONFIG_CHG_ATTR_MIN:
		{
			/* check to see if there are any events of this type registered */
			evt_list = &mpart->event_list[MEMPART_EVTYPE_IDX(evtype)];
			if (LIST_COUNT(evt_list->active.list) > 0)
			{
				/*
				 * addition arguments:
				 * arg1 - previous configuration (mempart_cfg_t *)
				 * arg2 - current configuration (mempart_cfg_t *)
				 * 
				 * From these args the 'cfg_chg.attr.min' will be built an sent
				 * to any registered processes
				*/
				mempart_cfg_t  *prev = va_arg(ap, mempart_cfg_t *);
				mempart_cfg_t  *cur = va_arg(ap, mempart_cfg_t *);

				if (cur->attr.size.min != prev->attr.size.min)
				{
#ifdef DISPLAY_EVENT_MSG
					kprintf("CONFIG_CHG_ATTR_MIN: mp %p (%s) from %P to %P\n",
							mpart, mpart->name, (paddr_t)prev->attr.size.min,
							(paddr_t)cur->attr.size.min);
#endif	/* DISPLAY_EVENT_MSG */
					SEND_MP_EVENT(evt_list, (void *)&cur->attr.size.min, NULL);
				}
			}
			break;
		}

		case mempart_evttype_t_CONFIG_CHG_ATTR_MAX:
		{
			/* check to see if there are any events of this type registered */
			evt_list = &mpart->event_list[MEMPART_EVTYPE_IDX(evtype)];
			if (LIST_COUNT(evt_list->active.list) > 0)
			{
				/*
				 * addition arguments:
				 * arg1 - previous configuration (mempart_cfg_t *)
				 * arg2 - current configuration (mempart_cfg_t *)
				 * 
				 * From these args the 'cfg_chg.attr.max' will be built an sent
				 * to any registered processes
				*/
				mempart_cfg_t  *prev = va_arg(ap, mempart_cfg_t *);
				mempart_cfg_t  *cur = va_arg(ap, mempart_cfg_t *);

				if (cur->attr.size.max != prev->attr.size.max)
				{
#ifdef DISPLAY_EVENT_MSG
					kprintf("CONFIG_CHG_ATTR_MAX: mp %p (%s) from %P to %P\n",
							mpart, mpart->name, (paddr_t)prev->attr.size.max,
							(paddr_t)cur->attr.size.max);
#endif	/* DISPLAY_EVENT_MSG */
					SEND_MP_EVENT(evt_list, (void *)&cur->attr.size.max, NULL);
				}
			}
			break;
		}

		case mempart_evttype_t_CONFIG_CHG_POLICY:
		{
			/* check to see if there are any events of this type registered */
			evt_list = &mpart->event_list[MEMPART_EVTYPE_IDX(evtype)];
			if (LIST_COUNT(evt_list->active.list) > 0)
			{
				/*
				 * addition arguments:
				 * arg1 - previous configuration (mempart_cfg_t *)
				 * arg2 - current configuration (mempart_cfg_t *)
				 * 
				 * Note that the policies in the previous configuration are munged
				 * and must be converted to boolean values
				 * 
				 * From these args a 'cfg_chg.policy' will be built an sent to any
				 * registered processes
				*/
				union mp_cfg_chg_s  cfgchg;
				mempart_cfg_t  *prev = va_arg(ap, mempart_cfg_t *);
				mempart_cfg_t  *cur = va_arg(ap, mempart_cfg_t *);

				if ((cur->policy.terminal != prev->policy.terminal) ||
					(cur->policy.config_lock != prev->policy.config_lock) ||
					(cur->policy.permanent != prev->policy.permanent) ||
					(cur->policy.alloc != prev->policy.alloc))
				{
					cfgchg.policy.b = (cur->policy.terminal ? cfgchg_t_TERMINAL_POLICY : 0);
					cfgchg.policy.b |= (cur->policy.config_lock ? cfgchg_t_LOCK_POLICY : 0);
					cfgchg.policy.b |= (cur->policy.permanent ? cfgchg_t_PERMANENT_POLICY : 0);
					cfgchg.policy.alloc = cur->policy.alloc;
				
#ifdef DISPLAY_EVENT_MSG
					kprintf("CONFIG_CHG_POLICY: mp %p (%s) alloc/term/cfg_lck/perm = %d/%c/%c/%c\n",
							mpart, mpart->name,
							cfgchg.policy.alloc,
							(cfgchg.policy.bool & cfgchg_t_TERMINAL_POLICY) ? 'T' : 'F',
							(cfgchg.policy.bool & cfgchg_t_LOCK_POLICY) ? 'T' : 'F',
							(cfgchg.policy.bool & cfgchg_t_PERMANENT_POLICY) ? 'T' : 'F');
#endif	/* DISPLAY_EVENT_MSG */

					SEND_MP_EVENT(evt_list, (void *)&cfgchg, NULL);
				}
			}
			break;
		}

		case mempart_evttype_t_PARTITION_CREATE:
		case mempart_evttype_t_PARTITION_DESTROY:
		{
			/* check to see if there are any events of this type registered */
			evt_list = &mpart->event_list[MEMPART_EVTYPE_IDX(evtype)];
			if (LIST_COUNT(evt_list->active.list) > 0)
			{
				/*
				 * addition arguments:
				 * arg1 - part_id_t to the created partition
				 * Note that the first argument <mpart> is the parent for which
				 * child partition add/del events were registered on
				*/
				part_id_t id = va_arg(ap, part_id_t);

#ifdef DISPLAY_EVENT_MSG
				kprintf("PARTITION_%s: %p added to parent %p (%s)\n",
						(evtype == mempart_evttype_t_PARTITION_CREATE) ? "CREATE" : "DESTROY", id,
						mpart, mpart->name);
#endif	/* DISPLAY_EVENT_MSG */

				/*
				 * for destroy events, we optionally check whether a
				 * specific partition id of interest has been provided
				*/
				if (evtype == mempart_evttype_t_PARTITION_DESTROY) {
					SEND_MP_EVENT(evt_list, (void *)&id, mpid_match);
				} else {
					SEND_MP_EVENT(evt_list, (void *)&id, NULL);
				}
			}
			break;
		}
		
		case mempart_evttype_t_OBJ_ASSOCIATE:
		case mempart_evttype_t_OBJ_DISASSOCIATE:
		{
			/* check to see if there are any events of this type registered */
			evt_list = &mpart->event_list[MEMPART_EVTYPE_IDX(evtype)];
			if (LIST_COUNT(evt_list->active.list) > 0)
			{
				/*
				 * addition arguments:
				 * arg1 - OBJECT * of the associated/disassociated object
				*/
				OBJECT  *obj = va_arg(ap, OBJECT *);
#ifdef DISPLAY_EVENT_MSG
				kprintf("OBJ_%sASSOCIATE: obj %p now associated with mp %p (%s)\n",
						(evtype == mempart_evttype_t_OBJ_DISASSOCIATE) ? "DIS" : "", obj,
						(evtype == mempart_evttype_t_OBJ_DISASSOCIATE) ? "no longer" : "now",
						mpart, mpart->name);
#endif	/* DISPLAY_EVENT_MSG */

				/*
				 * for disassociation events, we optionally check whether a
				 * specific process id of interest has been provided
				*/
				if (evtype == mempart_evttype_t_OBJ_DISASSOCIATE) {
					SEND_MP_EVENT(evt_list, (void *)&obj, oid_match);
				} else {
					SEND_MP_EVENT(evt_list, (void *)&obj, NULL);
				}
			}
			break;
		}

		default:
#ifdef DISPLAY_EVENT_MSG
			kprintf("Unknown event: %d\n", evtype);
#endif	/* DISPLAY_EVENT_MSG */
			break;
	}
	va_end(ap);
}

/*******************************************************************************
 * apmmgr_event_trigger2
 * 
 * This is the routine which is made available externally to the memory
 * partitioning module and performs initial event processing for memory classes
 * 
 * this event handler is called from the memory class allocator. The only
 * events sent from the allocator are ...
 * 
 * 	memclass_evttype_t_DELTA_TU_INCR - implying an allocation
 * 	memclass_evttype_t_DELTA_TU_DECR - implying a deallocation
 * 	memclass_evttype_t_DELTA_RF_INCR - implying an increase in the amount of
 * 										reserved memory configured
 * 	memclass_evttype_t_DELTA_RF_DECR - implying a decrease in the amount of
 * 										reserved memory configured
 * 	memclass_evttype_t_DELTA_RU_INCR - implying an exchange of memory as being
 * 										accounted to reserved where it was
 * 										previously accounted as unreserved
 *	memclass_evttype_t_DELTA_RU_DECR - implying an exchange of memory as being
 * 										accounted to unreserved where it was
 * 										previously accounted as reserved
 * 	memclass_evttype_t_PARTITION_CREATE	- a partition of the memory class was
 * 										created
 * 	memclass_evttype_t_PARTITION_DESTROY- a partition of the memory class was
 * 										destroyed
 * 
 * Note that all of the other events can be generated based on the previous
 * and current size information.
 * 
 * This function will generate all of the sub DELTA events from the primary
 * events received from the allocator and call _apmmgr_event_trigger2() to
 * deliver them. _apmmgr_event_trigger2() will in turn generate all
 * applicable THRESHOLD_CROSS events. Ultimately, __apmmgr_event_trigger2()
 * will be called to deliver any registered events.
 *
 * The PARTITION_CREATE/DESTROY events are handled directly by this routine.
 * Also, the PARTITION_CREATE/DESTROY events will send 'size_info' as a pointer
 * to the partition identifier created/destroyed.
 *
 * additional arguments:
 * arg1 - memclass_sizeinfo_t
*/
void apmmgr_event_trigger2(apmmgr_attr_t *mpart, memclass_evttype_t evtype,
								memclass_sizeinfo_t *cur_size_info,
								memclass_sizeinfo_t *prev_size_info)
{
	/*
	 * it is possible that there is no apmmgr_attr_t yet even though there
	 * is an mempart_t (ie. mempart_t.resmgr_attr_p == NULL)
	*/
	if (mpart == NULL) return;

	switch (evtype)
	{
		/*
		 * an allocation has occurred for the memory class. This has the
		 * possibility to generate 4 separate events
		 * 
		 * 		total used increment
		 * 		total free decrement
		 * 		reserved used increment
		 * 		unreserved used increment
		 * 		reserved free decrement
		 * 		unreserved free decrement
		 * 
		 * Either 4 of these events will occur or all 6 will occur as the first
		 * 2 are unconditional
		*/
		case memclass_evttype_t_DELTA_TU_INCR:
		{
			if ((cur_size_info->reserved.used + cur_size_info->unreserved.used) >
				(prev_size_info->reserved.used + prev_size_info->unreserved.used))
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_TU_INCR, 
											cur_size_info->reserved.used + cur_size_info->unreserved.used,
											prev_size_info->reserved.used + prev_size_info->unreserved.used);
			}
			if ((cur_size_info->reserved.free + cur_size_info->unreserved.free) <
				(prev_size_info->reserved.free + prev_size_info->unreserved.free))
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_TF_DECR, 
											cur_size_info->reserved.free + cur_size_info->unreserved.free,
											prev_size_info->reserved.free + prev_size_info->unreserved.free);
			}
			if (cur_size_info->reserved.used > prev_size_info->reserved.used)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_RU_INCR, 
											cur_size_info->reserved.used,
											prev_size_info->reserved.used);
			}
			if (cur_size_info->unreserved.used > prev_size_info->unreserved.used)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_UU_INCR, 
											cur_size_info->unreserved.used,
											prev_size_info->unreserved.used);
			}
			if (cur_size_info->reserved.free < prev_size_info->reserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_RF_DECR, 
											cur_size_info->reserved.free,
											prev_size_info->reserved.free);
			}
			if (cur_size_info->unreserved.free < prev_size_info->unreserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_UF_DECR, 
											cur_size_info->unreserved.free,
											prev_size_info->unreserved.free);
			}
			break;
		}

		/*
		 * a deallocation has occurred for the memory class. This has the
		 * possibility to generate 6 separate events
		 * 
		 * 		total used decrement
		 * 		total free increment
		 * 		reserved used decrement
		 * 		unreserved used decrement
		 * 		reserved free increment
		 * 		unreserved free increment
		 *
		 * Either 4 of these events will occur or all 6 will occur as the first
		 * 2 are unconditional
		*/
		case memclass_evttype_t_DELTA_TU_DECR:
		{
			if ((cur_size_info->reserved.used + cur_size_info->unreserved.used) <
				(prev_size_info->reserved.used + prev_size_info->unreserved.used))
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_TU_DECR, 
											cur_size_info->reserved.used + cur_size_info->unreserved.used,
											prev_size_info->reserved.used + prev_size_info->unreserved.used);
			}
			if ((cur_size_info->reserved.free + cur_size_info->unreserved.free) >
				(prev_size_info->reserved.free + prev_size_info->unreserved.free))
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_TF_INCR, 
											cur_size_info->reserved.free + cur_size_info->unreserved.free,
											prev_size_info->reserved.free + prev_size_info->unreserved.free);
			}
			if (cur_size_info->reserved.used < prev_size_info->reserved.used)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_RU_DECR, 
											cur_size_info->reserved.used,
											prev_size_info->reserved.used);
			}
			if (cur_size_info->unreserved.used < prev_size_info->unreserved.used)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_UU_DECR, 
											cur_size_info->unreserved.used,
											prev_size_info->unreserved.used);
			}
			if (cur_size_info->reserved.free > prev_size_info->reserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_RF_INCR, 
											cur_size_info->reserved.free,
											prev_size_info->reserved.free);
			}
			if (cur_size_info->unreserved.free > prev_size_info->unreserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_UF_INCR, 
											cur_size_info->unreserved.free,
											prev_size_info->unreserved.free);
			}
			break;
		}
		
		case memclass_evttype_t_DELTA_RF_INCR:
		{
			/*
			 * an increase to reserved memory occurs when a reservation is
			 * established for a partition. In this situation, the only
			 * parameters that change are the reserved.free increases and the
			 * unreserved.free decreases by a corresponding amount. The used
			 * amounts do not change hence there are only 2 possible events to
			 * generate from this event
			 * 
			 *	memclass_evttype_t_DELTA_RF_INCR
			 * 	memclass_evttype_t_DELTA_UF_DECR
			*/
			if (cur_size_info->reserved.free > prev_size_info->reserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_RF_INCR, 
											cur_size_info->reserved.free, prev_size_info->reserved.free);
			}
			if (cur_size_info->unreserved.free < prev_size_info->unreserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_UF_DECR, 
											cur_size_info->unreserved.free, prev_size_info->unreserved.free);
			}
			break;
		}

		case memclass_evttype_t_DELTA_RF_DECR:
		{
			/*
			 * a decrease to reserved memory occurs when a reservation is
			 * released for a partition. In this situation, the only
			 * parameters that change are the reserved.free decreases and the
			 * unreserved.free increases by a corresponding amount. The used
			 * amounts do not change hence there are only 2 possible events to
			 * generate from this event
			 * 
			 *	memclass_evttype_t_DELTA_RF_DECR
			 * 	memclass_evttype_t_DELTA_UF_INCR
			*/
			if (cur_size_info->reserved.free < prev_size_info->reserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_RF_DECR, 
											cur_size_info->reserved.free, prev_size_info->reserved.free);
			}
			if (cur_size_info->unreserved.free > prev_size_info->unreserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_UF_INCR, 
											cur_size_info->unreserved.free, prev_size_info->unreserved.free);
			}
			break;
		}

		case memclass_evttype_t_DELTA_RU_INCR:
		{
			/*
			 * an increase to reserved used memory occurs when allocated memory
			 * was previously accounted to unreserved and will now be accounted
			 * to reserved. In this situation, the parameters that change are
			 * are the reserved.used increases and the unreserved.used decreases.
			 * Also, in order to keep the totals constant, reserved.free decreases
			 * and the unreserved.free increases by identical amounts. The 4 events
			 * to generate from this event are
			 * 
			 *	memclass_evttype_t_DELTA_RU_INCR
			 * 	memclass_evttype_t_DELTA_RF_DECR
			 * 	memclass_evttype_t_DELTA_UU_DECR
			 * 	memclass_evttype_t_DELTA_UF_INCR
			*/
			if (cur_size_info->reserved.used > prev_size_info->reserved.used)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_RU_INCR, 
											cur_size_info->reserved.used, prev_size_info->reserved.used);
			}
			if (cur_size_info->reserved.free < prev_size_info->reserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_RF_DECR,
											cur_size_info->reserved.free, prev_size_info->reserved.free);
			}
			if (cur_size_info->unreserved.used < prev_size_info->unreserved.used)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_UU_DECR, 
											cur_size_info->unreserved.used, prev_size_info->unreserved.used);
			}
			if (cur_size_info->unreserved.free > prev_size_info->unreserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_UF_INCR,
											cur_size_info->unreserved.free, prev_size_info->unreserved.free);
			}
			break;
		}

		case memclass_evttype_t_DELTA_RU_DECR:
		{
			/*
			 * a decrease to reserved used memory occurs when allocated memory
			 * was previously accounted to reserved and will now be accounted
			 * to unreserved. In this situation, the parameters that change are
			 * are the reserved.used decreases and the unreserved.used increases.
			 * Also, in order to keep the totals constant, reserved.free increases
			 * and the unreserved.free decreases by identical amounts. The 4 events
			 * to generate from this event are
			 * 
			 *	memclass_evttype_t_DELTA_RU_DECR
			 * 	memclass_evttype_t_DELTA_RF_INCR
			 * 	memclass_evttype_t_DELTA_UU_INCR
			 * 	memclass_evttype_t_DELTA_UF_DECR
			*/
			if (cur_size_info->reserved.used < prev_size_info->reserved.used)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_RU_DECR, 
											cur_size_info->reserved.used, prev_size_info->reserved.used);
			}
			if (cur_size_info->reserved.free > prev_size_info->reserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_RF_INCR,
											cur_size_info->reserved.free, prev_size_info->reserved.free);
			}
			if (cur_size_info->unreserved.used > prev_size_info->unreserved.used)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_UU_INCR, 
											cur_size_info->unreserved.used, prev_size_info->unreserved.used);
			}
			if (cur_size_info->unreserved.free < prev_size_info->unreserved.free)
			{
				_apmmgr_event_trigger2(mpart, memclass_evttype_t_DELTA_UF_DECR,
											cur_size_info->unreserved.free, prev_size_info->unreserved.free);
			}
			break;
		}

		case memclass_evttype_t_PARTITION_CREATE:
		case memclass_evttype_t_PARTITION_DESTROY:
		{
			part_id_t id = *((part_id_t *)cur_size_info);
			part_evtlist_t  *evt_list;

			/* check to see if there are any events of this type registered */
			evt_list = &mpart->event_list[MEMCLASS_EVTYPE_IDX(evtype)];
			if (LIST_COUNT(evt_list->active.list) > 0)
			{
#ifdef DISPLAY_EVENT_MSG
				kprintf("%s in memory class %p (%s), id = %p\n",
							mclass_evt_string(evtype), mpart, mpart->name, id);
#endif	/* DISPLAY_EVENT_MSG */
				SEND_MC_EVENT(evt_list, (void *)&id);
			}
			break;
		}

		default:
#ifdef DISPLAY_EVENT_MSG
			kprintf("Unknown event: %d\n", evtype);
#endif	/* DISPLAY_EVENT_MSG */
			break;
	}
}

/*
 * =============================================================================
 * 
 * 					D E B U G   S U P P O R T   R O U T I N E S
 * 
 * =============================================================================
*/
#if !defined(NDEBUG) && defined(DISPLAY_EVENT_MSG)
static char *mclass_evt_strings[] =
{
	//	free and used crossings (total)
	[memclass_evttype_t_THRESHOLD_CROSS_TF_OVER] = "TOTAL FREE OVER",
	[memclass_evttype_t_THRESHOLD_CROSS_TF_UNDER] = "TOTAL FREE UNDER",
	[memclass_evttype_t_THRESHOLD_CROSS_TU_OVER] = "TOTAL USED OVER",
	[memclass_evttype_t_THRESHOLD_CROSS_TU_UNDER] = "TOTAL USED UNDER",
	//	free crossings (reserved and unreserved)
	[memclass_evttype_t_THRESHOLD_CROSS_RF_OVER] = "RESERVED FREE OVER",
	[memclass_evttype_t_THRESHOLD_CROSS_RF_UNDER] = "RESERVED FREE UNDER",
	[memclass_evttype_t_THRESHOLD_CROSS_UF_OVER] = "UNRESERVED FREE OVER",
	[memclass_evttype_t_THRESHOLD_CROSS_UF_UNDER] = "UNRESERVED FREE UNDER",
	//	used crossings	(reserved and unreserved)
	[memclass_evttype_t_THRESHOLD_CROSS_RU_OVER] = "RESERVED USED OVER",
	[memclass_evttype_t_THRESHOLD_CROSS_RU_UNDER] = "RESERVED USED UNDER",
	[memclass_evttype_t_THRESHOLD_CROSS_UU_OVER] = "UNRESERVED USED OVER",
	[memclass_evttype_t_THRESHOLD_CROSS_UU_UNDER] = "UNRESERVED USED UNDER",
	
	//	free and used deltas (total)
	[memclass_evttype_t_DELTA_TF_INCR] = "TOTAL FREE DELTA INCR",
	[memclass_evttype_t_DELTA_TF_DECR] = "TOTAL FREE DELTA DECR",
	[memclass_evttype_t_DELTA_TU_INCR] = "TOTAL USED DELTA INCR",
	[memclass_evttype_t_DELTA_TU_DECR] = "TOTAL USED DELTA DECR",
	//	free deltas (reserved and unreserved)
	[memclass_evttype_t_DELTA_RF_INCR] = "RESERVED FREE DELTA INCR",
	[memclass_evttype_t_DELTA_RF_DECR] = "RESERVED FREE DELTA DECR",
	[memclass_evttype_t_DELTA_UF_INCR] = "UNRESERVED FREE DELTA INCR",
	[memclass_evttype_t_DELTA_UF_DECR] = "UNRESERVED FREE DELTA DECR",
	//	used deltas (reserved and unreserved)
	[memclass_evttype_t_DELTA_RU_INCR] = "RESERVED USED DELTA INCR",
	[memclass_evttype_t_DELTA_RU_DECR] = "RESERVED USED DELTA DECR",
	[memclass_evttype_t_DELTA_UU_INCR] = "UNRESERVED USED DELTA INCR",
	[memclass_evttype_t_DELTA_UU_DECR] = "UNRESERVED USED DELTA DECR",
	
	[memclass_evttype_t_PARTITION_CREATE] = "MEMCLASS PARTITION CREATE",
	[memclass_evttype_t_PARTITION_DESTROY] = "MEMCLASS PARTITION DESTROY",
};
static char *mclass_evt_string(memclass_evttype_t evt) {return mclass_evt_strings[evt];}
#endif	/* NDEBUG */


__SRCVERSION("$IQ: apmmgr_event.c,v 1.23 $");

