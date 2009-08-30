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



#ifndef __LDD_H_INCLUDED__
#define __LDD_H_INCLUDED__

/* Common headers headers for the dynamic linker source files */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <libgen.h>
#include <inttypes.h>
#include <limits.h>
#include <unistd.h>
#include <unix.h>
#include <share.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/elf.h>
#include <sys/elf_386.h>
#include <sys/elf_dyn.h>
#include <sys/auxv.h>
#include <sys/syspage.h>
#include <sys/memmsg.h>
#include <sys/link.h>

/* Common macros for relocation */

#define RELOC(_o, _a)           (((_a) < (_o)->data_offset) ? (_a) + (_o)->text_rel : (_a) + (_o)->data_rel)
#define RELOCP(_o, _p)          ((void *)RELOC((_o), (unsigned)(_p)))
#define RELOFFSET(reversed,arg) ((reversed) ? (-(arg)) : (arg))

/* List handling structure and macros */
typedef struct list_head        list_head_t;

struct list_head {
        list_head_t             *next;
        list_head_t             *prev;
};

#define _iszero(_q)             (!(_q)->next && !(_q)->prev)
#define _isempty(_q)            ((_q)->next == (_q))
#define _islast(_q,_e)          ((_e)->next == (_q))
#define _atend(_q,_e)           ((_e) == (_q))
#define _empty(_q)              ((_q)->next = (_q)->prev = (_q))
#define _first(_q)              ((void *)((_q)->next))
#define _last(_q)               ((void *)((_q)->prev))
#define _next(_e)               ((void *)(((list_head_t *)(_e))->next))
#define _prev(_e)               ((void *)(((list_head_t *)(_e))->prev))
#define _delete(_e)             ((_e)->prev->next = (_e)->next, (_e)->next->prev = (_e)->prev)
#define _insert(_q,_e)          ((_e)->next = (_q),                     \
                                         (_e)->prev = (_q)->prev,       \
                                         (_q)->prev->next = (_e),       \
                                         (_q)->prev = (_e))
#define _append(_q,_e)          ((_e)->next = (_q)->next,               \
                                         (_e)->prev = (_q),             \
                                         (_q)->next->prev = (_e),       \
                                         (_q)->next = (_e) )
#define _forward(_q,_e)         for ((_e) = _first(_q); !_atend((void *)(_q), (_e)); (_e) = _next(_e))
#define _backward(_q,_e)        for ((_e) = _last(_q); !_atend((void *)(_q), (_e)); (_e) = _prev(_e))

#define list_iszero(_q)         _iszero((list_head_t *)(_q))
#define list_isempty(_q)        _isempty ((list_head_t *)(_q))
#define list_islast(_q,_e)      _islast((list_head_t *)(_q), (list_head_t *)(_e))
#define list_atend(_q,_e)       _atend((list_head_t *)(_q), (list_head_t *)(_e))
#define list_empty(_q)          _empty((list_head_t *)(_q))
#define list_first(_q)          _first((list_head_t *)(_q))
#define list_last(_q)           _last((list_head_t *)(_q))
#define list_next(_q)           _next((list_head_t *)(_q))
#define list_prev(_q)           _prev((list_head_t *)(_q))
#define list_delete(_e)         _delete((list_head_t *)(_e))
#define list_insert(_q,_e)      _insert((list_head_t *)(_q), (list_head_t *)(_e))
#define list_append(_q,_e)      _append((list_head_t *)(_q), (list_head_t *)(_e))
#define list_forward(_q,_e)     _forward((list_head_t *)(_q), (_e))
#define list_backward(_q,_e)    _backward((list_head_t *)(_q), (_e))

/* Link map list macros (quite ugly, because of where the ptr's are) */
#define OBJ(_o)					((struct object *)_o)
#define PREV(_o)				(OBJ(_o)->_link_map.l_prev)
#define NEXT(_o)				(OBJ(_o)->_link_map.l_next)
#define LMPREV(_o)				(_o->l_prev)
#define LMNEXT(_o)				(_o->l_next)

#define link_map_delete(_q,_o)	if(PREV(_o)) {						\
									PREV(_o)->l_next = NEXT(_o);	\
								} else if(OBJ(*_q) == OBJ(_o)) {	\
									(*_q) = NEXT(_o);				\
								}									\
								if(NEXT(_o)) {						\
									NEXT(_o)->l_prev = PREV(_o);	\
								}
#define link_map_insert(_q,_e) 	NEXT(_e) = (_q);					\
									if(_q) { PREV(_e) = PREV(_q);	\
										(PREV(_q)) ? (PREV(_q)->l_next = (_e)) : (_q);\
										PREV(_q) = (_e); }

#define link_map_append(_q,_e) 		{									\
									struct link_map *_lm = _q;			\
									while(_lm && LMNEXT(_lm)) _lm = LMNEXT(_lm);		\
									PREV(_e) = (_lm);					\
									if(_lm) { NEXT(_e) = LMNEXT(_lm);	\
										(LMNEXT(_lm)) ? (LMNEXT(_lm)->l_prev = (_e)) : (_lm);\
										LMNEXT(_lm) = (_e); }}

/* Object flags */
#define OBJFLAG_INIT            0x0001  /* object has .init processing done */
#define OBJFLAG_RESOLVED        0x0002  /* object has been resolved */
#define OBJFLAG_FINI            0x0004  /* Don't do .fini section */
#define OBJFLAG_SYMBOLIC        0x0010  /* DT_SYMBOLIC */
#define OBJFLAG_LAZY            0x0020  /* lazy binding for code references */
#define OBJFLAG_RELSDONE        0x0040  /* Relative relocs have been done */
#define OBJFLAG_REVERSED        0x0080  /* Reverse relocations (for bootstrapping) */
#define OBJFLAG_NOSONAME		0x0100 	/* No soname in object */
#define OBJFLAG_EXECUTABLE		0x0200 	/* Object is the program */
#define OBJFLAG_BEGANINIT		0x0400 	/* Init process was started on this object, but not finished */
#define OBJFLAG_BEGANFINI		0x0800 	/* Fini process was started on this object, but not finished */
#define OBJFLAG_TEXTREL			0x1000 	/* Object has text segment relocations */
#define OBJFLAG_INITARRAY		0x2000  /* object has .init_array done */
#define OBJFLAG_FINIARRAY		0x4000  /* object jas .fini_array done */
#define OBJFLAG_PREINIT			0x8000  /* object has .preinit_array  done */
#define OBJFLAG_BEGANINITARRAY		0x10000  /* .init_array process was started on this object, but not finished */ 
#define OBJFLAG_BEGANFINIARRAY		0x20000  /* .fini_array process was started on this object, but not finished */ 
#define OBJFLAG_BEGANPREINIT		0x40000  /* .preinit_array process was started on this object, but not finished */ 
#define OBJFLAG_NEW_RELOCS			0x80000  /* newer version of MIPS relocs present */
#define OBJFLAG_LD_PRELOAD		0x100000	/* mark the object as LD_PRELOAD-ed */

/* Mutex functions to protect lists */
#ifndef __WATCOMC__
#define LOCK_LIST               pthread_mutex_lock(&_dl_list_mutex)
#define UNLOCK_LIST             pthread_mutex_unlock(&_dl_list_mutex)
#else
#define LOCK_LIST
#define UNLOCK_LIST
#endif

/* Env. var for directory search of libraries */
#define LIBPATHENV              "LD_LIBRARY_PATH"

#define ROUNDDOWN(val, round)   (((val)) & ~((round)-1))
#define ROUNDUP(val, round)     ROUNDDOWN((val) + ((round)-1), round)


/* Dynamic object info structure */
struct object {
		struct link_map					_link_map;
        int                             refcount;
        unsigned                        flags;
        const char                      *name;
        const Elf32_Dyn                 *dynamic;
        const char                      *strings;
        const Elf32_Sym                 *symbols;
        const unsigned long             *hash;
        const char                      *rpath;
		uint32_t						*got;
        uintptr_t                       text_addr;      /* Start of code area in memory */
        size_t                          text_size;      /* Size of code area in memory */
        ptrdiff_t                       text_rel;       /* Relocation of code between memory and shared object
 */
        ptrdiff_t                       data_offset;    /* Offset from start of code to start of data */
        size_t                          data_size;      /* Size of data area in memory */
        ptrdiff_t                       data_rel;       /* Relocation of data between memory and shared object
 */
		struct stat						sinfo;

		const char						*notes;
		size_t							note_segment_size;
};

#define OBJLISTFLAG_REMOVE				0x1
struct objlist {
        list_head_t                     list;
        struct object                   *object;
        list_head_t                     *root;
		unsigned						flags;
};

/* Forward declaration of CPU-specific functions */
void bind_func(void);
void _start_(void);

/* Function to retrieve the dll list from the syspage */
void *_dll_list(void);

/* Fwd to get a past other mmap() covers */
#define ldd_mmap(_vaddr,_len,_prot,_mflag,_fd,_off) \
    _mmap2((_vaddr),(_len),(_prot),(_mflag),(_fd),(_off),0,0,0,0)
#define ldd_munmap(_vaddr,_len) \
    munmap_flags((_vaddr),(_len),0)

#if defined(__GNUC__)
#define get_return_address()	__builtin_return_address(0)
#elif defined(__WATCOMC__)
#define get_return_address()	(void *)0
#else
#error Platform not defined for get_return_address()
#endif


#endif

/* __SRCVERSION("ldd.h $Rev: 201235 $"); */
