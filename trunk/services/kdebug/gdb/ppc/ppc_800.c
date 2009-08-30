#include "kdebug.h"
#include <ppc/800cpu.h>

extern void exc_machine_check800();
extern void exc_program800();
extern void exc_trace800();
extern void exc_dbreak800();
extern void exc_ibreak800();
extern void exc_pbreak800();
extern void exc_devport800();

static const struct trap_entry ppc_800[] = {
	{PPC_EXC_ALIGNMENT,		exc_alignment},
	{PPC_EXC_MACHINE_CHECK, exc_machine_check800},
	{PPC_EXC_PROGRAM,		exc_program800},
	{PPC_EXC_TRACE,			exc_trace800},
	{PPC800_EXC_ID_DBRKPT,	exc_dbreak800},
	{PPC800_EXC_ID_IBRKPT,	exc_ibreak800},
	{PPC800_EXC_ID_PBRKPT,	exc_pbreak800},
	{PPC800_EXC_ID_NM_DEVPORT,exc_devport800},
};

static void
family_stuff_800(int type, CPU_REGISTERS *ctx) {
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
family_init_800(unsigned pvr) {
	switch(PPC_GET_FAM_MEMBER(pvr)) {
	case PPC_8xx:
		install_traps(ppc_800, NUM_ELTS(ppc_800));
		family_stuff = family_stuff_800;
		return 1;
	}
	return 0;
}
