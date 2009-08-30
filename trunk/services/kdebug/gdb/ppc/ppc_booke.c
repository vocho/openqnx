#include "kdebug.h"
#include <ppc/440cpu.h>
#include <ppc/e500cpu.h>


extern void exc_data_access_booke();
extern void exc_program_booke();
extern void exc_debug_booke();
extern void exc_machine_check_booke();
extern void exc_machine_check_e500();

#define BKE_EXC_ALIGNMENT		(0x100/4)
#define BKE_EXC_DATA_ACCESS		(0x110/4)
#define BKE_EXC_INSTR_ACCESS	(0x120/4)
#define BKE_EXC_PROGRAM			(0x130/4)
#define BKE_EXC_MACHINE_CHECK	(0x140/4)
#define BKE_EXC_DEBUG			(0x150/4)

static const struct trap_entry ppc_booke[] = {
	{BKE_EXC_ALIGNMENT,		exc_alignment},
	{BKE_EXC_DATA_ACCESS,	exc_data_access_booke},
	{BKE_EXC_INSTR_ACCESS,	exc_data_access_booke},
	{BKE_EXC_PROGRAM,		exc_program_booke},
	{BKE_EXC_DEBUG,			exc_debug_booke},
};

static const struct trap_entry ppc_mc_booke[] = {
	{BKE_EXC_MACHINE_CHECK, exc_machine_check_booke},
};

static const struct trap_entry ppc_mc_e500[] = {
	{BKE_EXC_MACHINE_CHECK, exc_machine_check_e500},
};


static int needs_icache_flush;


static void
flusher_440(uintptr_t mapped_vaddr, uintptr_t real_vaddr, unsigned len) {
	needs_icache_flush = 1;
	cache_flush(mapped_vaddr, len);
}


static void
family_stuff_booke(int type, CPU_REGISTERS *ctx) {
	switch(type) {
	case FAM_DBG_ENTRY:
		ctx->msr &= ~PPC_MSR_DE;

#ifdef DEBUG_GDB
		kprintf("DBSR=%x\n", get_spr(PPCBKE_SPR_DBSR));
#endif
		set_spr(PPCBKE_SPR_DBCR0, get_spr(PPCBKE_SPR_DBCR0) & ~PPCBKE_DBCR0_ICMP);
		set_spr(PPCBKE_SPR_DBSR, ~0L);
		set_spr(PPCBKE_SPR_ESR, 0);
		break;
	case FAM_DBG_EXIT_STEP:
		ctx->msr |= PPC_MSR_DE;
		set_spr(PPCBKE_SPR_DBCR0, PPCBKE_DBCR0_IDM | PPCBKE_DBCR0_ICMP);
		// fall through
	case FAM_DBG_EXIT_CONTINUE:
		if(needs_icache_flush) {
			needs_icache_flush = 0;
			// The 440 has a virtual instruction cache, so the icbi's being
			// done for breakpoints don't actually flush anything since
			// the MSR DS bit is off in kdebug and the IS bit is usually
			// on in procnto. Solve the problem by just flushing the whole
			// icache every time we leave kdebug and something's been written.
			asm volatile(
				".ifdef PPC_CPUOP_ENABLED;"
				".cpu 403;"
				".endif;"
				"iccci 0,0"
			);
		}
		break;
	}
}

int
family_init_booke(unsigned pvr) {
	msr_bits_off |= PPC_MSR_CE;

	set_spr(PPCBKE_SPR_IVOR5,  BKE_EXC_ALIGNMENT*4);
	set_spr(PPCBKE_SPR_IVOR2,  BKE_EXC_DATA_ACCESS*4);
	set_spr(PPCBKE_SPR_IVOR3,  BKE_EXC_INSTR_ACCESS*4);
	set_spr(PPCBKE_SPR_IVOR6,  BKE_EXC_PROGRAM*4);
	set_spr(PPCBKE_SPR_IVOR1,  BKE_EXC_MACHINE_CHECK*4);
	set_spr(PPCBKE_SPR_IVOR15, BKE_EXC_DEBUG*4);

	// Make data & instruction TLB misses go to the
	// data and instruction storage exceptions.
	set_spr(PPCBKE_SPR_IVOR13, get_spr(PPCBKE_SPR_IVOR2));
	set_spr(PPCBKE_SPR_IVOR14, get_spr(PPCBKE_SPR_IVOR3));

	install_traps(ppc_booke, NUM_ELTS(ppc_booke));
	family_stuff = family_stuff_booke;
	switch(PPC_GET_FAM_MEMBER(pvr)) {
	case PPC_440GP:
	case PPC_440GX:
	case PPC_440EP:
	case PPC_440EPx:
		icache_flusher = flusher_440;
		install_traps(ppc_mc_booke, NUM_ELTS(ppc_mc_booke));
		return 1;
	case PPC_E500:
	case PPC_E500V2:
		install_traps(ppc_mc_e500, NUM_ELTS(ppc_mc_e500));
		return 1;
	}
	return 0;
}
