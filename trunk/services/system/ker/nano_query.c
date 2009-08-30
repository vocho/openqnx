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

#include "externs.h"

THREAD *get_active(void) {
	return actives[KERNCPU];
}

static const unsigned vec_offset[] = {
	offsetof(PROCESS, chancons),
	offsetof(PROCESS, fdcons),
	offsetof(PROCESS, threads),
	offsetof(PROCESS, timers),
};


void *
nano_query(int type, unsigned index1, int subtype, unsigned index2,
							unsigned *next, void *objbuf, int objsize) {
	VECTOR	*vecp;
	void	*obj;
	PROCESS	*prp;

	if((vecp = vector_search(&query_vector, type, 0))) {
		switch(type) {
		case _QUERY_PROCESS:
			if(subtype == _QUERY_PROCESS_VECTOR) {
				while((obj = prp = vector_search(vecp, PINDEX(index1), next))) {
					if(next) {
						if(PINDEX(prp->pid) == *next) {
							break;						
						}
						index1 = *next + 1;
					} else {
						if(prp->pid != index1) {
							obj = 0;
						}
						break;
					}
				}
			} else if((obj = prp = vector_search(vecp, PINDEX(index1), 0)) && (prp->pid == index1) && (index1 != 0)) {
				obj = vector_search((VECTOR *)((uintptr_t)prp + vec_offset[subtype - _QUERY_PROCESS_CHANCONS]), index2, next);
			} else {
				obj = NULL;
			}
			break;
		case _QUERY_INTERRUPT:
			obj = vector_search(vecp, index1, next);
			if(obj && objbuf) {
				struct interrupt_entry			*iep = obj;
				struct interrupt_query_entry	*iqep = objbuf;
				struct interrupt_level			*ilp;

				// We have to fill in some extra stuff since proc
				// can't follow the thread pointer after we've returned
				// and it doesn't know about interrupt levels.
				if(iep->handler == NULL) {
					iqep->event = *(struct sigevent *)iep->area;
				}
				iqep->tid = iep->thread->tid + 1;
				iqep->pid = iep->thread->process->pid;
				ilp = &interrupt_level[iep->level];
				iqep->vector = ilp->info->vector_base + iep->level - ilp->level_base;

				// Restrict the copy to just the interrupt_entry portion.
				objsize = sizeof(*iep);
			}
			break;
		default:
			obj = vector_search(vecp, index1, next);
			break;
		}

		if(obj && objbuf) {
			memcpy(objbuf, obj, objsize);
		}

		return(obj);
	}

	return(NULL);
}

__SRCVERSION("nano_query.c $Rev: 153052 $");
