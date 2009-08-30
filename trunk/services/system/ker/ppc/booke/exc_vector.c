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

#ifdef VARIANT_smp
/*
 * Set up IVOR registers to point at exception handlers
 */
struct ppcbke_ivor {
	unsigned	idx;
	uintptr_t	value;
};

static struct ppcbke_ivor	ppcbke_ivor[64];

void
smp_exc_init()
{
	int		i;

	for (i = 0; i < NUM_ELTS(ppcbke_ivor); i++) {
		if (ppcbke_ivor[i].idx == ~0U) {
			break;
		}
		set_spr_indirect(ppcbke_ivor[i].idx, ppcbke_ivor[i].value);
	}
}
#endif

// The first 1K is used for unexpected exception locations.
#define STARTING_ADDR	0x400

static uintptr_t	next_location = STARTING_ADDR;

void
exc_vector_init() {
	unsigned	idx;

	/*
	 * Fill the exception area with branch and link instructions so
	 * if we get something we don't expect, the kernel will hardcrash.
	 */
	for(idx = 0; idx < STARTING_ADDR; idx += 0x10) {
		*(uint32_t *)idx = 0x48000003 | next_location;
		icache_flush(idx);
	}

#ifdef VARIANT_smp
	for(idx = 0; idx < NUM_ELTS(ppcbke_ivor); idx++) {
		ppcbke_ivor[idx].idx = ~0U;
	}
#endif

	// The IVOR number is arbitrary, it'll be reset when the
	// trap set gets installed - we're just using it as a placeholder.
	// Setting it to zero will cause exc_vector_address() allocate
	// a new location for the "real" handler.
	trap_install(PPCBKE_SPR_IVOR8, __exc_unexpected, &__common_exc_entry);
	set_spr(PPCBKE_SPR_IVOR8, 0); 
}

uint32_t *
exc_vector_address(unsigned idx, unsigned size) {
	uintptr_t	addr;
	uintptr_t	end;
#ifdef VARIANT_smp
	int			i;
#endif

	addr = get_spr_indirect(idx);
	if(addr < STARTING_ADDR) {
		addr = next_location;
		set_spr_indirect(idx, addr);

#ifdef VARIANT_smp
		for (i = 0; i < NUM_ELTS(ppcbke_ivor); i++) {
			if (ppcbke_ivor[i].idx == ~0U || ppcbke_ivor[i].idx == idx) {
				ppcbke_ivor[i].idx = idx;
				ppcbke_ivor[i].value = addr;
				break;
			}
		}
		CRASHCHECK(i == NUM_ELTS(ppcbke_ivor));
#endif
	}
	
	end = (addr + size + 0xf) & ~0xf;
	if(end > next_location) next_location = end;

	return (uint32_t *)addr;
}

void
exc_intr_install(struct intrinfo_entry *iip, void *handler, void **info) {
	const struct exc_copy_block	*intr;

	if(iip->flags & PPC_INTR_FLAG_CI) {
		intr = &intr_entry_critical;
	} else {
		intr = &intr_entry;
	}

	// If 'handler' == 0, we don't know where it's going to end up, but
	// we can't pass the NULL into trap_install() because that value is
	// a flag not to invoke the PPC_COMMON_ENTRY location and we want to
	// go there. Set the handler to a non-NULL value so trap_install()
	// does the right thing. We'll be coming back in here again once
	// we know where the handler location actually is.
	if(handler == 0) handler = (void *)1;
	trap_install(iip->cpu_intr_base, handler, intr);
}

void
exc_report_unexpected(unsigned vector) {
	kprintf("Unexpected IVOR%d exception usage\n"
			"Check startup code for missing interrupt callout definitions.\n",
			vector >> 4);
}

__SRCVERSION("exc_vector.c $Rev: 199396 $");
