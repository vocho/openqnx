#include "kdebug.h"

// This routines might be overridden by CPU specific ones.

/*
 * Set up a mapping for reading/writing
 */
void *
mapping_add(uintptr_t vaddr, size_t request_len, unsigned prot, size_t *valid_len) {
	paddr_t		paddr;
	size_t		size;

	if(vaddrinfo(NULL, vaddr, &paddr, &size) == PROT_NONE) return(NULL);
	if(SH_IS_P2(vaddr) || SH_IS_P4(vaddr)) {
		// If in a non-cacheable region, just return the original vaddr
		return (void *)vaddr;
	}
	return cpu_map(paddr, request_len, prot, -1, valid_len);
}


/*
 * Tear down a mapping from above
 */
void
mapping_del(void *p, size_t len) {
	cpu_unmap(p, len);
}
