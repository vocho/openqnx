#include <mips/cpu.h>
#include <sys/mman.h>
#include "kdumper.h"


void
cpu_elf_header(void *ehdr) {
	if(dip->big) {
		((Elf64_Ehdr *)ehdr)->e_machine = EM_MIPS;
	} else {
		((Elf32_Ehdr *)ehdr)->e_machine = EM_MIPS;
	}
}

void
cpu_note(struct kdump_note *note) {
	note->cpu_info = SYSPAGE_ENTRY(cpuinfo)[0].flags;
}


void
cpu_walk_extra_pmem(void (*func)(paddr_t, paddr_t, int, void *), void *data) {
	// Nothing to do (I think...)
}
