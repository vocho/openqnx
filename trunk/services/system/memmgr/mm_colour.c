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

#include "vmm.h"

// These functions are only used on systems with memory colour

struct colour_data {
	off64_t		off;
	unsigned	colour;
};


static int
mapping_colour(struct mm_object_ref *or, struct mm_map *mm, void *d) {
	struct colour_data 	*data = d;
	int					adjust;

	adjust = (data->off - mm->offset) >> ADDR_OFFSET_BITS;
	data->colour = ((mm->start >> ADDR_OFFSET_BITS) + adjust) & colour_mask;
	return -1;
}


unsigned
memobj_colour(OBJECT *obp, off64_t off) {
	unsigned			curr;
	struct pa_quantum	*pq;
	struct pa_quantum	*next;
	int					adjust;
	unsigned			num;
	struct colour_data	data;

	pq = obp->mem.mm.pmem;
	for( ;; ) {
		if(pq == NULL) break;
		next = pq->u.inuse.next;
		curr = pq->u.inuse.qpos;
		num = pq->run;
		if(pq->blk == PAQ_BLK_FAKE) {
			if(pq->flags & PAQ_FLAG_INITIALIZED) goto found_colour;
		} else {
			do {
				if(pq->flags & PAQ_FLAG_INITIALIZED) goto found_colour;
				++pq;
				curr += 1;
				--num;
			} while(num != 0);
		}
		pq = next;
	}

	// None of the pa_quantum's have been initialized - see if there
	// are any active mappings to the object and figure out the colour
	// from that.
	data.colour = PAQ_COLOUR_NONE;
	data.off = off;
	memref_walk(obp, mapping_colour, &data);
	return data.colour;

found_colour:
	adjust = (off - ((off64_t)curr << QUANTUM_BITS)) >> ADDR_OFFSET_BITS;
	return (PAQ_GET_COLOUR(pq) + adjust) & colour_mask;
}


void
colour_set(uintptr_t va, struct pa_quantum *pq, unsigned num) {
	unsigned	colour;
	unsigned	pq_colour;

	colour = COLOUR_VA(va);
	do {
		pq_colour = PAQ_GET_COLOUR(pq);
		if((pq->blk != PAQ_BLK_FAKE)
		  &&(pq_colour != PAQ_COLOUR_NONE)
		  &&(pq_colour != colour)) {
			cpu_colour_clean(pq, COLOUR_CLEAN_PURGE);
		}
		PAQ_SET_COLOUR(pq, colour);
		colour = (colour + (QUANTUM_SIZE >> ADDR_OFFSET_BITS)) & colour_mask;
		++pq;
		--num;
	} while(num != 0);
}

__SRCVERSION("mm_colour.c $Rev: 153052 $");
