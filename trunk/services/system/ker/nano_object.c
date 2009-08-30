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


//
// The object allocator for the system. All kernel objects are a fixed
// size and allocated using this routine. There is a soul list for each
// type of object. We an object is released we place it back on the soul
// list where it can be quickly reused by the next allocation for this
// object type. This is much faster than calling malloc().
//
//
// If objects have a special alignement constraint, we allocate "align" extra,
// and return an aligned pointer as the object.
//

#define ALIGN_PTR(ptr, align) 		(void **)(((uintptr_t)ptr + align) & ~(align-1))
#define SAVED_PTR(ptr)				(*(void **)((uintptr_t)ptr - 4))

void * rdecl
object_alloc(PROCESS *prp, SOUL *soulp) {
	void	**ptr;
	LIMITS	*lip;

#ifndef NDEBUG
	// We may be called one of three ways:
	//  1. a proc thread.
	//	2. a locked kernel.
	//  3. an unnested interrupt (uniprocessor only).
	{
		int 	ik;

#if defined(VARIANT_smp)
		InterruptDisable();
		ik = get_inkernel();
		if((ik & INKERNEL_NOW) && !(ik & INKERNEL_LOCK) && (RUNCPU == KERNCPU)) {
			crash();
		}
		if(alives[0]) {
			InterruptEnable();
		}
#else
		ik = get_inkernel();
		if((ik != 0) && !(ik & INKERNEL_LOCK) && (ik != 1)){
			crash();
		}
#endif
	}
#endif

	// Check to make sure prp has not exceeded its limits.
	if(prp  &&  (lip = prp->limits)) {
		int type = soulp->type;

		if(lip->cur[type] >= lip->max[type]  &&  (prp->flags & _NTO_PF_NO_LIMITS) == 0) {

			///////////////////////////////////////////////////////
			////////////////////  FIX_ME!!  ///////////////////////
			// For the moment, we comment the old "trace interface"
			// this old interface will be completely replaced
			// by the new one.
			//
			//Trace1(_TRACE_NTO_SOUL_MAX, _TRACE_SW_SEVERITY1, type);
			///////////////////////////////////////////////////////

			return(NULL);
		}

		++lip->cur[type];
	}

	if((ptr = soulp->next) == NULL) {
		if((ptr = object_grow(soulp)) == NULL) {
			return(ptr);
		}
	}

	soulp->next = *ptr;
	*ptr = NULL;

	// Track highwater
	if(soulp->highwater <= ++soulp->used)
		soulp->highwater = soulp->used;

	// Track highwater over 100 calls and make that our new min number
	// of objects to keep.
	if(++soulp->counter >= 100) {
		soulp->newmin = soulp->highwater;
		soulp->highwater = soulp->min;
		soulp->counter = 0;
	}
	if(soulp->align) {
		void	**aligned_ptr;

		aligned_ptr = ALIGN_PTR(ptr, soulp->align);
		SAVED_PTR(aligned_ptr) = ptr;
#ifndef NDEBUG
if((unsigned)aligned_ptr & (soulp->align-1)) crash();
#endif
		return(aligned_ptr);
	} else {
		return(ptr);
	}
}


//
// Return an object back to its soul list.
//
void rdecl
object_free(PROCESS *prp, SOUL *soulp, void *ptr) {
	LIMITS	*lip;

	// used to check to make sure we were in the kernel but now we
	// can't do that since proc may call these routines in its own
	// context

	if(soulp->align) {
		ptr = SAVED_PTR(ptr);
	}

	if(prp  &&  (lip = prp->limits)) {
		if(--lip->cur[soulp->type] == ~0U) {
			crash();
		}
	}

	--soulp->used;
	if(!(soulp->flags & SOUL_NOFREE)) {
		// Do we have too many souls on this list?
		if((soulp->total > soulp->newmin)) {
			// Yes. Free the soul back to the system.
			_sfree(ptr, soulp->size + soulp->align);
			--soulp->total;

			// Still more work to do.
			if(soulp->total > soulp->newmin)
				soul_compact = 1;	// This starts idle cleaning up.
			return;
		}
		// Was it a critical allocation?
		if(crit_sfree(ptr, soulp->size + soulp->align)) {
			--soulp->total;
			return;
		}
	}

	// Nope. Return this soul to the list.
	memset(ptr, 0, soulp->size + soulp->align);
	*(void **)ptr = soulp->next;
	soulp->next = ptr;
}


//
// Compact objects on a soul list if needed.
//
void rdecl
object_compact(SOUL *soulp) {
	void **ptr;

	// used to check to make sure we were in the kernel but now we
	// can't do that since proc may call these routines in its own
	// context

	if(soulp->total > soulp->newmin  &&  (ptr = soulp->next)) {
		soulp->next = *ptr;
		_sfree(ptr, soulp->size + soulp->align);
		--soulp->total;

		if(soulp->total > soulp->newmin) {
			soul_compact = 1;	// More work to do.
		}
	}
}


//
// This grabs one or more new souls.
//
void * rdecl
object_grow(SOUL *soulp) {
	void **ptr;
	unsigned num;

	// used to check to make sure we were in the kernel but now we
	// can't do that since proc may call these routines in its own
	// context

	// If this is the first time grab the minimum requested.
	if(soulp->newmin == 0) {
		soulp->newmin = soulp->min;
#if 0
		if(soulp->type != -1) {
			if(vector_add(&soul_vector, soulp, soulp->type) != soulp->type)
				crash();
		} else {
			vector_add(&soul_vector, soulp, _QUERY_SOULS_NUM);
		}
#endif

		for(num = soulp->min ; --num;) {
			(void)object_grow(soulp);
		}
	}

	if(soulp->flags & SOUL_CRITICAL) alloc_critical++;
	ptr = _scalloc(soulp->size + soulp->align);
	if(soulp->flags & SOUL_CRITICAL) alloc_critical--;

	if(ptr == NULL) {


		///////////////////////////////////////////////////////
		////////////////////  FIX_ME!!  ///////////////////////
		// For the moment, we comment the old "trace interface"
		// this old interface will be completely replaced
		// by the new one.
		//
		// Trace1(_TRACE_NTO_SOUL_MEM, _TRACE_SW_SEVERITY2, soulp->type);
		///////////////////////////////////////////////////////

		return(NULL);
	}

	++soulp->total;
	*ptr = soulp->next;
	soulp->next = ptr;

	return(soulp->next);
}



//
// Adjust the size of the soul entry to allow for more data
//
unsigned rdecl
object_register_data(SOUL *soulp, size_t size) {
	unsigned	r;

	CRASHCHECK(soulp->next != NULL);

	r = ROUNDUP(soulp->size, sizeof(uint64_t));
	soulp->size = r + size;
	return(r);
}


__SRCVERSION("nano_object.c $Rev: 153052 $");
