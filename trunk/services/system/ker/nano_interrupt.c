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
#include "kertrace.h"

int rdecl
get_interrupt_level(THREAD *act, unsigned vector) {
	unsigned				level;
	int						cascade;
	unsigned				i;
	struct intrinfo_entry	*iip;

	if(vector >= _NTO_INTR_CLASS_SYNTHETIC && vector <= _NTO_HOOK_LAST) {
		//A valid hook routine
		return(HOOK_TO_LEVEL(vector));
	}
	for( ;; ) {
		iip = intrinfoptr;
		level = 0;
		i = 0;
		for( ;; ) {
			if(i >= intrinfo_num) {
				/* out of range number */
				if(act != NULL) kererr(act, EINVAL);
				return(-1);
			}
			if(vector >= iip->vector_base
			&& vector < (iip->vector_base+iip->num_vectors)) break;
			level += iip->num_vectors;
			++i;
			++iip;
		}
		level += vector - iip->vector_base;
		cascade = interrupt_level[level].cascade_level;
		if(cascade < 0) {
			if((interrupt_level[level].config & INTR_CONFIG_FLAG_DISALLOWED)
				&& alives[0]) {
				if (act != NULL) kererr(act, EINVAL);
				return(-1);
			}
			return(level);
		}
		//
		// Guy wanted a cascaded interrupt level. Give him the first one
		// of the secondary controller instead.
		//
		vector = interrupt_level[cascade].info->vector_base;
	}
}

static const struct sigevent *
dummy_handler(void *dummy, int id) {
	return NULL;
}

//
// The plus two is because we have to leave the "-1" slot open for error
// reporting.
//
#define NUM_HOOK_RTNS	((_NTO_HOOK_LAST+2) & ~_NTO_INTR_CLASS_SYNTHETIC)

void rdecl
interrupt_init() {
	unsigned				i;
	struct intrinfo_entry	*iip;
	unsigned				level_base;
	unsigned				num_external_levels;
	INTRLEVEL				*ilp;

	num_external_levels = 0;
	iip = intrinfoptr;
	for(i = 0; i < intrinfo_num; ++i, ++iip) {
		num_external_levels += iip->num_vectors;
	}
	interrupt_level = _scalloc((num_external_levels+NUM_HOOK_RTNS) * sizeof(*interrupt_level));
	interrupt_level += NUM_HOOK_RTNS; //Point at first external level

	// Set the mask count of all interrupt levels to one, point at
	// appropriate info entry.
	level_base = 0;
	iip = intrinfoptr;
	for(i = 0; i < num_external_levels; ++i) {
		if(i >= (level_base+iip->num_vectors)) {
			level_base += iip->num_vectors;
			++iip;
		}
		ilp = &interrupt_level[i];
		ilp->info = iip;
		ilp->cascade_level = -1;
		ilp->level_base = level_base;
		ilp->mask_count = 1;
		ilp->config = (iip->config != NULL) ? iip->config(_syspage_ptr, iip, i - level_base) : 0;
	}
	// Generate the interrupt handlers
	cpu_interrupt_init(num_external_levels);

	// Initialize the cascaded entries
	level_base = 0;
	iip = intrinfoptr;
	for(i = 0; i < intrinfo_num; ++i, ++iip) {
		int level = get_interrupt_level(NULL, iip->cascade_vector);

		if(level != -1) {
			ilp = &interrupt_level[level];
			ilp->cascade_level = level_base;
			(void)ilp->info->unmask(_syspage_ptr, level - ilp->level_base);
			ilp->mask_count = 0;
		}
		level_base += iip->num_vectors;
	}
	for(i = 0; i < num_external_levels; ++i) {
		if(interrupt_level[i].config & INTR_CONFIG_FLAG_PREATTACH) {
			(void)interrupt_attach(i, dummy_handler, NULL, 0);
		}
	}
}

int rdecl
interrupt_attach(int level, const struct sigevent *(*handler)(void *area, int id),
			void *area, int flags) {
	INTERRUPT	*itp;
	INTERRUPT	**owner;
	INTRLEVEL	*ilp;
	unsigned	count;
	THREAD		*act = actives[KERNCPU];
	int			id;

	count = 0;
	ilp = &interrupt_level[level];
	owner = &ilp->queue;
	for( ;; ) {
		itp = *owner;
		if(itp == NULL) break;
		++count;
		owner = &itp->next;
	}
	if(count > INTR_LEVEL_ATTACH_MAX) {
		kererr(act, EAGAIN);
		return -1;
	}
	if(!(flags & _NTO_INTR_FLAGS_END)) {
		/* adding to start of list */
		owner = &ilp->queue;
	}

	// Allocate an interrupt entry.
	if((itp = object_alloc(act->process, &interrupt_souls)) == NULL) {
		kererr(act, EAGAIN);
		return -1;
	}

	if(level >= 0) {
		int	num_md;

		num_md = mdriver_intr_attach(_TRACE_INT_NUM(ilp));
		if(num_md != 0) {
			count -= num_md;
			if(flags & _NTO_INTR_FLAGS_END) {
				INTERRUPT	*itp2;

				// Some mini-driver interrupts have detached, so we
				// have to find the end of the queue again
				owner = &ilp->queue;
				for( ;; ) {
					itp2 = *owner;
					if(itp2 == NULL) break;
					owner = &itp2->next;
				}
			}
		}
	}

	itp->thread = act;
	itp->level = level;
	itp->handler = handler;
	itp->area = area;
	itp->flags = flags;
	cpu_intr_attach(itp, act);

	// Add the entry to the interrupt vector.
	if((id = vector_add(&interrupt_vector, itp, 0)) == -1) {
		object_free(act->process, &interrupt_souls, itp);
		kererr(act, EAGAIN);
		return -1;
	}
	itp->id = id;
	act->process->flags |= _NTO_PF_CHECK_INTR;

	// Add an interrupt handler to the interrupt table.
	itp->next = *owner;
	*owner = itp;

	if(count == 0) {
		/* enable level when first handler gets installed */
		do { } while(interrupt_unmask(level, NULL) != 0);
	}
	return(id);
}

void rdecl
interrupt_detach(int level, const struct sigevent *(*handler)(void *area, int id)) {
	INTERRUPT	*itp;
	THREAD		*act = actives[KERNCPU];
	int			 i;

	// Lookup the interrupt.
	for(i = 0; i < interrupt_vector.nentries; ++i) {
		if(VECP(itp, &interrupt_vector, i)) {
			if(itp->level == level  &&  itp->handler == handler
			&& itp->thread == act) {
				break;
			}
		}
	}

	if(i >= interrupt_vector.nentries) {
		kererr(act, EINVAL);
		return;
	}

	interrupt_detach_entry(act->process, i);
}

void rdecl
interrupt_detach_entry(PROCESS *prp, int index) {
	INTERRUPT	*itp;

	lock_kernel();
	// Remove the entry from the interrupt vector.
	if((itp = vector_rem(&interrupt_vector, index)) == NULL)
		crash();

	// If no more handlers, disable the interrupt.
	// It should be done before removing the itp from the interrupt table
	if( (interrupt_level[itp->level].queue->next == NULL) && (interrupt_level[itp->level].queue == itp) ) {
#ifdef VARIANT_instr
		if ( HOOK_TO_LEVEL(_NTO_HOOK_TRACE) == itp->level ) {
			/*
			 * If a tracelogger daemon was attached and was not logging
			 * in ringmode, then stop tracing.
			 */
			if( !trace_masks.ring_mode) {
				trace_masks.main_flags = 0;
			}
		}
#endif
		(void)interrupt_mask(itp->level, NULL);
		interrupt_level[itp->level].mask_count = 1;
		itp->mask_count = 0;
	} else {
		if(itp->flags & _NTO_INTR_FLAGS_TRK_MSK) {
			//If we've been keeping track of how many times an attach point has
			//masked a level, undo those masks now.
			while(itp->mask_count != 0) {
				(void)interrupt_unmask(itp->level, itp);
			}
		}
	}

	// Remove interrupt handler from the interrupt table.
	// NOTE: We're assuming that LINK1_REM is unlinking atomicly
	LINK1_REM(interrupt_level[itp->level].queue, itp, INTERRUPT);

	// flush out the interrupt queues to make sure there's nothing pending
	// that's still pointing at the INTERRUPT object
	intrevent_flush();

	// Release the interrupt entry.
	// If no handler we also release "area" which points to a kernel
	// allocated sigevent
	if(itp->handler == NULL) {
		_sfree(itp->area, sizeof(struct sigevent));
	}
	object_free(prp, &interrupt_souls, itp);
}

int rdecl
interrupt_mask(int level, INTERRUPT *itp) {
	INTRLEVEL	*ilp;
	int			new;

   	if(level < 0) return(0);

	ilp = &interrupt_level[level];
	CPU_SLOCK_INTR_LOCK(&intr_slock);
	if(itp != NULL) {
		++itp->mask_count;
	}
	if(ilp->mask_count++ == 0) {
		(void)ilp->info->mask(_syspage_ptr, level - ilp->level_base);
#if defined(VARIANT_smp)
		if(alives[0] && ilp->info->flags & INTR_FLAG_SMP_BROADCAST_MASK) {
			unsigned	i;
			unsigned	mycpu = RUNCPU;

			for(i = 0; i < NUM_PROCESSORS; ++i) {
				if(i != mycpu) SENDIPI(i, IPI_INTR_MASK);
			}
		}
#endif
	}
	new = ilp->mask_count;	// snarf before intrs enabled, avoid races
	CPU_SLOCK_INTR_UNLOCK(&intr_slock);
	return(new);
}

int rdecl
interrupt_mask_vector(unsigned vector, int id) {
	int level = get_interrupt_level(NULL, vector);

	if(level == -1) return(-1);
	return(interrupt_mask(level, vector_lookup(&interrupt_vector, id)));
}

int rdecl
interrupt_unmask(int level, INTERRUPT *itp) {
	INTRLEVEL	*ilp;
	int			new;

   	if(level < 0) return(0);
	ilp = &interrupt_level[level];
	/* don't enable/disable interrupts during kernel initialization */
	if(alives[0]){
		CPU_SLOCK_INTR_LOCK(&intr_slock);
	}

	if(itp != NULL && itp->mask_count != 0) {
		--itp->mask_count;
	}
	switch(ilp->mask_count) {
	case 0:
		/* already unmasked */
		break;
	case 1:
		(void)ilp->info->unmask(_syspage_ptr, level - ilp->level_base);
#if defined(VARIANT_smp)
		if(alives[0] && ilp->info->flags & INTR_FLAG_SMP_BROADCAST_UNMASK) {
			unsigned	i;
			unsigned	mycpu = RUNCPU;

			for(i = 0; i < NUM_PROCESSORS; ++i) {
				if(i != mycpu) SENDIPI(i, IPI_INTR_UNMASK);
			}
		}
#endif
		/* fall through */
	default:
		ilp->mask_count--;
		break;
	}
	new = ilp->mask_count;	// snarf before intrs enabled, avoid races
	/* don't enable/disable interrupts during kernel initialization */
	if(alives[0]) {
		CPU_SLOCK_INTR_UNLOCK(&intr_slock);
	}
	return(new);
}

int rdecl
interrupt_unmask_vector(unsigned vector, int id) {
	int level = get_interrupt_level(NULL, vector);

	if(level == -1) return(-1);
	return(interrupt_unmask(level, vector_lookup(&interrupt_vector, id)));
}

__SRCVERSION("nano_interrupt.c $Rev: 201493 $");
