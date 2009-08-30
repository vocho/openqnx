#include "kdebug.h"

//ZZZ The ARM versions of cpu_map & cpu_unmap in libkdutil.a should
//be implemented and this file removed so that ../mapping.c gets used.

/*
 * Set up a mapping for reading/writing
 */
void *
mapping_add(uintptr_t va, size_t request_len, unsigned prot, size_t *valid_len) {
	unsigned	off = va & PGMASK;
	pte_t		pte;

	if(*VTOPDIR(va) == 0 || ((pte = *VTOPTEP(va)) & ARM_PTE_VALID) == 0) {
		return NULL;
	}

	if(prot & PROT_WRITE) {
		*VTOPTEP(va) |= ARM_PTE_PROT(ARM_PTE_RW);
		arm_v4_dtlb_addr(va);
	}
	*valid_len = __PAGESIZE - off;
	return (void *)va;
}


/*
 * Tear down a mapping from above
 */
void
mapping_del(void *p, size_t len) {
}
