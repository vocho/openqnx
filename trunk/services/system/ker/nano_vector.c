/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */

#include "externs.h"

#if defined(CHECK_VECTOR) && !defined(NDEBUG)
check_vec(VECTOR *vec) {
	int i, nfree, nused;
	unsigned n;
	void **ptr;

	for(i = nfree = nused = 0 ; i < vec->nentries ; ++i) {
		n = (unsigned)VEC(vec, i);
		if(n & 1) {
			++nfree;
			// Must be in free list
			for(ptr = vec->free ; VECAND(ptr, ~3); ptr = *(void **)VECAND(ptr, ~3)) {
				if(VECAND(ptr, ~3) == VECAND(n, ~3)) {
					break;
				}
			}
			if(ptr == NULL) {
				crash();
			}
		} else if(n == 0) {
			++nfree;
		} else {
			++nused;
			// Must NOT be in free list
			for(ptr = vec->free; VECAND(ptr, ~3); ptr = *(void **)VECAND(ptr, ~3)) {
				if(VECAND(ptr, ~3) == VECAND(n, ~3)) {
					crash();
				}
			}
		}
	}

	if(nused + nfree != vec->nentries) {
		crash();
	}

	if(nfree != vec->nfree) {
		crash();
	}

	ptr = vec->free;
	while(VECAND(ptr, ~3)) {
		n = (unsigned)ptr;
		ptr = (void **)(n & ~3);
		ptr = *ptr;
		if((n & 1) == 0) {
			crash();
		}
	}
}
#define CHECK_VEC(v)	check_vec(v)
#else
#define CHECK_VEC(v)
#endif

void * rdecl
vector_lookup(VECTOR *vec, int id) {
	void *ptr;

	/* Do unsigned compare to catch negative values as well */
	if(vec  &&  (unsigned)id < vec->nentries) {
		return(VECP(ptr, vec, id));
	}
	
	return(NULL);
}


void * rdecl
vector_lookup2(VECTOR *vec, int id) {
	void *ptr;

	/* Do unsigned compare to catch negative values as well */
	if(vec  &&  (unsigned)id < vec->nentries  &&  VECAND(ptr = VEC(vec, id), 1) == 0) {
		return(VECAND(ptr, ~3));
	}
	
	return(NULL);
}


void * rdecl
vector_search(VECTOR *vec, unsigned id, unsigned *found) {
	void	 *ptr;

	if((vec != NULL) && (vec->nfree < vec->nentries)) {
		while(id < vec->nentries) {
			ptr = VECP2(ptr, vec, id);
			if(found) {
				if(ptr) {
					*found = id;
				} else {
					id++;
					continue;
				}
			}
			return(ptr);
		}
	}
	return(NULL);
}


//
// The bottom bit of a pointer in the vector table vec->vector[n] is
// used as follows:
//
// 0  - A regular pointer (this assumes all pointers are 4 byte aligned)
// 1  - On the free list which runs through the vector (vec->free)
//
// The next bit may be used as a flag for various purposes. Thread
// vectors use it to indicate a destroyed thread which has not been joined
// while connections use it to indicate close on exec.
// 
int rdecl
vector_add(VECTOR *vec, void *object, unsigned index) {
	unsigned nentries, inc;
	void *ptr, *optr, **prev, **cur;
	int adjust;

	// all objects must be 32-bit aligned
	if(VECAND(object, 3)) {
		crash();
	}

	// Look for a free spot starting at index.
	for(prev = &vec->free; (cur = VECAND(*prev, ~3)) ; prev = cur) {
		unsigned i = cur - &vec->vector[0];

		if(i >= index) {
			*prev = *cur;
			*cur = object;
			--vec->nfree;
			CHECK_VEC(vec);
			return(i);
		}
	}

	// No free spot so we must grow the vector. Max it out at 65K-1.
	if(vec->nentries == VECTOR_MAX) {
		return(-1);
	}

	if(index >= vec->nentries) {
		nentries = index + 1;					// Grow to what was asked.
	} else {
		nentries = vec->nentries + (vec->nentries + 4) / 4;	// Grow by 25%
	}

	// Always start out with a vector capable of handling at least 8 entries
	// to avoid memory fragmentation
	if(nentries < 8) {
		nentries = 8;
	}

	if(nentries > VECTOR_MAX) {
		nentries = VECTOR_MAX;
	}

	/* Note: since we need to fix up the entries, we cannot simply
	 * realloc a bigger vector as this leave the vectors 
	 * inconsistent (think timers firing from the clock handler)
	 * See PR 24407.
	 */
	ptr = _scalloc(nentries * sizeof(void *));

	if(ptr == NULL) {
		return(-1);
	}
	optr = vec->vector;
	memcpy(ptr, optr, vec->nentries * sizeof(void *));

	// Since we always allocate a new vector, we must fixup the freelist since it runs through it.
	adjust = (char *)ptr - (char *)optr;

#ifndef NDEBUG
if(adjust & 0x03) crash();
#endif

	for(prev = &vec->free; VECAND(*prev, ~3) ; prev = VECAND(*prev, ~3)) {
		*(int *)prev += adjust;
	}
	// This is safe since any IRQ code will only reference valid entries
	vec->vector = ptr;
	// We can now free the old vector
	_sfree(optr, vec->nentries * sizeof(void *));

	// Link all the new slots in the extended vector.
	// Bottom bit indicates it is on the free list.
	*prev = VECOR(cur = &vec->vector[vec->nentries], 1);
	for(inc = nentries - vec->nentries ; --inc ; ++cur) {
		*cur = VECOR(cur + 1, 1);
	}
	*cur = VECOR(NULL, 1);

	// Update the vector info for the new size and possible location.
	vec->nfree += nentries - vec->nentries;
	vec->nentries = nentries;

	// Now that we have grown the vector the add can't fail!
	return(object ? vector_add(vec, object, index) : index);
}


void * rdecl
vector_rem(VECTOR *vec, unsigned index) {
	void *ptr, **vecp;

	// Verify index is range and that the entry is not already free.
	if(index >= vec->nentries  ||  VECAND(ptr = VEC(vec, index), 1)) {
		return(NULL);
	}

	ptr = VECAND(ptr, ~3);

	vecp = &VEC(vec, index - 1);
	if(VECAND(vec->free, ~3) == NULL  ||  vecp < (void **)VECAND(vec->free, ~3)) {
		// Simple case is returning to the start of the list.
		vecp = &vec->free;
	} else {
		// Scan backwards for the insertion point (this cannot fail)!
		while(VECAND(*vecp, 1) == 0) {
			--vecp;
		}
	}

	VEC(vec, index) = *vecp;
	*vecp = VECOR(&VEC(vec, index), 1);
	++vec->nfree;

	CHECK_VEC(vec);
	return(ptr);
}


int rdecl
vector_flag(VECTOR *vec, unsigned index, int flag) {
	void *ptr;

	if(index >= vec->nentries  ||  VECAND(ptr = VEC(vec, index), 1)) {
		return(-1);
	}

	VEC(vec, index) = flag ? VECOR(ptr, 2) : VECAND(ptr, ~2);

	return(0);
}


void rdecl
vector_free(VECTOR *vec) {

	_sfree(vec->vector, vec->nentries * sizeof(void *));
	vec->nentries = vec->nfree = 0;
	vec->vector = NULL;
}

__SRCVERSION("nano_vector.c $Rev: 204178 $");
