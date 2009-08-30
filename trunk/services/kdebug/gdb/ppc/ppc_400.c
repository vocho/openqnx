#include "kdebug.h"
#include <ppc/401cpu.h>
#include <ppc/403cpu.h>
#include <ppc/405cpu.h>

extern void exc_data_access400();
extern void exc_program400();
extern void exc_debug400();
extern void exc_machine_check403();

static const struct trap_entry ppc_403[] = {
	{PPC_EXC_ALIGNMENT,		exc_alignment},
	{PPC_EXC_DATA_ACCESS,	exc_data_access400},
	{PPC_EXC_PROGRAM,		exc_program400},
	{PPC400_EXC_DEBUG,		exc_debug400},
	{PPC_EXC_MACHINE_CHECK, exc_machine_check403},
};

static void
family_stuff_400(int type, CPU_REGISTERS *ctx) {
	switch(type) {
	case FAM_DBG_ENTRY:
		ctx->msr &= ~PPC_MSR_DE;
#ifdef DEBUG_GDB
		kprintf("DBSR=%x\n", get_spr(PPC400_SPR_DBSR));
#endif
		set_spr(PPC400_SPR_DBCR, get_spr(PPC400_SPR_DBCR) & ~PPC400_DBCR_IC);
		set_spr(PPC400_SPR_DBSR, ~0L);
		set_spr(PPC400_SPR_ESR, 0);
		break;
	case FAM_DBG_EXIT_STEP:
		ctx->msr |= PPC_MSR_DE;
		set_spr(PPC400_SPR_DBCR, PPC400_DBCR_IDM | PPC400_DBCR_IC);
		break;
	case FAM_DBG_EXIT_CONTINUE:
		break;
	}
}

int
family_init_400(unsigned pvr) {
	switch(PPC_GET_FAM_MEMBER(pvr)) {
	case PPC_401:
	case PPC_403:
	case PPC_405: //NYI: machine check hander is wrong!
	case PPC_405GPR: //NYI: machine check hander is wrong!
	case PPC_VESTA: //NYI: machine check hander is wrong!
		install_traps(ppc_403, NUM_ELTS(ppc_403));
		family_stuff = family_stuff_400;
		break;
	default:
		return 0;
	}
	msr_bits_off |= PPC_MSR_CE;
	return 1;
}
