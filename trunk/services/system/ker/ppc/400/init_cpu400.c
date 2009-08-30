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
#include <ppc/400cpu.h>

extern const struct exc_copy_block __common_exc_entry_405workaround;
extern const struct exc_copy_block __exc_entry_machine_check;
extern const struct exc_copy_block __exc_entry_debug;

extern void __exc_alignment400();
extern void __exc_instr_access400();
extern void __exc_data_access400();
extern void __exc_debug400();
extern void __exc_program400();

extern void __exc_machine_check403();
extern void __exc_machine_check405();

static const struct trap_entry ppc_traps[] = {
	{PPC_EXC_SYSTEM_CALL, __ker_entry, &__ker_entry_exc},
	{PPC_EXC_ALIGNMENT, __exc_alignment400, &__common_exc_entry_405workaround},
	{PPC_EXC_PROGRAM, __exc_program400, &__common_exc_entry},
	{PPC_EXC_INSTR_ACCESS, __exc_instr_access400, &__common_exc_entry},
	{PPC_EXC_DATA_ACCESS, __exc_data_access400, &__common_exc_entry_405workaround},
	{PPC400_EXC_DEBUG, __exc_debug400, &__exc_entry_debug},
};

static const struct trap_entry traps_403[] = {
	{PPC_EXC_MACHINE_CHECK, __exc_machine_check403, &__exc_entry_machine_check},
};

static const struct trap_entry traps_405[] = {
	{PPC_EXC_MACHINE_CHECK, __exc_machine_check405, &__exc_entry_machine_check},
};
	
void
config_cpu(unsigned pvr, const struct exc_copy_block **entry, const struct exc_copy_block **exitlocal) {
	set_spr(PPC400_SPR_DBCR, /*PPC400_DBCR_IC|*/PPC400_DBCR_IDM);

	switch(PPC_GET_FAM_MEMBER(pvr)) {
	case PPC_405:
		trap_install_set(traps_405, NUM_ELTS(traps_405));
		break;
	default:
		trap_install_set(traps_403, NUM_ELTS(traps_403));
		break;
	}

	trap_install_set(ppc_traps, NUM_ELTS(ppc_traps));

	// no extra save/restore code needed.
	*entry = NULL;
	*exitlocal = NULL;

	ppc_ienable_bits |= PPC_MSR_CE | PPC_MSR_ME;
}

unsigned
determine_family(unsigned pvr) {

	//NOTE: We shouldn't have to add anymore entries to these tables.
	//The startup program should be filling the family member indicator
	//properly.
	switch(PPC_GET_FAMILY(pvr)) {
	case PPC_400:	return PPC_FAMILY_400;
	case PPC_400B:	return PPC_FAMILY_400;
	case PPC_400C:	return PPC_FAMILY_400;
	default: break;
	}
	return PPC_FAMILY_UNKNOWN;
}

__SRCVERSION("init_cpu400.c $Rev: 170836 $");
