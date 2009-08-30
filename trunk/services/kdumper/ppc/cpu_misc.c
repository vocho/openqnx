#include <ppc/cpu.h>
#include <ppc/inline.h>
#include "kdumper.h"


void
cpu_elf_header(void *ehdr) {
	if(dip->big) {
		((Elf64_Ehdr *)ehdr)->e_machine = EM_PPC;
	} else {
		((Elf32_Ehdr *)ehdr)->e_machine = EM_PPC;
	}
}

void
cpu_note(struct kdump_note *note) {
	note->cpu_info = SYSPAGE_CPU_ENTRY(ppc, kerinfo)->ppc_family;
}


void
cpu_walk_extra_pmem(void (*func)(paddr_t, paddr_t, int, void *), void *data) {
	unsigned	sdr1;
	unsigned	size;

	// Dump the exception table area
	func(0, 0x3000, -1, data);

	if(SYSPAGE_CPU_ENTRY(ppc, kerinfo)->ppc_family == PPC_FAMILY_900) {
		// Need to dump the hash table
		sdr1 = get_spr(PPC_SPR_SDR1);
		size = 0x40000 << (sdr1 & 0x1f);
		func(sdr1 & ~((1 << 18) - 1), size, -1, data);
	}
}
