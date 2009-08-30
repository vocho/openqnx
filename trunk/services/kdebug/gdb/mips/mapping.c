#include "kdebug.h"

// Override the generic versions of the routines.


/*
 * Set up a mapping for reading/writing
 */
void *
mapping_add(uintptr_t vaddr, size_t request_len, unsigned prot, size_t *valid_len) {
	paddr_t		paddr;
	size_t		size;

	//If the original addr was in kseg1, don't switch it to kseg0 - the
	//cache screws up device access.
	if(MIPS_IS_KSEG1(vaddr)) {
		*valid_len = MIPS_R4K_K1SIZE - vaddr;
		return (void *)vaddr;
	}
	if(vaddrinfo(NULL, vaddr, &paddr, &size) == PROT_NONE) return(NULL);

	//ZZZ Get the memory colour correct...
	return cpu_map(paddr, request_len, prot, -1, valid_len);
}


/*
 * Tear down a mapping from above
 */
void
mapping_del(void *p, size_t len) {
	cpu_unmap(p, len);
}
