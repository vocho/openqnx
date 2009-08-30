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
#include <ppc/800cpu.h>

extern const struct exc_copy_block __exc_itlb;
extern const struct exc_copy_block __exc_dtlb;
extern const struct exc_copy_block mmu_on;
extern const struct exc_copy_block mmu_off;

extern uint8_t *ppc_kerentrycom_ptr;

void
get_mmu_code(const struct exc_copy_block **on, const struct exc_copy_block **off) {
	*on  = &mmu_on;
	*off = &mmu_off;
}

void
copy_vm_code() {
	copy_code(&_syspage_ptr->un.ppc.exceptptr[PPC800_EXC_ID_DTLB_MISS], &__exc_dtlb);
	copy_code(&_syspage_ptr->un.ppc.exceptptr[PPC800_EXC_ID_ITLB_MISS], &__exc_itlb);
}

__SRCVERSION("init_vm800.c $Rev: 153052 $");
