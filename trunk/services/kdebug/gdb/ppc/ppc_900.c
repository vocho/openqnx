#include "kdebug.h"
#include <ppc/603cpu.h>
#include <ppc/970cpu.h>


extern void exc_alignment900();
extern void exc_program900();
extern void exc_trace900();

static const struct trap_entry ppc_970[] = {
	{PPC_EXC_ALIGNMENT,		exc_alignment900},
	{PPC_EXC_TRACE,			exc_trace900},
	{PPC_EXC_PROGRAM,		exc_program900},
};


static void
flusher_970(uintptr_t mapped_vaddr, uintptr_t real_vaddr, unsigned len) {
	uintptr_t	end;
	unsigned	line_size;

	// The 970 has a virtual instruction cache, so we have to do the icache
	// flushing with the real vaddr of the memory, not the translated one
	line_size = SYSPAGE_ENTRY(cacheattr)[SYSPAGE_ENTRY(cpuinfo)->ins_cache].line_size;
	end = real_vaddr + len;
	while(real_vaddr < end) {
		asm volatile( "icbi 0,%0" :: "r" (real_vaddr));
		real_vaddr += line_size;
	}
	cache_flush(mapped_vaddr, len);
}


static void
family_stuff_900(int type, CPU_REGISTERS *ctx) {
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
family_init_900(unsigned pvr) {
	family_stuff = family_stuff_900;

	// Don't turn off the DR & IR bits when we come into the kernel debugger.
	msr_bits_off &= ~(get_msr() & (PPC_MSR_DR|PPC_MSR_IR));

	switch(PPC_GET_FAM_MEMBER(pvr)) {
	case PPC_PA6T:
		install_traps(ppc_970, NUM_ELTS(ppc_970));
		break;
	case PPC_970FX:
		icache_flusher = flusher_970;
		install_traps(ppc_970, NUM_ELTS(ppc_970));
		break;
	default:
		return 0;
	}
	return 1;
}
