#include "kdebug.h"
#include <ppc/603cpu.h>


extern void exc_program603();
extern void exc_trace603();
extern void exc_ibreak603();

static const struct trap_entry ppc_603[] = {
	{PPC_EXC_ALIGNMENT,		exc_alignment},
	{PPC_EXC_TRACE,			exc_trace603},
	{PPC_EXC_PROGRAM,		exc_program603},
	{PPC603_EXC_IABREAKPOINT,exc_ibreak603},
};

static void
family_stuff_600(int type, CPU_REGISTERS *ctx) {
	switch(type) {
	case FAM_DBG_ENTRY:
		ctx->msr &= ~PPC_MSR_SE;
		break;
	case FAM_DBG_EXIT_STEP:
		ctx->msr |= PPC_MSR_SE;
		break;
	case FAM_DBG_EXIT_CONTINUE:
		break;
	}
}

int
family_init_600(unsigned pvr) {

	// If we're using a startup that turns on the MMU early, don't
	// turn off the DR & IR bits when we come into the kernel debugger.
	msr_bits_off &= ~(get_msr() & (PPC_MSR_DR|PPC_MSR_IR));

	switch(PPC_GET_FAM_MEMBER(pvr)) {
	case PPC_604:
	case PPC_604e:
	case PPC_604e5:
	case PPC_750:
	case PPC_7400:
	case PPC_7448:
	case PPC_7450:
	case PPC_603e7:
	case PPC_603e:
	case PPC_8260:
	case PPC_7455:
	case PPC_7457:
		install_traps(ppc_603, NUM_ELTS(ppc_603));
		break;
	default:
		return 0;
	}
	family_stuff = family_stuff_600;
	return 1;
}
