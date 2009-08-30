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
#include <ppc/700cpu.h>

extern void tlb_flush_all_64(void);

extern const struct exc_copy_block __exc_data_access700;
extern const struct exc_copy_block __exc_instr_access700;
extern const struct exc_copy_block __exc_ffpu;
extern const struct exc_copy_block __exc_fvmx;
extern const struct exc_copy_block ctx_save_ear, ctx_restore_ear;
extern const struct exc_copy_block ctx_save_vmx;

extern void __exc_alignment700();
extern void __exc_machine_check700();
extern void __exc_machine_check74xx();
extern void __exc_program700();
extern void __exc_trace700();
extern void __exc_ibreak700();
extern void __exc_access700();
extern void __exc_fpu_unavail();
extern void __exc_fpu_emulation();
extern void __exc_system_mgt700();

extern void __exc_instr_access603();
extern void __exc_data_access603();

extern void __exc_vmx_unavailable();
extern void __exc_vmx_assist();

static const struct trap_entry ppc_traps[] = {
	{PPC_EXC_SYSTEM_CALL, __ker_entry, &__ker_entry_exc},
	{PPC_EXC_ALIGNMENT, __exc_alignment700, &__common_exc_entry},
	{PPC_EXC_PROGRAM, __exc_program700,  &__common_exc_entry},
	{PPC_EXC_TRACE, __exc_trace700, &__common_exc_entry},
	{PPC700_EXC_IABREAKPOINT, __exc_ibreak700, &__common_exc_entry},
};

static const struct trap_entry ppc_traps700[] = {
	{PPC_EXC_INSTR_ACCESS, NULL, &__exc_instr_access700},
	{PPC_EXC_DATA_ACCESS, NULL,  &__exc_data_access700}
};

static const struct trap_entry ppc_traps603[] = {
	{PPC_EXC_INSTR_ACCESS, __exc_instr_access603,&__common_exc_entry},
	{PPC_EXC_DATA_ACCESS, __exc_data_access603, &__common_exc_entry}
};

static const struct trap_entry ppc_traps_altivec[] = {
	{PPC_EXC_VMX_UNAVAILABLE, NULL, &__exc_fvmx},
	{PPC_EXC_VMX_ASSIST, __exc_vmx_assist, &__common_exc_entry},
};


void
config_cpu(unsigned pvr, const struct exc_copy_block **entry, const struct exc_copy_block **exitlocal) {
	unsigned	type = PPC_GET_FAM_MEMBER(pvr);

	switch(type) {
	case PPC_750:
		tlb_flush_all = tlb_flush_all_64;
		break;
	default: break;
	}
	trap_install_set(ppc_traps, NUM_ELTS(ppc_traps));

	/*
	 * These are the exceptions which are different with/without a hash table
	 */
	if(__cpu_flags & PPC_CPU_HW_HT) {
		trap_install_set(ppc_traps700, NUM_ELTS(ppc_traps700));
	} else {
		trap_install_set(ppc_traps603, NUM_ELTS(ppc_traps603));
	}

	/*
	 * Install the Altivec exception handler if this CPU supports it.
	 */
	if(__cpu_flags & PPC_CPU_ALTIVEC) {
		trap_install_set(ppc_traps_altivec, NUM_ELTS(ppc_traps_altivec));
		alt_souls.size = sizeof(PPC_VMX_REGISTERS);
	}

	if(fpuemul) {
		// Emulation
		trap_install(PPC_EXC_FPU_UNAVAILABLE, __exc_fpu_emulation, &__common_exc_entry);
	} else {
		// Real floating point
		trap_install(PPC_EXC_FPU_UNAVAILABLE, __exc_fpu_unavail, &__exc_ffpu);
	}

	switch(type) {
	case PPC_7450:
	case PPC_7455:
		trap_install(PPC_EXC_MACHINE_CHECK, __exc_machine_check74xx, &__common_exc_entry);
		break;
	default: 
		trap_install(PPC_EXC_MACHINE_CHECK, __exc_machine_check700, &__common_exc_entry);
		break;
	}
	if(__cpu_flags & PPC_CPU_EAR) {
		*entry++ = &ctx_save_ear;
		*exitlocal++ = &ctx_restore_ear;
	}
	if(__cpu_flags & PPC_CPU_ALTIVEC) {
		*entry++ = &ctx_save_vmx;
	}
	*entry = NULL;
	*exitlocal = NULL;
}

unsigned
determine_family(unsigned pvr) {
	unsigned	type = PPC_GET_FAM_MEMBER(pvr);

	//NOTE: We shouldn't have to add any more entries to these tables.
	//The startup program should be filling the family member indicator
	//and __cpu_flags bits properly.

	if(!(__cpu_flags & PPC_CPU_SW_HT)) {
		switch(type) {
		case PPC_750:
		case PPC_604:
		case PPC_604e:
		case PPC_604e5:
		case PPC_7400:
		case PPC_7410:
		case PPC_7450:
		case PPC_7455:
		case PPC_7457:
			__cpu_flags |= PPC_CPU_HW_HT;
			break;
		default:
			__cpu_flags |= PPC_CPU_SW_HT;
		}
	}
	// an old style startup won't have set these properly.
	if(type == PPC_750) {
		__cpu_flags |= PPC_CPU_DCBZ_NONCOHERENT;
	} else {
		__cpu_flags |= PPC_CPU_TLB_SHADOW;
	}

	switch(type) {
	case PPC_750:
	case PPC_603e:
	case PPC_603e7:
	case PPC_604e:
	case PPC_604e5:
	case PPC_7400:
	case PPC_7410:
	case PPC_7450:
	case PPC_7455:
	case PPC_7457:
	case PPC_8260:
		__cpu_flags |= PPC_CPU_HW_POW;
		break;
	default: break;
	}

	switch(PPC_GET_FAMILY(pvr)) {
	case PPC_600:	return PPC_FAMILY_600;
	case PPC_82XX:	return PPC_FAMILY_600;
	case PPC_74XX:	return PPC_FAMILY_600;
	default: break;
	}
	return PPC_FAMILY_UNKNOWN;
}

__SRCVERSION("init_cpu600.c $Rev: 170836 $");
