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

extern const struct exc_copy_block __exc_data_access900;
extern const struct exc_copy_block __exc_instr_access900;
extern const struct exc_copy_block __exc_ffpu;
extern const struct exc_copy_block __exc_fvmx;
extern const struct exc_copy_block ctx_save_ear, ctx_restore_ear;
extern const struct exc_copy_block ctx_save_vmx;
extern const struct exc_copy_block __exc_data_segment900;
extern const struct exc_copy_block __exc_instr_segment900;

extern void __exc_alignment700();
extern void __exc_machine_check700();
extern void __exc_program700();
extern void __exc_trace700();
extern void __exc_ibreak700();
extern void __exc_fpu_unavail();
extern void __exc_fpu_emulation();
extern void __exc_system_mgt700();

extern void __exc_vmx_assist();

static const struct trap_entry ppc_traps[] = {
	{PPC_EXC_SYSTEM_CALL, __ker_entry, &__ker_entry_exc},
	{PPC_EXC_ALIGNMENT, __exc_alignment700, &__common_exc_entry},
	{PPC_EXC_MACHINE_CHECK, __exc_machine_check700, &__common_exc_entry},
	{PPC_EXC_PROGRAM, __exc_program700,  &__common_exc_entry},
	{PPC_EXC_TRACE, __exc_trace700, &__common_exc_entry},
	{PPC700_EXC_IABREAKPOINT, __exc_ibreak700, &__common_exc_entry},
	{PPC_EXC_INSTR_SEGMENT, NULL, &__exc_instr_segment900},
	{PPC_EXC_DATA_SEGMENT,  NULL, &__exc_data_segment900},
	{PPC_EXC_INSTR_ACCESS,  NULL, &__exc_instr_access900},
	{PPC_EXC_DATA_ACCESS,   NULL, &__exc_data_access900}
};

static const struct trap_entry ppc_traps_altivec[] = {
	{PPC_EXC_VMX_UNAVAILABLE, NULL, &__exc_fvmx},
	{PPC64_EXC_VMX_ASSIST, __exc_vmx_assist, &__common_exc_entry},
};


void
config_cpu(unsigned pvr, const struct exc_copy_block **entry, const struct exc_copy_block **exitlocal) {

	// The 970 doesn't support the "tlbia" instruction, which is what
	// tlb_flush_all defaults to using. However, the tlb_flush_all_64 
	// routine doesn't flush enough entries. We can get away without defining
	// a proper routine however because the only thing this is used for is
	// handling IPI_TLB_FLUSH and that is only needed if PPC_CPU_SW_TLBSYNC
	// is on, which it won't be for any 900 series processor.
//	tlb_flush_all = tlb_flush_all_64;
	CRASHCHECK(__cpu_flags & PPC_CPU_SW_TLBSYNC);

	trap_install_set(ppc_traps, NUM_ELTS(ppc_traps));

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
	return PPC_FAMILY_UNKNOWN;
}
