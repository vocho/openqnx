/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <stdint.h>
#include <ppc/603cpu.h>
#include <ppc/400cpu.h>
#include <ppc/440cpu.h>
#include <ppc/e500cpu.h>
#include <ppc/700cpu.h>
#include <ppc/970cpu.h>
#include <sys/syspage.h>
#include <kernel/nto.h>
#include "kdintl.h"

//ZZZ Need to have generic allocation routine.
//ZZZ This will work for right now...
#define ALLOC(s)	(void *)(uintptr_t)alloc_pmem((s), 0)

extern void ppcbke_tlb_read_ibm(int tlb, int idx, ppcbke_tlb_t *);
extern void ppcbke_tlb_write_ibm(int tlb, int idx, const ppcbke_tlb_t *);
extern void ppcbke_tlb_read_e500(int tlb, int idx, ppcbke_tlb_t *);
extern void ppcbke_tlb_write_e500(int tlb, int idx, const ppcbke_tlb_t *);
extern void ppcbke_tlb_read_e500v2(int tlb, int idx, ppcbke_tlb_t *);
extern void ppcbke_tlb_write_e500v2(int tlb, int idx, const ppcbke_tlb_t *);

static void
ppcbke_tlb_read_dummy(int tlb, int idx, ppcbke_tlb_t *entry) {
	// Don't know how to read entry
}

static void
ppcbke_tlb_write_dummy(int tlb, int idx, const ppcbke_tlb_t *entry) {
	// Don't know how to write entry
}

void	(*bke_tlb_read)(int, int, ppcbke_tlb_t *) = ppcbke_tlb_read_dummy;
void	(*bke_tlb_write)(int, int, const ppcbke_tlb_t *) = ppcbke_tlb_write_dummy;


static void
extra_dummy(struct cpu_extra_state *state) {
	// nothing to do
}


static void
extra_440_save(struct cpu_extra_state *state) {
	state->u.ppc440.spr.ccr0 = get_spr(PPC440_SPR_CCR0);
	state->u.ppc440.spr.mmucr = get_spr(PPC440_SPR_MMUCR);
	state->u.ppc440.spr.rstcfg = get_spr(PPC440_SPR_RSTCFG);
}


static void
extra_440_restore(struct cpu_extra_state *state) {
	set_spr(PPC440_SPR_MMUCR, state->u.ppc440.spr.mmucr);
}


static void
extra_e500_save(struct cpu_extra_state *state) {
	//NYI: Should look at MMUCFG to decide how many pid registers
	//and if we need to save MAS5
	state->u.ppcbkem.spr.mas[0] = get_spr(PPCBKEM_SPR_MAS0);
	state->u.ppcbkem.spr.mas[1] = get_spr(PPCBKEM_SPR_MAS1);
	state->u.ppcbkem.spr.mas[2] = get_spr(PPCBKEM_SPR_MAS2);
	state->u.ppcbkem.spr.mas[3] = get_spr(PPCBKEM_SPR_MAS3);
	state->u.ppcbkem.spr.mas[4] = get_spr(PPCBKEM_SPR_MAS4);
//	state->u.ppcbkem.spr.mas[5] = get_spr(PPCBKEM_SPR_MAS5);
	state->u.ppcbkem.spr.mas[6] = get_spr(PPCBKEM_SPR_MAS6);
	state->u.ppcbkem.spr.pid1   = get_spr(PPCBKEM_SPR_PID1);
	state->u.ppcbkem.spr.pid2   = get_spr(PPCBKEM_SPR_PID2);

	state->u.ppce500.spr.ivor32 = get_spr(PPCE500_SPR_IVOR32);
	state->u.ppce500.spr.ivor33 = get_spr(PPCE500_SPR_IVOR33);
	state->u.ppce500.spr.ivor34 = get_spr(PPCE500_SPR_IVOR34);
	state->u.ppce500.spr.ivor35 = get_spr(PPCE500_SPR_IVOR35);
	state->u.ppce500.spr.hid0   = get_spr(PPCE500_SPR_HID0);
	state->u.ppce500.spr.hid1   = get_spr(PPCE500_SPR_HID1);
	state->u.ppce500.spr.bucsr  = get_spr(PPCE500_SPR_BUCSR);
}


static void
extra_e500v2_save(struct cpu_extra_state *state) {
	extra_e500_save(state);
	state->u.ppcbkem.spr.mas[7] = get_spr(PPCBKEM_SPR_MAS7);
}


static void
extra_e500_restore(struct cpu_extra_state *state) {
	set_spr(PPCBKEM_SPR_MAS0, state->u.ppcbkem.spr.mas[0]);
	set_spr(PPCBKEM_SPR_MAS1, state->u.ppcbkem.spr.mas[1]);
	set_spr(PPCBKEM_SPR_MAS2, state->u.ppcbkem.spr.mas[2]);
	set_spr(PPCBKEM_SPR_MAS3, state->u.ppcbkem.spr.mas[3]);
}


static void
extra_e500v2_restore(struct cpu_extra_state *state) {
	extra_e500_restore(state);
	set_spr(PPCBKEM_SPR_MAS7, state->u.ppcbkem.spr.mas[7]);
}


static void
extra_970_save(struct cpu_extra_state *state) {
	state->u.ppc900.hid[4] = get_spr64(PPC970_SPR_HID4);
	state->u.ppc900.hid[5] = get_spr64(PPC970_SPR_HID5);
	state->u.ppc970.dabrx = get_spr64(PPC970_SPR_DABRX);
	state->u.ppc970.scomc = get_spr64(PPC970_SPR_SCOMC);
	state->u.ppc970.scomd = get_spr64(PPC970_SPR_SCOMD);
}

static void
extra_74xx_save(struct cpu_extra_state *state) {
    state->u.ppc600.pir = get_spr(PPC700_SPR_PIR);
    state->u.ppc600.ldstcr = get_spr(PPC700_SPR_LDSTCR);
    state->u.ppc600.ictrl = get_spr(PPC700_SPR_ICTRL);
    state->u.ppc600.l2cr = get_spr(PPC700_SPR_L2CR);
	state->u.ppc600.l3cr = get_spr(PPC700_SPR_L3CR);
    state->u.ppc600.msscr0 = get_spr(PPC700_SPR_MSSCR0);
    state->u.ppc600.msssr0 = get_spr(PPC700_SPR_MSSSR0);
}


static void		(*extra_chip_save   )(struct cpu_extra_state *) = extra_dummy;
static void		(*extra_chip_restore)(struct cpu_extra_state *) = extra_dummy;


void
cpu_init_extra(struct cpu_extra_state *state) {
	struct ppc_kerinfo_entry	*ker;
	struct ppc_tlbinfo_entry	*tlbinfo;
	unsigned					num;
	unsigned					i;
	unsigned					pvr;

	ker = SYSPAGE_CPU_ENTRY(ppc, kerinfo);
	pvr = ker->pretend_cpu;
	if(pvr == 0) pvr = SYSPAGE_ENTRY(cpuinfo)->cpu;
	switch(ker->ppc_family) {
	case PPC_FAMILY_400:	
		if(SYSPAGE_ENTRY(cpuinfo)->flags & CPU_FLAG_MMU) {
			extern void clr_tlb400(unsigned);

			state->u.ppc400.tlb.num_entries = 64;
			state->u.ppc400.tlb.entry = ALLOC(64 * sizeof(*state->u.ppc400.tlb.entry));
			for(i = 0; i < state->u.ppc400.tlb.num_entries; ++i) {
				clr_tlb400(i);
			}
		}
		break;
	case PPC_FAMILY_600:
		switch(PPC_GET_FAM_MEMBER(pvr)) {
		case PPC_7450:
		case PPC_7455:
			extra_chip_save    = extra_74xx_save;
			break;
		}
		break;
	case PPC_FAMILY_booke:	
		tlbinfo = SYSPAGE_CPU_ENTRY(ppc, tlbinfo);
		num = _syspage_ptr->un.ppc.tlbinfo.entry_size / sizeof(*tlbinfo);
		for(i = 0; i < num; ++i, ++tlbinfo) {
			state->u.ppcbke.tlb[i].num_entries = tlbinfo->num_entries;
			state->u.ppcbke.tlb[i].entry = ALLOC(tlbinfo->num_entries * sizeof(ppcbke_tlb_t));
		}
		switch(PPC_GET_FAM_MEMBER(pvr)) {
		case PPC_440GP:
		case PPC_440GX:
		case PPC_440EP:
			extra_chip_save    = extra_440_save;
			extra_chip_restore = extra_440_restore;
			bke_tlb_read = ppcbke_tlb_read_ibm;
			bke_tlb_write = ppcbke_tlb_write_ibm;
			break;
		case PPC_E500:
			extra_chip_save    = extra_e500_save;
			extra_chip_restore = extra_e500_restore;
			bke_tlb_read = ppcbke_tlb_read_e500;
			bke_tlb_write = ppcbke_tlb_write_e500;
			break;
		case PPC_E500V2:
			extra_chip_save    = extra_e500v2_save;
			extra_chip_restore = extra_e500v2_restore;
			bke_tlb_read = ppcbke_tlb_read_e500v2;
			bke_tlb_write = ppcbke_tlb_write_e500v2;
			break;
		}
		break;
	case PPC_FAMILY_900:	
		switch(PPC_GET_FAM_MEMBER(pvr)) {
		case PPC_970FX:
			extra_chip_save    = extra_970_save;
			break;
		}
	}
}


void
cpu_save_extra(struct cpu_extra_state *state) {
	unsigned		i;
	unsigned		j;
	unsigned		num_entries;

	switch(SYSPAGE_CPU_ENTRY(ppc, kerinfo)->ppc_family) {
	case PPC_FAMILY_400:
		state->u.ppc400.spr.dear = get_spr(PPC400_SPR_DEAR);
		state->u.ppc400.spr.esr = get_spr(PPC400_SPR_ESR);
		if(SYSPAGE_ENTRY(cpuinfo)->flags & CPU_FLAG_MMU) {
			state->u.ppc400.spr.pid = get_spr(PPC400_SPR_PID);
			state->u.ppc400.spr.zpr = get_spr(PPC400_SPR_ZPR);
			for(i = 0; i < state->u.ppc400.tlb.num_entries; ++i) {
				extern void get_tlb400();
		
				get_tlb400(i, &state->u.ppc400.tlb.entry[i]);
			}
		}
		break;
	case PPC_FAMILY_600:
		state->u.ppc600.hid0 = get_spr(PPC603_SPR_HID0);
		state->u.ppc600.hid1 = get_spr(PPC603_SPR_HID1);
		state->u.ppc600.dbat[0].lo = get_spr(PPC_SPR_DBAT0L);
		state->u.ppc600.dbat[0].up = get_spr(PPC_SPR_DBAT0U);
		state->u.ppc600.dbat[1].lo = get_spr(PPC_SPR_DBAT1L);
		state->u.ppc600.dbat[1].up = get_spr(PPC_SPR_DBAT1U);
		state->u.ppc600.dbat[2].lo = get_spr(PPC_SPR_DBAT2L);
		state->u.ppc600.dbat[2].up = get_spr(PPC_SPR_DBAT2U);
		state->u.ppc600.dbat[3].lo = get_spr(PPC_SPR_DBAT3L);
		state->u.ppc600.dbat[3].up = get_spr(PPC_SPR_DBAT3U);
		state->u.ppc600.ibat[0].lo = get_spr(PPC_SPR_IBAT0L);
		state->u.ppc600.ibat[0].up = get_spr(PPC_SPR_IBAT0U);
		state->u.ppc600.ibat[1].lo = get_spr(PPC_SPR_IBAT1L);
		state->u.ppc600.ibat[1].up = get_spr(PPC_SPR_IBAT1U);
		state->u.ppc600.ibat[2].lo = get_spr(PPC_SPR_IBAT2L);
		state->u.ppc600.ibat[2].up = get_spr(PPC_SPR_IBAT2U);
		state->u.ppc600.ibat[3].lo = get_spr(PPC_SPR_IBAT3L);
		state->u.ppc600.ibat[3].up = get_spr(PPC_SPR_IBAT3U);
		if(SYSPAGE_ENTRY(cpuinfo)->flags & PPC_CPU_EXTRA_BAT) {
			state->u.ppc600.dbat[4].lo = get_spr(PPC_SPR_DBAT4L);
			state->u.ppc600.dbat[4].up = get_spr(PPC_SPR_DBAT4U);
			state->u.ppc600.dbat[5].lo = get_spr(PPC_SPR_DBAT5L);
			state->u.ppc600.dbat[5].up = get_spr(PPC_SPR_DBAT5U);
			state->u.ppc600.dbat[6].lo = get_spr(PPC_SPR_DBAT6L);
			state->u.ppc600.dbat[6].up = get_spr(PPC_SPR_DBAT6U);
			state->u.ppc600.dbat[7].lo = get_spr(PPC_SPR_DBAT7L);
			state->u.ppc600.dbat[7].up = get_spr(PPC_SPR_DBAT7U);
			state->u.ppc600.ibat[4].lo = get_spr(PPC_SPR_IBAT4L);
			state->u.ppc600.ibat[4].up = get_spr(PPC_SPR_IBAT4U);
			state->u.ppc600.ibat[5].lo = get_spr(PPC_SPR_IBAT5L);
			state->u.ppc600.ibat[5].up = get_spr(PPC_SPR_IBAT5U);
			state->u.ppc600.ibat[6].lo = get_spr(PPC_SPR_IBAT6L);
			state->u.ppc600.ibat[6].up = get_spr(PPC_SPR_IBAT6U);
			state->u.ppc600.ibat[7].lo = get_spr(PPC_SPR_IBAT7L);
			state->u.ppc600.ibat[7].up = get_spr(PPC_SPR_IBAT7U);
		}
    	state->u.ppc600.srr0 = get_spr(PPC_SPR_SRR0);
    	state->u.ppc600.srr1 = get_spr(PPC_SPR_SRR1);
   		for (i = 0; i < 16; i++) {
        	state->u.ppc600.sr[i] = get_sreg(i << 28);
    	}

    	state->u.ppc600.sprg0 = get_spr(PPC_SPR_SPRG0);
    	state->u.ppc600.sprg1 = get_spr(PPC_SPR_SPRG1);
    	state->u.ppc600.sprg2 = get_spr(PPC_SPR_SPRG2);
    	state->u.ppc600.sprg3 = get_spr(PPC_SPR_SPRG3);

		break;
	case PPC_FAMILY_900:
		state->u.ppc900.hid[0] = get_spr64(PPC603_SPR_HID0);
		state->u.ppc900.hid[1] = get_spr64(PPC603_SPR_HID1);
		state->u.ppc900.dsisr = get_spr64(PPC_SPR_DSISR);
		state->u.ppc900.dar = get_spr64(PPC_SPR_DAR);
		state->u.ppc900.sdr1 = get_spr64(PPC_SPR_SDR1);
//PA6T doesn't seem to have ACCR, but AMR uses same SPR number?		
//		state->u.ppc900.accr = get_spr64(PPC_SPR_ACCR);
//PA6T hangs up on read of CTRL register		
//		state->u.ppc900.ctrl = get_spr64(PPC_SPR_CTRL);
		state->u.ppc900.asr = get_spr64(PPC_SPR_ASR);
		state->u.ppc900.pir = get_spr64(PPC700_SPR_PIR);
		for(i = 0; i < 16; ++i) {
			ppc_slbmfev(state->u.ppc900.slb[i].v, i);
			ppc_slbmfee(state->u.ppc900.slb[i].e, i);
		}
		break;

	case PPC_FAMILY_800:
		// nothing to do (for now)
		break;
	case PPC_FAMILY_booke:
		state->u.ppcbke.spr.dbcr0 = get_spr(PPCBKE_SPR_DBCR0);
		state->u.ppcbke.spr.dbcr1 = get_spr(PPCBKE_SPR_DBCR1);
		state->u.ppcbke.spr.dbcr2 = get_spr(PPCBKE_SPR_DBCR2);
		state->u.ppcbke.spr.dbsr = get_spr(PPCBKE_SPR_DBSR);
		state->u.ppcbke.spr.dear = get_spr(PPCBKE_SPR_DEAR);
		state->u.ppcbke.spr.esr = get_spr(PPCBKE_SPR_ESR);
		state->u.ppcbke.spr.ivpr = get_spr(PPCBKE_SPR_IVPR);
		state->u.ppcbke.spr.ivor[0] = get_spr(PPCBKE_SPR_IVOR0);
		state->u.ppcbke.spr.ivor[1] = get_spr(PPCBKE_SPR_IVOR1);
		state->u.ppcbke.spr.ivor[2] = get_spr(PPCBKE_SPR_IVOR2);
		state->u.ppcbke.spr.ivor[3] = get_spr(PPCBKE_SPR_IVOR3);
		state->u.ppcbke.spr.ivor[4] = get_spr(PPCBKE_SPR_IVOR4);
		state->u.ppcbke.spr.ivor[5] = get_spr(PPCBKE_SPR_IVOR5);
		state->u.ppcbke.spr.ivor[6] = get_spr(PPCBKE_SPR_IVOR6);
		state->u.ppcbke.spr.ivor[7] = get_spr(PPCBKE_SPR_IVOR7);
		state->u.ppcbke.spr.ivor[8] = get_spr(PPCBKE_SPR_IVOR8);
		state->u.ppcbke.spr.ivor[9] = get_spr(PPCBKE_SPR_IVOR9);
		state->u.ppcbke.spr.ivor[10] = get_spr(PPCBKE_SPR_IVOR10);
		state->u.ppcbke.spr.ivor[11] = get_spr(PPCBKE_SPR_IVOR11);
		state->u.ppcbke.spr.ivor[12] = get_spr(PPCBKE_SPR_IVOR12);
		state->u.ppcbke.spr.ivor[13] = get_spr(PPCBKE_SPR_IVOR13);
		state->u.ppcbke.spr.ivor[14] = get_spr(PPCBKE_SPR_IVOR14);
		state->u.ppcbke.spr.ivor[15] = get_spr(PPCBKE_SPR_IVOR15);
		state->u.ppcbke.spr.pid = get_spr(PPCBKE_SPR_PID);
		state->u.ppcbke.spr.pir = get_spr(PPCBKE_SPR_PIR);
		state->u.ppcbke.spr.sprg4 = get_spr(PPCBKE_SPR_SPRG4RO);
		state->u.ppcbke.spr.sprg5 = get_spr(PPCBKE_SPR_SPRG5RO);
		state->u.ppcbke.spr.sprg6 = get_spr(PPCBKE_SPR_SPRG6RO);
		state->u.ppcbke.spr.sprg7 = get_spr(PPCBKE_SPR_SPRG7RO);
		state->u.ppcbke.spr.tcr = get_spr(PPCBKE_SPR_TCR);
		state->u.ppcbke.spr.tsr = get_spr(PPCBKE_SPR_TSR);
	
		i = 0;
		for(;;) {
			num_entries = state->u.ppcbke.tlb[i].num_entries;
			if(num_entries == 0) break;
			for(j = 0; j < num_entries; ++j) {
				bke_tlb_read(i, j, &state->u.ppcbke.tlb[i].entry[j]);
			}
			++i;
		}
		break;
	}
	extra_chip_save(state);
}


void
cpu_restore_extra(struct cpu_extra_state *state) {
	if(mmu_needs_restoring) {
		switch(SYSPAGE_CPU_ENTRY(ppc, kerinfo)->ppc_family) {
		case PPC_FAMILY_600:
			set_spr(PPC_SPR_DBAT3L, state->u.ppc600.dbat[3].lo);
			set_spr(PPC_SPR_DBAT3U, state->u.ppc600.dbat[3].up);
			break;
		}
	}
	extra_chip_restore(state);
	mmu_needs_restoring = 0;
}
