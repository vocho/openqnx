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

// These functions are overridden for the book E version

uintptr_t	low_code = PPC_LOW_CODE_START;

#define SHORT_VECTOR 0x40000000

void
exc_vector_init() {
	unsigned	idx;
	uint32_t	*vector;

	/*
	 * Fill the exception area with branch and link instructions so
	 * if we get something we don't expect, the kernel will hardcrash.
	 */
	vector = _syspage_ptr->un.ppc.exceptptr;
	for(idx = PPC_EXC_SIZE/sizeof(uint32_t); idx > 0; ++vector, --idx) {
		*vector = 0x48000003 | PPC_LOW_CODE_START;
		icache_flush((uintptr_t)vector);
	}
	trap_install(PPC_LOW_CODE_START/4, __exc_unexpected, &__common_exc_entry);
}

uint32_t *
exc_vector_address(unsigned idx, unsigned size) {
	unsigned	sv = idx & SHORT_VECTOR;
	unsigned	end;

	idx &= ~SHORT_VECTOR;
	if(idx < PPC_LOW_CODE_START/4) {
		unsigned	freelocal;
		uint32_t	*p;

		freelocal = sv ? 4 : 0x100 - ((idx*4) & (0x100-1));
		if(size > freelocal) {
			// "ba *low_code"
			p = &_syspage_ptr->un.ppc.exceptptr[idx];
			*p = 0x48000002 | low_code;
			icache_flush((uintptr_t)p);
			idx = low_code / 4;
		}
	}
	end = (idx * 4) + size;
	if(end > low_code) low_code = end;
	return &_syspage_ptr->un.ppc.exceptptr[idx];
}

void
exc_intr_install(struct intrinfo_entry *iip, void *handler, void **info) {
	const struct exc_copy_block	*intr;
	unsigned					idx;

	idx = iip->cpu_intr_base;
	if(iip->flags & PPC_INTR_FLAG_CI) {
		intr = &intr_entry_critical;
		if(intr->size == 0) {
			kprintf("Critical interrupts not supported by this CPU (exc vector=0x%x)\n", idx*4);
			crash();
		}
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
	if(iip->flags & PPC_INTR_FLAG_SHORTVEC) idx |= SHORT_VECTOR;
	if(*info == 0) {
		*info = trap_install(idx, handler, intr);
	} else {
		// put things where they went before
		trap_install(((uintptr_t)*info - (uintptr_t)_syspage_ptr->un.ppc.exceptptr)/4,
						handler, intr);
	}
}

void
exc_report_unexpected(unsigned vector) {
	kprintf("Unexpected exception vector usage (0x%x)\n"
			"Check startup code for missing interrupt callout definitions.\n",
			vector - 4);
}

__SRCVERSION("exc_vector.c $Rev: 153052 $");
