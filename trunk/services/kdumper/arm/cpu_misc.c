#include "kdumper.h"


void
cpu_elf_header(void *ehdr) {
	if(dip->big) {
		((Elf64_Ehdr *)ehdr)->e_machine = EM_ARM;
	} else {
		((Elf32_Ehdr *)ehdr)->e_machine = EM_ARM;
	}
}

void
cpu_note(struct kdump_note *note) {
	// Nothing to do (I think...)
}


void
cpu_walk_extra_pmem(void (*func)(paddr_t, paddr_t, int, void *), void *data) {
	//ZZZ Dump the early L2 tables, same as X86
}
