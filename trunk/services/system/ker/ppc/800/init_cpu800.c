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

extern void __exc_alignment800();
extern void __exc_machine_check800();
extern void __exc_program800();
extern void __exc_trace800();
extern void __exc_emulation800();
extern void __exc_dbreak800();
extern void __exc_ibreak800();
extern void __exc_pbreak800();
extern void __exc_devport800();
extern void	__exc_instr_access800();
extern void	__exc_data_access800();
	

static const struct trap_entry ppc_traps[] = {
	{PPC_EXC_SYSTEM_CALL, __ker_entry, &__ker_entry_exc},
	{PPC_EXC_ALIGNMENT,	 __exc_alignment800, &__common_exc_entry},
	{PPC_EXC_MACHINE_CHECK, __exc_machine_check800, &__common_exc_entry},
	{PPC_EXC_PROGRAM, __exc_program800, &__common_exc_entry},
	{PPC_EXC_TRACE,	 __exc_trace800, &__common_exc_entry},
	{PPC800_EXC_ID_SOFTEMU, __exc_emulation800, &__common_exc_entry},
	{PPC800_EXC_ID_DBRKPT, __exc_dbreak800, &__common_exc_entry},
	{PPC800_EXC_ID_IBRKPT, __exc_ibreak800, &__common_exc_entry},
	{PPC800_EXC_ID_PBRKPT, __exc_pbreak800, &__common_exc_entry},
	{PPC800_EXC_ID_NM_DEVPORT, __exc_devport800, &__common_exc_entry},
	
	{PPC_EXC_INSTR_ACCESS, __exc_instr_access800, &__common_exc_entry},
	{PPC_EXC_DATA_ACCESS, __exc_data_access800, &__common_exc_entry},

	{PPC800_EXC_ID_ITLB_ERR, __exc_instr_access800, &__common_exc_entry},
	{PPC800_EXC_ID_DTLB_ERR, __exc_data_access800, &__common_exc_entry},
};
	
void
config_cpu(unsigned pvr, const struct exc_copy_block **entry, const struct exc_copy_block **exitlocal) {
	trap_install_set(ppc_traps, NUM_ELTS(ppc_traps));

	// no extra save/restore code needed.
	*entry = NULL;
	*exitlocal = NULL;
}

unsigned
determine_family(unsigned pvr) {

	//NOTE: We shouldn't have to add any more entries to these tables.
	//The startup program should be filling the family member indicator
	//and __cpu_flags bits properly.

	switch(PPC_GET_FAM_MEMBER(pvr)) {
	case PPC_8xx:	return PPC_FAMILY_800;
	default: break;
	}
	return PPC_FAMILY_UNKNOWN;
}

__SRCVERSION("init_cpu800.c $Rev: 170836 $");
