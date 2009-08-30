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




#ifdef NDEBUG
#undef CHECK_HEAP
#undef CHECK_ARGS
#else
#ifdef __NEUTRINO__
#undef CHECK_HEAP			// include heap checking code
#define CHECK_ARGS			// verify dword alignment
#else
#undef CHECK_HEAP			// include heap checking code
#undef CHECK_ARGS			// verify dword alignment
#endif
#endif

#define MP_SAFE			1	// Define this to make alloc functions re-enterent
							// between multiple processors (not currently coded)

#define IRQ_SAFE		2	// Define this to make alloc functions re-enterent
							// through interupt handlers (Not currently nessessary)

#define KERNEL_SAFE		3	// Define this to make alloc functions re-enterent
							// between the kernel and threads bound into the kernel.

#define THREAD_SAFE		4	// Define this to make alloc functions re-enterent
							// between threads (for normal C libraries)

#ifdef __NEUTRINO__
#define ALLOC_SAFE		KERNEL_SAFE
#else
#define ALLOC_SAFE		THREAD_SAFE
#endif

#ifdef __NEUTRINO__
#include "externs.h"
#else
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>
#endif
#include <sys/mman.h>

#define ALIGN(x)			((unsigned)((((unsigned)(x))+7)&~7))
#define PTRDIFF(x, y)		(((unsigned)(x))-((unsigned)(y)))
#define PTRADD(x, y)		((void *)((char *)(x)+(y)))

#ifdef TEST
#undef ALLOC_SAFE
#define ALLOC_SAFE		0
#undef __NEUTRINO__
#define ALLOC_SAFE		0
#define malloc Malloc
#define calloc Calloc
#define free Free
#define realloc Realloc
#define sbrk Sbrk
#endif

/*
 	Global variables are defined in a seperate source file (_salloc_data.c)
	so that we can have a single instance of them (in libc.so) for a
	process - this code is not in libc.so, only libc[S].a and the variables
	would get duplicated if multiple shared objects (e.g. file systems)
	use the _salloc functions.
*/
extern struct free_entry {
	struct free_entry			*next;
	unsigned					size;
}							*_Sfree_list;

#if !defined(__NEUTRINO__) && !defined(TEST)
extern pthread_mutex_t		_Salloc_mutex;
extern pthread_once_t		_Salloc_once;
extern void					*_curbrk;

static void alloc_prepare(void) {
	pthread_mutex_lock(&_Salloc_mutex);
}

static void alloc_parent(void) {
	pthread_mutex_unlock(&_Salloc_mutex);
}

static void alloc_child(void) {
	static const pthread_mutex_t			mutex = PTHREAD_MUTEX_INITIALIZER;

	_Salloc_mutex = mutex;
}

static void alloc_init(void) {
	(void)pthread_atfork(alloc_prepare, alloc_parent, alloc_child);
}
#endif

void *_sreallocfunc(void *data, unsigned old_size, unsigned new_size, unsigned (*alloc)(unsigned size, void **addr)) {
	struct free_entry			*p, *prev, *new, **pp, **fit, *add;
#ifdef CHECK_HEAP
	void check(struct free_entry *p);
#endif
#if ALLOC_SAFE == IRQ_SAFE
	static volatile int			restart;
#endif

#if !defined(__NEUTRINO__) && !defined(TEST)
	pthread_once(&_Salloc_once, alloc_init);
#endif

#if ALLOC_SAFE == KERNEL_SAFE
	INDIVISIBLE();
#elif ALLOC_SAFE == THREAD_SAFE
	if(pthread_mutex_lock(&_Salloc_mutex) != EOK) {
		return 0;
	}
#endif

#if defined(CHECK_ARGS) && defined(__NEUTRINO__)
if(3 & (unsigned) data) crash(__FILE__, __LINE__);
#endif
	if(new_size) {
		new_size = new_size <= sizeof *p ? sizeof *p : ALIGN(new_size);
	}

#ifdef CHECK_HEAP
	check(_Sfree_list);
#endif

#if ALLOC_SAFE == IRQ_SAFE
again:
	restart = 0;
#endif
	add = 0;
addmem:
	pp = fit = &_Sfree_list;
	p = prev = new = 0;
	if(data && old_size) {
		// realloc and free
		old_size = old_size < sizeof *p ? sizeof *p : ALIGN(old_size);

		if(new_size == old_size) {
			// If no size change, we are done
#if ALLOC_SAFE == KERNEL_SAFE
			DIVISIBLE();
#elif ALLOC_SAFE == THREAD_SAFE
			pthread_mutex_unlock(&_Salloc_mutex);
#endif
#if defined(CHECK_ARGS) && defined(__NEUTRINO__)
if(3 & (unsigned) data) crash(__FILE__, __LINE__);
#endif
			return data;
		}
		if(new_size && new_size + sizeof *p <= old_size) {
			// If shrinking, change to a free, but keep "new" to return
			new = data;
			data = PTRADD(data, new_size);
			old_size -= new_size;
			new_size = 0;
		}

#if ALLOC_SAFE == IRQ_SAFE
		_disable();
		if(restart) {
			_enable();
			goto again;
		}
#endif
		for(pp = fit, fit = 0, prev = 0; (p = *pp); pp = &(prev = p)->next) {
			if(new_size && (p->size == new_size ||
					(!fit && p->size >= new_size + sizeof *p))) {
				fit = pp;
			}
#if ALLOC_SAFE == IRQ_SAFE
			_enable();
#endif

			if(data && ((unsigned)data >= (unsigned)p && (unsigned)data < ((unsigned)p + p->size))) {
#if ALLOC_SAFE == IRQ_SAFE
				restart = 1;
#elif ALLOC_SAFE == KERNEL_SAFE
				DIVISIBLE();
#elif ALLOC_SAFE == THREAD_SAFE
				pthread_mutex_unlock(&_Salloc_mutex);
#endif
				return 0;
			}

			if(data && (unsigned)p > (unsigned)data) {
				break;
			}
#if ALLOC_SAFE == IRQ_SAFE
			_disable();
			if(restart) {
				_enable();
				goto again;
			}
#endif
		}
#if ALLOC_SAFE == IRQ_SAFE
		_enable();
#endif
#if 0
	} else {
		// malloc, zero data and old_size just in case
		old_size = 0;
		data = 0;
#endif
	}

	if(new_size) {
		if(data && PTRADD(data, old_size) == p) {
#if ALLOC_SAFE == IRQ_SAFE
			_disable();
			if(restart) {
				_enable();
				goto again;
			}
#endif
			if(new_size == old_size + p->size) {
				*pp = p->next;
#if ALLOC_SAFE == IRQ_SAFE
				restart = 1;
				_enable();
#elif ALLOC_SAFE == KERNEL_SAFE
				DIVISIBLE();
#elif ALLOC_SAFE == THREAD_SAFE
				pthread_mutex_unlock(&_Salloc_mutex);
#endif
#if defined(CHECK_ARGS) && defined(__NEUTRINO__)
if(3 & (unsigned) data) crash(__FILE__, __LINE__);
#endif
				return data;
			} else if(new_size + sizeof *p <= old_size + p->size) {
				// We must save "*p" as prev may overlap 
				struct free_entry			save = *p;

				prev = PTRADD(data, new_size);
				prev->size = (old_size + save.size) - new_size;
				prev->next = save.next;
				*pp = prev;
#if ALLOC_SAFE == IRQ_SAFE
				restart = 1;
				_enable();
#elif ALLOC_SAFE == KERNEL_SAFE
				DIVISIBLE();
#elif ALLOC_SAFE == THREAD_SAFE
				pthread_mutex_unlock(&_Salloc_mutex);
#endif
#if defined(CHECK_ARGS) && defined(__NEUTRINO__)
if(3 & (unsigned) data) crash(__FILE__, __LINE__);
#endif
				return data;
			}
#if ALLOC_SAFE == IRQ_SAFE
			_enable();
#endif
		}
#if ALLOC_SAFE == IRQ_SAFE
		_disable();
		if(restart) {
			_enable();
			goto again;
		}
#endif
		for(fit = fit ? fit : pp; (new = *fit); fit = &new->next) {
			if(new->size == new_size) {
				*fit = new->next;
				if(prev == new) {
					pp = fit;
					prev = 0;
				}
				break;
			}
			if(new->size >= new_size + sizeof *new) {
				p = PTRADD(new, new_size);
				p->size = new->size - new_size;
				p->next = new->next;
				*fit = p;
				if(prev == new) {
					pp = &p->next;
					prev = p;
				}
				break;
			}
#if ALLOC_SAFE == IRQ_SAFE
			_enable();
			_disable();
			if(restart) {
				_enable();
				goto again;
			}
#endif
		}
#if ALLOC_SAFE == IRQ_SAFE
		_enable();
#endif
		if(!new) {
			unsigned					size;

			// when allocating make sure there is enough room of a new free entry
			if(alloc && (size = alloc(new_size + sizeof *new, (void **)&new))) {
#if ALLOC_SAFE == IRQ_SAFE
				_disable();
				if(restart) {
					_enable();
					// must free "new" allocated area
					goto again;
				}
#endif
				if((size -= new_size)) {
#ifdef CHECK_HEAP
if(size < sizeof *new || (size & 3)) crash(__FILE__, __LINE__);
#endif
					add = PTRADD(new, new_size);
					add->size = size;
					// the alloc function in neutrino could re-enter the function to free objects so always do the slow add
#ifndef __NEUTRINO__
					// If new allocation is above last free, add it to the list (optimization)
					if((unsigned)add > (unsigned)fit) {
						add->next = *fit;
						*fit = add;
						add = 0;
					}
#endif
				}
#if ALLOC_SAFE == IRQ_SAFE
				_enable();
#endif
			} else {
				// alloc failed, make sure we don't do a free
				new = 0;
				data = 0;
			}
		}
		if(data && new) {
			// realloc to new area, so copy old data over
			memcpy(new, data, min(new_size, old_size));
		}
	} else if(!data && old_size == 0) {
		// zero length malloc
		new = *fit;
	}
	if(data) {
#if ALLOC_SAFE == IRQ_SAFE
		_disable();
		if(restart) {
			_enable();
			new_size = 0;
			goto again;
		}
#endif
		// Free old entry, and coless if possible
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
#if ALLOC_SAFE == IRQ_SAFE
		_enable();
#endif
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
#if ALLOC_SAFE == IRQ_SAFE
	restart = 1;
#elif ALLOC_SAFE == KERNEL_SAFE
	DIVISIBLE();
#elif ALLOC_SAFE == THREAD_SAFE
	pthread_mutex_unlock(&_Salloc_mutex);
#endif
#if defined(CHECK_ARGS) && defined(__NEUTRINO__)
if(3 & (unsigned) new) crash(__FILE__, __LINE__);
#endif
	return new;
}

void _salloc_init(void *data, unsigned size) {
	if(data && size) {
		(void)_sreallocfunc(data, size, 0, 0);
	}
}
	
unsigned _heap_alloc(unsigned size, void **addr);

void *_srealloc(void *data, unsigned old_size, unsigned new_size) {
	return _sreallocfunc(data, old_size, new_size, _heap_alloc);
}

void _sfree(void *data, unsigned size) {
	(void)_sreallocfunc(data, size, 0, _heap_alloc);
}

void *_smalloc(unsigned size) {
	return _sreallocfunc(0, 0, size, _heap_alloc);
}

void *_scalloc(unsigned size) {
	void						*p;

	if((p = _sreallocfunc(0, 0, size, _heap_alloc))) {
		memset(p, 0x00, size);
	}
	return p;
}

#ifndef TEST

#ifdef __NEUTRINO__
static unsigned _heap_alloc(unsigned size, void **addr) {
	if(memmgr.mmap(0, 0, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON|MAP_PRIVATE, 0, 0, addr, &size) != EOK) {
		return 0;
	}
	return size;
}
#else
static int _brk(void *brkpt) {
	if(brkpt > _curbrk) {
		unsigned			size = (char *)brkpt - (char *)_curbrk;

		if((brkpt = mmap(_curbrk, size, PROT_READ | PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
			return -1;
		}
		brkpt = (char *)brkpt + size;
	} else if(brkpt < _curbrk) {
		if(munmap(brkpt, (char *)_curbrk - (char *)brkpt) == -1) {
			return -1;
		}
	}
	_curbrk = brkpt;
	return (int)brkpt;
}

int brk(void *brkpt) {
	pthread_mutex_lock(&_Salloc_mutex);
	brkpt = (void *)_brk(brkpt);
	pthread_mutex_unlock(&_Salloc_mutex);
	return ((int)brkpt == -1) ? -1 : 0;
}

void *sbrk(int increment) {
	void				*oldbrk;

	pthread_mutex_lock(&_Salloc_mutex);
	if(_brk((char *)(oldbrk = _curbrk) + increment) == -1) {
		oldbrk = (void *)-1;
	}
	pthread_mutex_unlock(&_Salloc_mutex);
	return oldbrk;
}

unsigned _heap_alloc(unsigned len, void **addr) {
	size_t							size;

	len = max(len, _amblksiz);
	if(_mmap(0, len, PROT_READ | PROT_WRITE, MAP_ANON|MAP_PRIVATE, NOFD, 0, 0, addr, &size) == MAP_FAILED) {
		return 0;
	}
	_curbrk = (char *)*addr + size;
	return size;
}

size_t qnx_heap_alloc(size_t size, void **addr) {
	pthread_mutex_lock(&_Salloc_mutex);
	size = _heap_alloc(size, addr);
	pthread_mutex_unlock(&_Salloc_mutex);
	return size;
}
#endif

#else

#include <stdio.h>

static struct alloc {
	struct alloc	*next;
	char		*p;
	unsigned	size;
} *a, *al;

int sbrk(int increment) {
	static void				*ptr;
	void					*p;
	extern void				*_nmalloc(unsigned);

	if(increment > 0) {
		unsigned			align;

		increment = (increment + 4095 + 3) & ~4095;
		do {
			p = (void *)_nmalloc(increment);
		} while((unsigned)p < (unsigned)ptr);
		ptr = p;
		if(align = (unsigned)ptr & 3) {
			ptr = PTRADD(ptr, align);
			increment -= align;
		}
		if(ptr) {
			a = _nmalloc(sizeof *a);
			a->next = al;
			a->p = ptr;
			a->size = increment;
			al = a;
		}
	}
	p = ptr;
	if(increment > 0) {
		ptr = (char *)ptr + increment;
	}
	return (int)p;
}

struct r {
	char *p;
	int size;
} r[60];

void data_set(struct r *r2) {
	unsigned char			c;

	c = (unsigned char)r2->p;
	c |= r2->size > 0 ? 1 : 2;
	memset(r2->p, c, r2->size > 0 ? r2->size : -r2->size);
}

void data_check(struct r *r1, void *r2) {
	unsigned char			c;
	char					*p;
	unsigned				len;

	c = (unsigned char)r1->p;
	c |= r1->size > 0 ? 1 : 2;
	for(p = r2 ? r2 : r1->p, len = r1->size > 0 ? r1->size : -r1->size; len; len--, p++) {
		if(*p != c) {
			printf("Data got corrupt\n");
			for(;;);
		}
	}
}

main() {
	void *p1;
	struct r *p;
	unsigned size;
	unsigned i;
	int all;

	i = 0;
	srand(0);
	for(;;) {
		tstcheck();
		i++;
		switch(rand() % 4) {
		case 0:
			for(p = r; p < &r[sizeof r / sizeof *r]; p++) {
				if(p->p == 0) {
					printf("ittr=%-8d malloc(%u)=", i, size = rand());
					fflush(stdout);
					p->p = malloc(p->size = size);
					printf("%p\n", p->p);
					p->size = -p->size;
					data_set(p);
					break;
				}
			}
			break;
		case 1:
			for(p = r; p < &r[sizeof r / sizeof *r]; p++) {
				if(p->p == 0) {
					printf("ittr=%-8d smalloc(%u)=", i, size = rand());
					fflush(stdout);
					p->p = _smalloc(p->size = size);
					printf("%p\n", p->p);
					data_set(p);
					break;
				}
			}
			break;
		case 2:
			for(p = r; p < &r[sizeof r / sizeof *r]; p++) {
				if(p->p) {
					while((p = &r[rand() % (sizeof r / sizeof *r)]) && !p->p);
					if(p->size > 0) {
						printf("ittr=%-8d srealloc(%p, %u, %u)=", i, p->p, p->size, size = rand());
						fflush(stdout);
						data_check(p, 0);
						if(p1 = _srealloc(p->p, p->size, size)) {
							if(size) {
								if(size > p->size) {
									data_check(p, p1);
									p->p = p1;
									p->size = size;
									data_set(p);
								} else {
									p->size = size;
									data_check(p, p1);
									p->p = p1;
								}
							}
						} else if(size) {
							data_check(p, 0);
						} else {
							p->p = 0;
							p->size = 0;
						}
						printf("%p\n", p->p);
					} else {
						printf("ittr=%-8d realloc(%p, %u)=", i, p->p, size = rand());
						fflush(stdout);
						data_check(p, 0);
						if(p1 = realloc(p->p, size)) {
							if(size) {
								if(size > -p->size) {
									data_check(p, p1);
									p->p = p1;
									p->size = -size;
									data_set(p);
								} else {
									p->size = -size;
									data_check(p, p1);
									p->p = p1;
								}
							}
						} else if(size) {
							data_check(p, 0);
						} else {
							p->p = 0;
							p->size = 0;
						}
						printf("%p\n", p->p);
					}
					break;
				}
			}
			break;
		case 3:
			all = (rand() % 10000) == 0;
			for(p = r; p < &r[sizeof r / sizeof *r]; p++) {
				if(p->p) {
					if(!all) {
						while((p = &r[rand() % (sizeof r / sizeof *r)]) && !p->p);
					}
					if(p->size > 0) {
						printf("ittr=%-8d sfree(%p, %u)=", i, p->p, p->size);
						fflush(stdout);
						data_check(p, 0);
						sfree(p->p, p->size);
					} else {
						printf("ittr=%-8d free(%p)=", i, p->p);
						fflush(stdout);
						data_check(p, 0);
						free(p->p);
					}
					if(all) {
						printf("(ALL) ");
					}
					printf("done\n");
					p->p = 0;
					p->size = 0;
					if(!all) {
						break;
					}
				}
			}
			break;
		}
	}
}

void tstcheck(void) {
	struct free_entry *p1, *p2;
	volatile int err;

	for(a = al; a; a = a->next) {
		unsigned				size = 0;
		struct r				*p;

		for(p1 = _Sfree_list; p1; p1 = p1->next) {
			if((unsigned)p1 >= (unsigned)a->p && (unsigned)p1 < (unsigned)a->p + a->size) {
				size += p1->size;
			}
		}
		for(p = r; p < &r[sizeof r / sizeof *r]; p++) {
			if(p->p && (unsigned)p->p >= (unsigned)a->p && (unsigned)p->p < (unsigned)a->p + a->size) {
				unsigned			len = (p->size > 0) ? p->size : -p->size + sizeof(unsigned *);

				if(len) {
					len = len <= sizeof *_Sfree_list ? sizeof *_Sfree_list : ALIGN(len);
				}
				size += len;
			}
		}
		if(size != a->size) {
			printf("Size is %u, should be %u...", size, a->size);
			err = 1;
			goto error;
		}
	}
	for(p1 = _Sfree_list, p2 = 0; p1; p1 = (p2 = p1)->next) {
		if((unsigned)p1 & 3) {
			printf("Address is misaligned (%p)...", p1);
			err = 2;
			goto error;
		}
		for(a = al; a; a = a->next) {
			if((unsigned)a->p & 3) {
				printf("Address is misaligned (%p)...", a->p);
				err = 3;
				goto error;
			}
			if((unsigned)p1 >= (unsigned)a->p && (unsigned)p1 < (unsigned)a->p + a->size) {
				if((unsigned)p1 + p1->size > (unsigned)a->p + a->size) {
					printf("free size out of alloced area...");
					err = 4;
					goto error;
				}
				break;
			}
		}			
		if(!a) {
			printf("free entry not in any alloced area...");
			err = 5;
			goto error;
		}
		if(p1->next == p1) {
			printf("recursive free entry...");
			err = 6;
			goto error;
		}
		if(p1 <= p2) {
			printf("free list entry not sorted...");
			err = 7;
			goto error;
		}
		if(p2 && ((unsigned)p1 == (unsigned)p2 + p2->size)) {
			printf("free list entries not collessed...");
			err = 8;
			goto error;
		}
		if(p2 && ((unsigned)p1 < (unsigned)p2 + p2->size)) {
			printf("free list entries overlaping...");
			err = 9;
			goto error;
		}
	}
	return;
error:
	printf("Error %d\n", err);
	while(err);
}
#endif

#ifdef CHECK_HEAP
#ifndef __NEUTRINO__
void crash(char *file, unsigned line) {
	printf("Crash int %s, line %d\n", file, line);
	exit(1);
}
unsigned memory(void far *addr, void *src, unsigned len) {
	return len;
}
#endif
static void check(struct free_entry *p) {
	extern unsigned memory(void far *addr, void *src, unsigned len);
	
	if(p) {
//		if(memory(0, p, sizeof p) < sizeof p) {
//			crash(__FILE__, __LINE__);
//		}
		p = p->next;
	}
	while(p) {
		unsigned			size;

		if((unsigned)p & 3) {
			crash(__FILE__, __LINE__);
		}
//		if((size = memory(0, p, sizeof *p)) < sizeof *p) {
//			crash(__FILE__, __LINE__);
//		}
		if((unsigned)p->next & 3) {
			crash(__FILE__, __LINE__);
		}
		if(p->size & 3) {
			crash(__FILE__, __LINE__);
		}
		if(p->next && (unsigned)p >= (unsigned)p->next) {
			crash(__FILE__, __LINE__);
		}
		if(p->next && (unsigned)p->next - (unsigned)p <= p->size) {
			crash(__FILE__, __LINE__);
		}
//		size = memory(0, p, p->size);
//		if(size < p->size) {
//			crash(__FILE__, __LINE__);
//		}
		p = p->next;
	}
}
#endif

__SRCVERSION("_salloc.c $Rev: 153052 $");
