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

/*
 * Altivec support routines for the kernel. Basically allocates and frees
 * Altivec contexts
 */

#include "externs.h"
#include <ppc/context.h>

/*
 * ALT context needs to be 16-byte aligned, but our allocator will only give
 * us 8-byte alignement. We allocate extra for the objects, and align to the 
 * next 16 byte boundary. We store the original pointer at alt-4.
 */

#define ALIGN_ALT(ptr)			(void *)((unsigned)((((char *)ptr)+(2*ALT_CONTEXT_ALIGN)-1))&~(ALT_CONTEXT_ALIGN-1))

void
alt_context_alloc(void) {
	void 		*alt;
	THREAD		*act = actives[KERNCPU];

	if(act->cpu.alt) crash();
	if((alt = object_alloc(NULL, &alt_souls)) == NULL) {
		usr_fault(SIGFPE + (FPE_NOMEM*256) + (FLTFPE*65536), act, KIP(act));
		return;
	}
	act->cpu.alt = alt;
	alt_context_init(act);
}

void
alt_context_free(THREAD *thp) {
	int			i;

	if(thp->cpu.alt == NULL) crash();

	// Make sure no-one writes to that context anymore
	for(i = 0; i < NUM_PROCESSORS; i++) {
		if(actives_alt[i] == thp->cpu.alt) actives_alt[i] = NULL;
	}
	object_free(NULL, &alt_souls, thp->cpu.alt);
	thp->cpu.alt = NULL;
}

__SRCVERSION("nano_altivec.c $Rev: 153052 $");
