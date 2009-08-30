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
#include <ppc/440cpu.h>
#include <ppc/e500cpu.h>

extern void __exc_instr_access_booke();

extern void __exc_alignment_booke();
extern void __exc_data_access_booke();
extern void __exc_debug_booke();
extern void __exc_program_booke();

extern void __exc_machine_check_440gp();
extern void __exc_machine_check_e500();

extern void __exc_spe_unavail();
extern void __exc_spe_data();
extern void __exc_spe_round();

extern void __exc_fpu_unavail();
extern void __exc_fpu_emulation();

extern const struct exc_copy_block	__exc_entry_spe_unavail;
extern const struct exc_copy_block	__exc_entry_machine_check_booke;
extern const struct exc_copy_block	__exc_entry_machine_check_e500;
extern const struct exc_copy_block	__exc_entry_machine_check_440gx;
extern const struct exc_copy_block	__exc_entry_debug;
extern const struct exc_copy_block  __exc_ffpu;

extern const struct exc_copy_block	ctx_save_usprg0;
extern const struct exc_copy_block	ctx_restore_usprg0;

extern const struct exc_copy_block	ctx_save_e500_extra;
extern const struct exc_copy_block	ctx_restore_e500_extra;

// NOTE: The ker/ppc/booke/vm_e500.s has a workaround for the CPU29
// errata in the mmu_on code that assumes it is being immediately followed
// by a BLR instruction - that means that exceptions be used by the E500
// always need to specify a non-NULL value for the second level exception
// handling code (third value in the struct trap_entry).
static const struct trap_entry traps_booke[] = {
	{PPCBKE_SPR_IVOR8,  __ker_entry, &__ker_entry_exc},
	{PPCBKE_SPR_IVOR5,  __exc_alignment_booke, &__common_exc_entry},
	{PPCBKE_SPR_IVOR6,  __exc_program_booke, &__common_exc_entry},
	{PPCBKE_SPR_IVOR3,  __exc_instr_access_booke, &__common_exc_entry},
	{PPCBKE_SPR_IVOR2,  __exc_data_access_booke, &__common_exc_entry},
	{PPCBKE_SPR_IVOR15, __exc_debug_booke, &__exc_entry_debug},
};

static const struct trap_entry traps_440gp[] = {
	{PPCBKE_SPR_IVOR1,  __exc_machine_check_440gp, &__exc_entry_machine_check_booke},
};

static const struct trap_entry traps_440gx[] = {
	{PPCBKE_SPR_IVOR1,  __exc_machine_check_440gp, &__exc_entry_machine_check_440gx},
};

static const struct trap_entry traps_e500[] = {
	{PPCBKE_SPR_IVOR1,  __exc_machine_check_e500, &__exc_entry_machine_check_e500},
	{PPCE500_SPR_IVOR32,__exc_spe_unavail,	&__exc_entry_spe_unavail},
	{PPCE500_SPR_IVOR33,__exc_spe_data, 	&__common_exc_entry},
	{PPCE500_SPR_IVOR34,__exc_spe_round, 	&__common_exc_entry},
};


// TEMP KLUDGE
static void
fix_pgsizes(void) {
	struct ppc_tlbinfo_entry	*tlbinfo = SYSPAGE_CPU_ENTRY(ppc, tlbinfo);

	// Old startups set the supported page sizes for the IBM 440 to 0x3ff, but
	// it really should be 0x2bf. After a suitable interval of time, this
	// code can be removed.
	// bstecher 2006/07/06
	if(tlbinfo->page_sizes == 0x3ff) tlbinfo->page_sizes = 0x2bf;
}
// END TEMP KLUDGE
 
	
void
config_cpu(unsigned pvr, const struct exc_copy_block **entry, const struct exc_copy_block **exitlocal) {
	set_spr( PPCBKE_SPR_DBCR0, 0 );
#ifdef VARIANT_booke
	SYSPAGE_CPU_ENTRY(ppc,kerinfo)->init_msr |= PPC_MSR_DE;
#endif

	switch(PPC_GET_FAM_MEMBER(pvr)) {
	case PPC_440GP:
		ppcbke_tlb_select = PPCBKE_TLB_SELECT_IBM;
		trap_install_set(traps_440gp, NUM_ELTS(traps_440gp));
fix_pgsizes();
		break;
	case PPC_440GX:
		ppcbke_tlb_select = PPCBKE_TLB_SELECT_IBM;
		trap_install_set(traps_440gx, NUM_ELTS(traps_440gx));
fix_pgsizes();
		break;
	case PPC_E500V2:
		ppcbke_tlb_select = PPCBKE_TLB_SELECT_E500v2;
		trap_install_set(traps_e500, NUM_ELTS(traps_e500));
		alt_souls.size = sizeof(PPC_SPE_REGISTERS);
		*entry++ = &ctx_save_e500_extra;
		*exitlocal++ = &ctx_restore_e500_extra;
		break;
	case PPC_E500:
		ppcbke_tlb_select = PPCBKE_TLB_SELECT_E500;
		trap_install_set(traps_e500, NUM_ELTS(traps_e500));
		alt_souls.size = sizeof(PPC_SPE_REGISTERS);
		*entry++ = &ctx_save_e500_extra;
		*exitlocal++ = &ctx_restore_e500_extra;
		break;
	default:
		kprintf("Unsupported PVR value: %x\n", pvr);
		crash();
		break;
	}

	trap_install_set(traps_booke, NUM_ELTS(traps_booke));

	if(__cpu_flags & CPU_FLAG_FPU) {
	    if(fpuemul) {
	        // Emulation
			trap_install(PPCBKE_SPR_IVOR7,
						 __exc_fpu_emulation, &__common_exc_entry);
        } else {
            // Real floating point
			trap_install(PPCBKE_SPR_IVOR7,
						 __exc_fpu_unavail, &__exc_ffpu);

		}
	}

	// Make data & instruction TLB misses go to the
	// data and instruction storage exceptions. This will
	// be changed if/when copy_vm_code() gets called and
	// we know we're running in virtual mode.
	set_spr(PPCBKE_SPR_IVOR13, get_spr(PPCBKE_SPR_IVOR2));
	set_spr(PPCBKE_SPR_IVOR14, get_spr(PPCBKE_SPR_IVOR3));

	*entry++ = &ctx_save_usprg0;
	*exitlocal++ = &ctx_restore_usprg0;
	*entry = NULL;
	*exitlocal = NULL;

	ppc_ienable_bits |= PPC_MSR_CE | PPC_MSR_ME;
}

unsigned
determine_family(unsigned pvr) {
	// Nothing should be added to this function - startup should
	// fill things in properly.
	return PPC_FAMILY_UNKNOWN;
}

__SRCVERSION("config_cpu.c $Rev: 164330 $");
