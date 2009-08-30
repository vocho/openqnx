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
#include "apm.h"

#define ALIGN(x)			((unsigned)((((unsigned)(x))+(_MALLOC_ALIGN-1))&~(_MALLOC_ALIGN-1)))
#define PTRDIFF(x, y)		(((unsigned)(x))-((unsigned)(y)))
#define PTRADD(x, y)		((void *)((char *)(x)+(y)))

#ifdef NDEBUG
	#define LOCK_KERNEL()	lock_kernel()
#else
	#define LOCK_KERNEL()	if(!(get_inkernel() & INKERNEL_NOW)) crash(); lock_kernel()
#endif

struct free_entry {
	struct free_entry			*next;
	unsigned					size;
};

// There are separate free lists for the kernel and for proc
// (actually two for the kernel and one for proc, see below).
// We choose which free list to use based on whether we are
// in the kernel or not.
//
// Keeping separate free lists avoids proc having to enter the
// kernel for memory allocation and thus reduces the amount of
// time we spend in the kernel (which in turn helps reduce
// thread scheduling latencies).
//
// You may look at this and say, gee, if two heaps are good
// why not have a lot more (one per cpu) to reduce contention
// even further.  Normally this is a good idea but the issue
// is that when you free the memory there is no way to tell
// what free list the memory came from.  While it would still
// work to put the chunk of memory on any free list, that
// causes the free lists to wind up with gaps and holes that
// will never get coalesced (because a chunk of memory from
// list A may get free'd and put on list B).
//
// The other BIG caveat is that you should never allocate memory
// in proc and then free it in the kernel (or vice-versa).  If
// you do that you will cause the same problem described in the
// last paragraph.  This will *severely* affect latencies and
// it will cause the the kernel to use loads more memory.  If
// you need to do this you will need to use the special allocator
// functions that first do a __Ring0() call (see the definition
// of _kfree() and _ksmalloc() at the bottom of this file).
//
// --------------------------------------------------
//
// The kernel now has two separate free lists of it's own. One
// for regular allocations, and one for 'critical' ones.
// We always start out using the regular free list and only check
// the critical list if "alloc_critical" is non-zero. Freeing
// memory that came from that list always goes back on that list (we can
// tell because we get the memory in one blob during initialization
// so any pointer between 'crit_start' and 'crit_end' originally
// came from the critical list. Also, when freeing a SOUL entry,
// we check if it was from the critical region and, if so, release
// it back to the critical free pool right away, rather than hanging
// on to it in nano_object.c (see the crit_sfree routine below).
// The reason for all of this is we had a problem with our old scheme
// where the critical memory would gradually leak away from us and
// be made available for general allocation. Eventually, we would
// run out and have a critical allocation fail.
//
struct free_entry *ker_free_list;
struct free_entry *ker_crit_list;
struct free_entry *proc_free_list;
uintptr_t	crit_start = ~0; // to make initial IS_CRIT() fail
uintptr_t	crit_end = 0;
static void			*pregrow_start;
static size_t		pregrow_size;

#define IS_CRIT(p)	(((uintptr_t)(p) >= crit_start) && ((uintptr_t)(p) <= crit_end))


// This is the lock used when we do allocations on behalf of proc.
static pthread_mutex_t    proc_alloc_mutex = PTHREAD_MUTEX_INITIALIZER;



void *
_sreallocfunc(void *data, unsigned old_size, unsigned new_size, unsigned (*alloc)(unsigned size, void **addr)) {
	struct free_entry *p, *prev, *new, **pp, **fit, *add;
	int                ker;

	if(new_size) {
		new_size = new_size <= sizeof *p ? sizeof *p : ALIGN(new_size);
	}

	if((uintptr_t)data & (_MALLOC_ALIGN-1)) crash();

	add = 0;
	ker = 0;
addmem:
	if(!am_inkernel()) {
		pthread_mutex_lock(&proc_alloc_mutex);
		ker = 0;
		fit = &proc_free_list;
	} else if((ker <= 1) && !IS_CRIT(data)) {
		ker = 1;
		fit = &ker_free_list;
	} else {
		ker = 2;
		fit = &ker_crit_list;
	}
	pp = fit;

	p = prev = new = 0;
	if(data && old_size) {
		// realloc and free
		old_size = old_size < sizeof *p ? sizeof *p : ALIGN(old_size);

		if(new_size == old_size) {
			// If no size change, we are done
			if (!ker) {
				pthread_mutex_unlock(&proc_alloc_mutex);
			}
			return data;
		}
		if(new_size && new_size + sizeof *p <= old_size) {
			// If shrinking, change to a free, but keep "new" to return
			new = data;
			data = PTRADD(data, new_size);
			old_size -= new_size;
			new_size = 0;
		}

		for(pp = fit, fit = 0, prev = 0; (p = *pp); pp = &(prev = p)->next) {
			if(new_size && (p->size == new_size || (!fit && p->size >= new_size + sizeof *p))) {
				fit = pp;
			}

			if(data && ((unsigned)data >= (unsigned)p && (unsigned)data < ((unsigned)p + p->size))) {
				// freed an already freed area
				crash();

				/* NOTREACHED */
				if (!ker) {
					pthread_mutex_unlock(&proc_alloc_mutex);
				}

				return 0;
			}

			if(data && (unsigned)p > (unsigned)data) {
				break;
			}
		}
	}

	if(new_size) {
		if(data && PTRADD(data, old_size) == p) {
			if(new_size == old_size + p->size) {
				if (ker) {
					LOCK_KERNEL();
				}

				*pp = p->next;

				if (!ker) {
					pthread_mutex_unlock(&proc_alloc_mutex);
				}

				return data;
			} else if(new_size + sizeof *p <= old_size + p->size) {
				// We must save "*p" as prev may overlap
				struct free_entry			save = *p;

				prev = PTRADD(data, new_size);
				if (ker) {
					LOCK_KERNEL();
				}

				prev->size = (old_size + save.size) - new_size;
				prev->next = save.next;
				*pp = prev;

				if (!ker) {
					pthread_mutex_unlock(&proc_alloc_mutex);
				}

				return data;
			}
		}

		for(fit = fit ? fit : pp; (new = *fit); fit = &new->next) {
			if(new->size == new_size) {
				if (ker) {
					LOCK_KERNEL();
				}

				*fit = new->next;
				if(prev == new) {
					pp = fit;
					prev = 0;
				}
				break;
			}
			if(new->size >= new_size + sizeof *new) {
				p = PTRADD(new, new_size);
				if (ker) {
					LOCK_KERNEL();
				}

				p->size = new->size - new_size;
				p->next = new->next;
				*fit = p;
				if(prev == new) {
					pp = &p->next;
					prev = p;
				}
				break;
			}
		}

		if(!new) {
			unsigned					size;

			// when allocating make sure there is enough room of a new free entry
			if(alloc) {
				if (ker) {
					LOCK_KERNEL();
				} else {
					pthread_mutex_unlock(&proc_alloc_mutex);
				}

				if(ker == 2) {
					//If we're working the critical list, we know
					//there's no memory available from the memmgr.
					size = 0;
				} else {
					size = alloc(new_size + sizeof *new, (void **)&new);
				}
				if (size) {
					if(!ker) {
						// If we're not the kernel we can't just
						// continue on, because the realloc handling
						// code down below is going to attempt to
						// use the 'prev' pointer which may no longer
						// be valid since we released the mutex before
						// calling 'alloc' (another proc thread might
						// have allocated/freed memory). Instead we
						// "free" the memory to put it into the heap
						// and then just start over from the top.

						_sreallocfunc(new, size, 0, NULL);
						goto addmem;
					}
					if((size -= new_size)) {
						add = PTRADD(new, new_size);
						add->size = size;
					}
				} else {
					// NYI: Before going to the critical list
					// or failing the allocation, we should tell
					// all the SOUL lists to free any unneeded entries.
					if((ker == 1) && alloc_critical) {
						// alloc failed and it's critical. Try
						// again from the critical list
						ker = 2;
						goto addmem;
					}
					// alloc failed, make sure we don't do a free
					new = 0;
					data = 0;
				}
			}

		}
		if(data && new) {
			// realloc to new area, so copy old data over
			if (ker) {
				LOCK_KERNEL();
			}

			memcpy(new, data, min(new_size, old_size));
		}
	}
	if(data) {
		if (ker) {
			LOCK_KERNEL();
		}
#ifndef NDEBUG
		// Erase data currently there - we pick something that will
		// usually generate an illegal address
		memset(data, 0x6d, old_size);
#endif

		// Free old entry, and coalesce if possible
		if(prev && PTRADD(prev, prev->size) == data) {
			// Previous free entry connects to this entry so join them
			prev->size += old_size;
		} else {
			// Make a new free entry for this data
			prev = data;
			prev->next = *pp;
			prev->size = old_size;
			*pp = prev;
		}
		if(PTRADD(prev, prev->size) == (p = prev->next)) {
			// Join this area with the free entry following this one
			prev->next = p->next;
			prev->size += p->size;
		}
	}

	// When allocating memory, extra was allocated, so add it to the free list
	if(add) {
		if(alloc) {
			data = add;
			old_size = add->size;
			new_size = 0;
			add = new;
			alloc = 0;
			goto addmem;
		}
		new = add;
	}

	if((uintptr_t)new & (_MALLOC_ALIGN-1)) crash();

	if (!ker) {
		pthread_mutex_unlock(&proc_alloc_mutex);
	}

	return new;
}

static unsigned
_heap_alloc(unsigned size, void **addr) {
	size = ROUNDUP(size, __PAGESIZE);
	if(size <= pregrow_size) {
		// Can use the memory that was pre-allocated by heap_init()
		*addr = pregrow_start;
		pregrow_size -= size;
		pregrow_start = (uint8_t *)pregrow_start + size;
		return size;
	}

	if(memmgr.mmap(0, 0, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON|MAP_PRIVATE,
					0, 0, _MALLOC_ALIGN, 0, NOFD, addr, &size, mempart_getid(NULL, sys_memclass_id)) != EOK) {
		return 0;
	}
	return size;
}

void *
_srealloc(void *data, unsigned old_size, unsigned new_size) {
	return _sreallocfunc(data, old_size, new_size, _heap_alloc);
}

void
_sfree(void *data, unsigned size) {
	if((uintptr_t)data & (_MALLOC_ALIGN-1)) {
		size -= _MALLOC_ALIGN - ((uintptr_t)data & (_MALLOC_ALIGN-1));
		size &= ~(_MALLOC_ALIGN-1);
		data = (char *)data + _MALLOC_ALIGN - ((uintptr_t)data & (_MALLOC_ALIGN-1));
	};
	_sreallocfunc(data, size, 0, _heap_alloc);
}

void *
_smalloc(unsigned size) {
	return _sreallocfunc(0, 0, size, _heap_alloc);
}

void *
_scalloc(unsigned size) {
	void						*p;

	if((p = _sreallocfunc(0, 0, size, _heap_alloc))) {
		memset(p, 0x00, size);
	}
	return p;
}

void
free(void *data) {
	unsigned					*p = data;

	if(p) {
		p = (unsigned *)((char *)p - _MALLOC_ALIGN);
		_sreallocfunc(p, *p, 0, _heap_alloc);
	}
}

void *
malloc(unsigned size) {
	unsigned					*p;
	size += _MALLOC_ALIGN;
	if((p = _sreallocfunc(0, 0, size, _heap_alloc))) {
		*p = size;
		p = (unsigned *)((char *)p + _MALLOC_ALIGN);
	}
	return p;
}

void *
calloc(size_t size, size_t num) {
	unsigned					*p;

	size = size * num + _MALLOC_ALIGN;
	if((p = _sreallocfunc(0, 0, size, _heap_alloc))) {
		memset(p, 0x00, size);
		*p = size;
		p = (unsigned *)((char *)p + _MALLOC_ALIGN);
	}
	return p;
}

void *
realloc(void *data, unsigned size) {
	unsigned					*p;
	unsigned					old_size;

	old_size = 0;
	if((p = data)) {
		p = (unsigned *)((char *)p - _MALLOC_ALIGN);
		old_size = *p;
	}
	if(size) {
		size += _MALLOC_ALIGN;
	}
	if((p = _sreallocfunc(p, old_size, size, _heap_alloc))) {
		if(size) {
			*p = size;
			p = (unsigned *)((char *)p + _MALLOC_ALIGN);
		} else {
			p = 0;
		}
	}
	return p;
}

#define CRIT_SIZE (4*__PAGESIZE)

void
heap_init(size_t pregrow) {
	if(pregrow != 0) {
		//Should I loop with smaller sizes if the alloc fails?
		pregrow_size = _heap_alloc(pregrow, &pregrow_start);
	}

	// Preallocate some pages of memory for critical allocations.
	// We only dip into these when the alloc_critical counter is non-zero

	ker_crit_list = _smalloc(CRIT_SIZE);
	ker_crit_list->next = NULL;
	ker_crit_list->size = CRIT_SIZE;
	crit_start = (uintptr_t)ker_crit_list;
	crit_end = crit_start + (CRIT_SIZE-1);
}


int
crit_sfree(void *p, unsigned size) {
	// If the pointer is from the critical area, we want to put it back
	// in the critical free list as soon as possible so that it's available
	// for future allocations.
	if(IS_CRIT(p)) {
		_sfree(p, size);
		return 1;
	}
	return 0;
}


//
// The following variants of the allocator calls first do a __Ring0() so
// that the memory is allocated or free'd from the kernel heap and not the
// proc heap.  This is necessary because sometimes the kernel will allocate
// something that proc will free or vice-versa.  It's rare, but it does
// happen (the prp->vfork_info and prp->session structures are two examples).

static void
ker_free(void *data) {
	free(data);
}

void _kfree(void *data) {
	__Ring0(ker_free, data);
}


struct kerargs_sfree {
	void *p;
	unsigned size;
};

static void
ker_sfree(void *data) {
	struct kerargs_sfree *kap = data;

	_sfree(kap->p, kap->size);
}

void _ksfree(void *p, unsigned size) {
	struct kerargs_sfree data;

	data.p = p;
	data.size = size;
	__Ring0(ker_sfree, &data);
}

struct kerargs_smalloc {
	unsigned size;
};


static void
ker_smalloc(void *data) {
	struct kerargs_smalloc *kap = data;
	void *ptr;

	ptr = _sreallocfunc(0, 0, kap->size, _heap_alloc);
	lock_kernel();
	SETKSTATUS(actives[KERNCPU],ptr);
}

void *
_ksmalloc(unsigned size) {
	struct kerargs_smalloc data;

	data.size = size;
	return (void *)(__Ring0(ker_smalloc, &data));
}



__SRCVERSION("nano_alloc.c $Rev: 199078 $");
