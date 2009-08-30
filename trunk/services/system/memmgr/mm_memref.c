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

#include "vmm.h"



//static SOUL	or_souls	INITSOUL(-1, struct mm_object_ref, 32, 8, ~0);
//SMP_SPINVAR(static, ref_spin);
//static struct mm_object_ref	*ref_free_list;


void
memref_walk_restart(struct mm_map_head *mh) {
	atomic_add(&mh->walk_gen, 1);
}


int
memref_add(struct mm_map *mm, OBJECT *obp, ADDRESS *adp, int fd) {
	struct mm_object_ref	**owner;
	struct mm_object_ref	*or;
	struct mm_map			*next;

	VERIFY_OBJ_LOCK(obp);
	//RUSH3: cache last N results for faster operation? 
	//RUSH3: keep all the same 'adp' values grouped together so we can bail
	//RUSH3: from the list faster? Maybe vis-versa and group 'fd' together?
	owner = &obp->mem.mm.refs;
	for( ;; ) {
		or = *owner;
		if(or == NULL) break;
		if((or->adp == adp) && (or->fd == fd)) goto found_it;
		owner = &or->next;
	}

	//RUSH3: object allocator is slightly slower than straight _smalloc
	//RUSH3: keep our own free list? - that's slower as well for some reason.
//	or = proc_object_alloc(&or_souls);
//	INTR_LOCK(&ref_spin);
//	or = ref_free_list;
//	if(or != NULL) {
//		ref_free_list = or->next;
//		INTR_UNLOCK(&ref_spin);
//	} else {
//		INTR_UNLOCK(&ref_spin);
		or = _smalloc(sizeof(*or));
		if(or == NULL) return ENOMEM;
//	}
	or->next = *owner;
	*owner = or;
	or->fd = fd;
	or->adp = adp;
	or->obp = obp;
	or->first_ref = NULL;

found_it:	
	mm->obj_ref = or;
	next = or->first_ref;
	mm->ref.next = next;
	if(next != NULL) {
		next->ref.owner = &mm->ref.next;
	}
	mm->ref.owner = &or->first_ref;
	or->first_ref = mm;
	atomic_add(&adp->map.walk_gen, 1);
	return EOK;
}


void
memref_del(struct mm_map *del_mm) {
	struct mm_map			*mm;
	struct mm_object_ref	*or;
	
	//Don't have to worry about the reference list changing on us - we 
	//should always have the aspace locked in this function. 
	or = del_mm->obj_ref;
	if(or != NULL) {
		mm = del_mm->ref.next;
		*del_mm->ref.owner = mm;
		if(mm != NULL) {
			mm->ref.owner = del_mm->ref.owner;
		}

		if(or->first_ref == NULL) {
			struct mm_object_ref	**owner;
			struct mm_object_ref	*chk;
			OBJECT					*obp;

			obp = or->obp;
			VERIFY_OBJ_LOCK(obp);
			//RUSH3: cache last N results for faster operation? reorder list?
			//RUSH3: double link list?
			owner = &obp->mem.mm.refs;
			for( ;; ) {
				chk = *owner;
				CRASHCHECK(chk == NULL);
				if(chk == or) break;
				owner = &chk->next;
			}
			*owner = chk->next;
			del_mm->obj_ref = NULL;
//			proc_object_free(&or_souls, or);
			_sfree(or, sizeof(*or));
//			INTR_LOCK(&ref_spin);
//			or->next = ref_free_list;
//			ref_free_list = or;
//			INTR_UNLOCK(&ref_spin);
		}
	}
}


//This function has to be able to traverse the reference list
//without locking the address space(s) - the object is locked however.
int
memref_walk(OBJECT *obp, 
		int (*func)(struct mm_object_ref *, struct mm_map *, void *),
		void *d) {
	struct mm_object_ref	*or;
	struct mm_map			*mm;
	int						r;
	struct mm_map_head		*mh;
	unsigned				wg;

	VERIFY_OBJ_LOCK(obp);
	r = EOK;
	for(or = obp->mem.mm.refs; or != NULL; or = or->next) {
		mh = &or->adp->map;
		mm = NULL;
		wg = mh->walk_gen - 1; // force a restart
		for( ;; ) {
			INTR_LOCK(&map_spin);
			if(mm != NULL) mm->inuse = 0;
			if(mh->walk_gen != wg) {
				wg = mh->walk_gen;
				mm = or->first_ref;
			} else {
				CRASHCHECK(mm == NULL);
				mm = mm->ref.next;
			}
			if(mm != NULL) mm->inuse = 1;
			INTR_UNLOCK(&map_spin);
			if(mm == NULL) break;
			CRASHCHECK(mm->obj_ref != or);
			r = func(or, mm, d);
			if(r != EOK) {
				mm->inuse = 0;
				if(r < 0) r = EOK;
				return r;
			}
		}
	}
	return EOK;
}

__SRCVERSION("mm_memref.c $Rev: 199396 $");
