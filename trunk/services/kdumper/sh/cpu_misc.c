#include "kdumper.h"


void
cpu_elf_header(void *ehdr) {
	if(dip->big) {
		((Elf64_Ehdr *)ehdr)->e_machine = EM_SH;
	} else {
		((Elf32_Ehdr *)ehdr)->e_machine = EM_SH;
	}
}

void
cpu_note(struct kdump_note *note) {
	// Nothing to do (I think...)
}


void
cpu_walk_extra_pmem(void (*func)(paddr_t, paddr_t, int, void *), void *data) {
	// Nothing to do (I think...)
}
