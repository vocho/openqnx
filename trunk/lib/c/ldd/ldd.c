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




#include "ldd.h"
#include "externs.h"
#include "init.h"
#define TEMP_HACK
#ifdef TEMP_HACK
#include <sys/memmsg.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX	1024 	/* Should use fpathconf(path, _PC_PATH_MAX) */
#endif

/* For resolve_func(), _do_exit_fini(), and ldd(), gcc-2.95.3 needs the 
   'unused' attribute, or else it complains about a static function that 
   is never used.  Newer gcc's need the 'used' attribute, which tells gcc 
   that code must be emitted for this function even if it appears to be 
   unreferenced, otherwise the function gets eliminated since it appears 
   to be unused.  */ 
#ifdef LDD_C_USEAGE_ATTRIBUTE
#error LDD_C_USEAGE_ATTRIBUTE already defined
#else
#if defined(__GNUC__) && (__GNUC__ < 3)
#define LDD_C_USEAGE_ATTRIBUTE __attribute__((unused))
#else
#define LDD_C_USEAGE_ATTRIBUTE __attribute__((used))
#endif
#endif

#include "xsupport.ci"

static void _dl_debug_state(void);

volatile struct r_debug _r_debug = {
	R_DEBUG_VERSION,
	0,
	(uintptr_t)&_dl_debug_state,
	RT_CONSISTENT,
	0,
	0,
	RD_NONE,
	RD_FL_NONE
};

static void _dl_debug_state(void) {
	_r_debug.r_rdevent = RD_NONE;
}

extern list_head_t		_dl_all_objects;
static list_head_t		_dl_start_objects;
static list_head_t		_dl_handle_list;

static char				*_dl_error;

#define LIBPATHENV		"LD_LIBRARY_PATH"
#define LDPRELOAD		"LD_PRELOAD"
static char				*_system_libpath;

/* Flag set if we are setuid; we don't search LI_LIBRARY_PATH then */
static int				_is_setuid;			

/*
 * These are the error strings returned by dlerror()
 * (silly way of doing errors, but that's what the spec says)
 */
static char * _dl_error_table[] = {
		"No dynamic section in executable",
		"Shared library is corrupted",
		"Library cannot be found",
		"Not enough memory",
		"Symbol not found",
		"Unresolved symbols",
		"Invalid handle",
		"Invalid mode",
		};

static int _trace_loaded_objects;

#define ERROR_NO_DYN		(_dl_error=_dl_error_table[0])
#define ERROR_BAD_LIB 		(_dl_error=_dl_error_table[1])
#define ERROR_NOT_FOUND		(_dl_error=_dl_error_table[2])
#define ERROR_NO_MEMORY 	(_dl_error=_dl_error_table[3])
#define ERROR_NO_SYM		(_dl_error=_dl_error_table[4])
#define ERROR_UNRES_SYM		(_dl_error=_dl_error_table[5])
#define ERROR_INV_HANDLE	(_dl_error=_dl_error_table[6])
#define ERROR_BAD_MODE		(_dl_error=_dl_error_table[7])


#ifdef __WATCOMC__

#define __attribute__(x)	
#pragma disable_message(202);

static int resolve(const list_head_t *this, int mode) { return -1; }
static int resolve_rels(const Elf32_Rel *rel, int n, struct objlist *o, int mode) { return -1; }
static void relocate_local_got(struct object *obj) { }

#endif

static int resolve(const list_head_t *this, int mode);
static int resolve_rels(const Elf32_Rel *rel, int n, struct objlist *o, int mode);
static void relocate_local_got(struct object *obj);

static size_t (*ldd_strlen)(const char *s);
static ssize_t (*ldd_write)(int fd, const void *buff, size_t nbytes);
static void (*ldd_exit)(int status);

static void __ldd_run_all_handlers(struct object *obj, unsigned type);

static void error(const char *msg, const char *msg2) {
	(void) ldd_write(STDERR_FILENO, msg, ldd_strlen(msg));
	if(msg2) {
		(void) ldd_write(STDERR_FILENO, msg2, ldd_strlen(msg2));
	}
	(void) ldd_write(STDERR_FILENO, "\n", 1);
	ldd_exit(EXIT_FAILURE);
}

/* @@@ overridable by executable */

void *_dl_alloc(int size) {
	return xmalloc(size);
}

void _dl_free(void* ptr) {
	xfree(ptr);
}

#ifdef TEMP_HACK
static void debug_info(void *vaddr, Elf32_Phdr *phdr, int fd, struct object *obj) {
	mem_debug_info_t			msg;
	iov_t						iov[2];

	msg.i.type = _MEM_DEBUG_INFO;
	msg.i.zero = msg.i.reserved = 0;
	msg.i.offset = phdr->p_offset;
	msg.i.ino = 0;
	msg.i.vaddr = (uintptr_t)vaddr;
	msg.i.size = phdr->p_memsz;
	msg.i.flags = 0;
	msg.i.dev = 0;
	msg.i.old_vaddr = phdr->p_vaddr;
	msg.i.path[0] = '\0';
	SETIOV(iov + 0, &msg.i, offsetof(mem_debug_info_t, i.path));
	SETIOV(iov + 1, obj->name, strlen(obj->name) + 1);
	
	(void) MsgSendv(MEMMGR_COID, iov, 2, 0, 0);
}
#endif

static void vector_decode(unsigned long *dst, int size, const void *src, uint32_t offset) {
	const struct {
		unsigned long		key;
		unsigned long		val;
	}					*s = src;

	xmemset(dst, -1, size * sizeof *dst);
	while (s->key) {
		if ((s->key - offset) < size) {
			dst[(s->key - offset)] = s->val;
		}
		s++;
	}
}

static void stuff_path_and_time(struct link_map *lm, const char *path)
{
      lm->l_path = (char *)path;
      lm->l_loaded = time(NULL);
}

static void phdr_walk(uintptr_t phdrp, int num, struct object *obj, intptr_t data, char **interp) {
	const Elf32_Phdr	*phdr;
	int					i;

	phdr = (const Elf32_Phdr *)(phdrp + obj->text_addr);
	for (i = 0; i < num; i++, phdr++) {
		if (phdr->p_type == PT_LOAD) {
			if(phdr->p_flags & PF_W) {
				if(data == -1) {
					data = phdr->p_vaddr + obj->text_rel;
				}
				obj->data_rel = data - phdr->p_vaddr;
				obj->data_offset = phdr->p_vaddr - (obj->text_addr - obj->text_rel);
				obj->data_size = phdr->p_memsz;
			} else {
				obj->text_rel = obj->text_addr ? 
					(obj->text_addr - phdr->p_vaddr) : 0;
				obj->text_addr = obj->text_addr ? 
					obj->text_addr : phdr->p_vaddr;
				obj->text_size = phdr->p_memsz;
			}
		} else if (phdr->p_type == PT_DYNAMIC) {
			obj->dynamic = RELOCP(obj, phdr->p_vaddr);
			obj->_link_map.l_ld = RELOCP(obj, phdr->p_vaddr);
		} else if (phdr->p_type == PT_INTERP && interp) {
			*interp = (char *)RELOCP(obj, phdr->p_vaddr);
		}
	}
	obj->_link_map.l_addr = obj->text_addr;
}

static unsigned long hash(const unsigned char *name) {
	unsigned long		h = 0, g;

	while (*name) {
		h = (h << 4) + *name++;
		if ((g = h & 0xf0000000)) {
			h ^= g >> 24;
		}
		h &= ~g;
	}
	return h;
}

#include <ldd_mod.h>

static const Elf32_Sym *hashed_lookup(const char *sym, unsigned long h, const struct object *obj) {
	unsigned long		nb, hv;

	if (obj->symbols && obj->hash) {
		nb = obj->hash[0];
		hv = obj->hash[static_mod(h,nb) + 2];
		do {
			if (!xstrcmp(sym, obj->strings + obj->symbols[hv].st_name)) {
				return &obj->symbols[hv];
			}
			hv = obj->hash[hv + nb + 2];
		} while (hv);
	}
	return 0;
}

static int dynamic_decode(struct object *obj) {
	unsigned long		dynamic[50];

	vector_decode(dynamic, sizeof dynamic / sizeof *dynamic, obj->dynamic, 0);

	obj->strings = RELOCP(obj, dynamic[DT_STRTAB]);
	obj->symbols = RELOCP(obj, dynamic[DT_SYMTAB]);

	if (!obj->name) {
		if (dynamic[DT_SONAME] != (unsigned long)-1) {
			obj->name = obj->strings + dynamic[DT_SONAME];
			obj->_link_map.l_name = (char *)obj->strings + dynamic[DT_SONAME];
		} else {
			obj->name = 0;
		}
	}
	if (dynamic[DT_HASH] != (unsigned long)-1) {
		obj->hash = RELOCP(obj, dynamic[DT_HASH]);
	}
	if (dynamic[DT_SYMBOLIC] != (unsigned long)-1) {
		obj->flags |= OBJFLAG_SYMBOLIC;
	}
	if (dynamic[DT_RPATH] != (unsigned long)-1) {
		obj->rpath = obj->strings + dynamic[DT_RPATH];
	}
	if (dynamic[DT_PLTGOT] != (unsigned long)-1) {
		obj->got = RELOCP(obj, dynamic[DT_PLTGOT]);

		relocate_local_got(obj);
	}
	return 0;
}


/*
 * Note to future implementors - should you ever try to support
 * lazy binding, make sure you tell the gdb guys - since they
 * need to add support to gdb for this too!
 */
static const Elf32_Sym *lookup_global(const char *name, const list_head_t *this, const struct object *omit, int jumpslot, struct object **obj) {
	struct objlist		*ol;
	const Elf32_Sym		*sym, *wsym = 0;
	unsigned long		h;

	h = hash((const unsigned char *)name);
	list_forward(this, ol) {
		if (ol->object != omit && (sym = hashed_lookup(name, h, ol->object))) {
			int					bind;
			/*
			 * objects may label the PLT entries by declaring
			 * symbols within SHN_UNDEF, a non-zero value and
			 * STT_FUNC type.  When processing non-jump slot
			 * relocations this symbol is used instead of the
			 * real symbol.  This is done so that function
			 * address comparisions will work correctly between
			 * different objects.
			 */

			bind = ELF32_ST_BIND(sym->st_info);
			if (sym->st_shndx == SHN_UNDEF) {
				if (jumpslot) {
					continue;
				}
				if (sym->st_value == 0 || ELF32_ST_TYPE(sym->st_info) != STT_FUNC) {
					continue;
				}
			}
			if(bind == STB_GLOBAL || bind == STB_GLOBALOMIT) {
				*obj = ol->object;
				return sym;
			}
			if(bind == STB_WEAK || bind == STB_LAZY || bind == STB_LOCAL) {
				if (!wsym) {
					*obj = ol->object;
					wsym = sym;
				}
			}
		}
	}

	return wsym;
}

static void LDD_C_USEAGE_ATTRIBUTE *resolve_func(struct object *obj, unsigned reloff) {
	unsigned long		vec[50];

	vector_decode(vec, sizeof vec / sizeof *vec, obj->dynamic, 0);
	if (vec[DT_REL] != (unsigned long)-1) {
        const Elf32_Rel		*rel = (Elf32_Rel *)RELOCP(obj, vec[DT_REL] + reloff);
		const Elf32_Sym		*sym;

		// @@@ Need to pass the proper object list; won't work now...
		(void) resolve_rels(rel, 1, (struct objlist *)obj, RTLD_NOW);

		sym = &obj->symbols[ELF32_R_SYM(rel->r_info)];
		return RELOCP(obj, sym->st_value);
	}
	return (void *)-1;
}




/* getenv which doesn't result in errno being set on failure. */
static char * getenv_ne(char * env_str) {
	int 			cur_errno = errno;
	char 			*retrn = getenv(env_str);

	errno = cur_errno;
	return(retrn);
	
}



/*
 * TODO: if mmap fails at base adress, do a MAP_ANON for the whole size
 * and let the library fall wherever.
 */

static int load_elf32(struct object *obj, int fd) {
	Elf32_Ehdr		ehdr;
	Elf32_Phdr		*phdr,*nphdr;
	int				n, lseg;
	void			*vaddr;
	void			*base, *new_base;
	unsigned		size = 0, finished = 0;

	if ((read(fd, &ehdr, sizeof ehdr) != sizeof ehdr) || memcmp(ehdr.e_ident, ELFMAG, SELFMAG) || ehdr.e_type != ET_DYN) {
		ERROR_BAD_LIB;
		return -1;
	}
	n = sizeof *phdr * ehdr.e_phnum;
	phdr = _alloca(n);
	if (lseek(fd, ehdr.e_phoff, SEEK_SET) == -1 || read(fd, phdr, n) != n) {
		ERROR_BAD_LIB;
		return -1;
	}
	base = (void *)-1;
	nphdr = phdr;
	for (n = 0, lseg = 0; n < ehdr.e_phnum; n++, nphdr++) {
		if (nphdr->p_type == PT_LOAD) {
			if(base == (void *)-1) {
				base = (void *)nphdr->p_vaddr;
				size = (nphdr->p_vaddr + nphdr->p_memsz) - (uintptr_t) base;
			} else {
				size = (nphdr->p_vaddr + nphdr->p_memsz) - (uintptr_t) base;
			}
		}
	}
	// Now we should have a base and proper size; do a MAP_LAZY|MAP_ELF to 
	// get a new base
	if((new_base = mmap(base, size, PROT_NONE, MAP_ELF|MAP_ANON|MAP_LAZY|MAP_PRIVATE, NOFD, 0)) == MAP_FAILED) {
		ERROR_NO_MEMORY;
		return -1;
	}
	while(!finished) {
	nphdr = phdr;
	for (n = 0, lseg = 0; n < ehdr.e_phnum; n++, phdr++) {
		if (phdr->p_type == PT_LOAD) {
			int		prot = 0;
	
			prot |= (phdr->p_flags & PF_R) ? PROT_READ : 0;
			prot |= (phdr->p_flags & PF_W) ? PROT_WRITE : 0;
			prot |= (phdr->p_flags & PF_X) ? PROT_EXEC : 0;
			if ( obj->flags & OBJFLAG_TEXTREL )
				prot |= PROT_WRITE;

			vaddr = mmap((void *)((phdr->p_vaddr - (uintptr_t)base) + (uintptr_t)new_base),
					phdr->p_filesz, prot, 
					lseg || obj->flags & OBJFLAG_TEXTREL ? MAP_ELF | MAP_PRIVATE | MAP_NOSYNCFILE | MAP_FIXED:
					MAP_ELF | MAP_SHARED | MAP_NOSYNCFILE | MAP_FIXED, fd, phdr->p_offset);

			if (vaddr == MAP_FAILED) {   // OK, let's do a MAP_ANON...
				vaddr = mmap((void *)((phdr->p_vaddr - (uintptr_t)base) +
							(uintptr_t)new_base), phdr->p_memsz,prot | PROT_WRITE, 
							MAP_ANON | MAP_ELF | MAP_PRIVATE, NOFD, (phdr->p_vaddr - (off_t)base) + (off_t)new_base);

if(getenv_ne("DL_DEBUG")) {
		xprintf("load_elf32: mmaped, addr %p %p vaddr %p\n", 
			base, (char *)base + phdr->p_vaddr, vaddr);
}
				if ((vaddr == MAP_FAILED) || (lseg && (uintptr_t)vaddr != ((phdr->p_vaddr - (uintptr_t)base) + (uintptr_t)new_base))) {
					if(lseg) munmap((void *) obj->text_addr, obj->text_size);
					ERROR_NO_MEMORY;
					return -1;
				}
#ifdef TEMP_HACK
				// @@@ tmp for 4M lib problem
				if (phdr->p_filesz > 0x3ff000) {
					int			bytes, nbytes;

					for(nbytes = 0; nbytes < phdr->p_filesz; nbytes += bytes) {
						if((bytes = readblock(fd, 1, phdr->p_offset + nbytes, 
							min(phdr->p_filesz - nbytes, 0x3ff000), (void *)((unsigned) vaddr + nbytes))) == -1 || bytes == 0) {
							munmap(vaddr, phdr->p_memsz);
							if(lseg) munmap((void *)obj->text_addr, obj->text_size);
							ERROR_NO_MEMORY;
							return -1;
						}
					}
				}
				else 			
#endif
				if (lseek(fd, phdr->p_offset, SEEK_SET) == -1 || 
					read(fd, vaddr, phdr->p_filesz) != (phdr->p_filesz)) {
					munmap(vaddr, phdr->p_memsz);
					if(lseg) munmap((void *)obj->text_addr, obj->text_size);
					ERROR_NO_MEMORY;
					return -1;
				}
				mprotect(vaddr, phdr->p_memsz, prot);
			} else if (lseg && phdr->p_memsz > phdr->p_filesz) {
				uintptr_t fil_end, mem_end;

				fil_end = ROUNDUP((uintptr_t)vaddr + phdr->p_filesz, __PAGESIZE);
				mem_end = ROUNDUP((uintptr_t)vaddr + phdr->p_memsz, __PAGESIZE);
				if ( mem_end > fil_end ) {
					/* allocate remainder of bss pages */
					if(mmap((void *)fil_end, mem_end-fil_end, prot,
								MAP_ELF|MAP_FIXED|MAP_ANON|MAP_PRIVATE, NOFD, 0) == MAP_FAILED) {
						munmap(vaddr, phdr->p_filesz);
						if(lseg) munmap((void *)obj->text_addr, obj->text_size);
						ERROR_NO_MEMORY;
						return -1;
					}
				}
				memset((char *)vaddr + phdr->p_filesz, 0x00, phdr->p_memsz - phdr->p_filesz);
			}
			
			if (!lseg) {	
				/* vaddr could be adjusted by mmap to be not equal to the required addr */
				new_base = (void*) (((uintptr_t)vaddr - (uintptr_t)phdr->p_vaddr) + (uintptr_t)base);

				obj->text_addr = (uintptr_t)vaddr;
				obj->text_size = phdr->p_memsz;
				obj->text_rel = (unsigned)vaddr - phdr->p_vaddr;
				(void) msync(vaddr, phdr->p_memsz, MS_INVALIDATE_ICACHE);
			}
			if (phdr->p_flags & PF_W) {
				obj->data_offset = (ptrdiff_t) vaddr - obj->text_addr;
				obj->data_size = phdr->p_memsz;
				obj->data_rel = (uintptr_t)vaddr - phdr->p_vaddr;
			}
#ifdef TEMP_HACK
			debug_info(vaddr, phdr, fd, obj);
#endif
			lseg ++;	
		} else if (phdr->p_type == PT_DYNAMIC) {
			obj->dynamic = (void *)phdr->p_vaddr;
		}
	}

	obj->_link_map.l_addr = obj->text_addr;
	obj->dynamic = RELOCP(obj, obj->dynamic);
	obj->_link_map.l_ld = (Elf32_Dyn *)obj->dynamic;
	finished = 1;

#ifndef __MIPS__
	if ( !(obj->flags & OBJFLAG_TEXTREL) ) {
	Elf32_Dyn *dyn;

		for ( dyn = (Elf32_Dyn *)obj->dynamic; dyn->d_tag != 0; dyn++ )
			if ( dyn->d_tag == DT_TEXTREL ) {
				/* the presence of DT_TEXTREL indicates that there
				are relocations in the shared object against a read-only
				section.  For this reason we need to go back and map in
				the text segment as writeable and private */
				if(getenv_ne("DL_DEBUG")) {
					xprintf("load_elf32: found DT_TEXTREL, mapping a private copy of text sections!\n");
				}
				obj->flags |= OBJFLAG_TEXTREL;
				phdr = nphdr;
				finished = 0;
			}
	}
#endif
	}


	if(getenv_ne("DL_DEBUG")) {
		xprintf("load_elf32: loaded lib at addr %x(text) %x(data)\n", 
			obj->text_addr, obj->text_addr + obj->data_offset);
	}
	return 0;
}

static const char *name_only(const char *pathname) {
	const char			*p;

	for (p = pathname + strlen(pathname); p != pathname && p[-1] != '/'; --p) {
		//nothing to do
	}
	return p;
}

static int searchpath(const char *name, const char *path, int amode, char *buffer, int bufsize) {
	char				*b;
	int					n;

	if (path == NULL) {
		return -1;
	}
	bufsize -= strlen(name);
	do {
		for (n = bufsize, *(b = buffer) = 0; path && *path && *path != ':'; n--, path++) {
			if (n > 0) {
				*b++ = *path;
			}
		}
		if (n > 0) {
			int curr_err = errno;
			if (*buffer && b[-1] != '/') {
				*b++ = '/';
			}
			strcpy(b, name);
			if (eaccess(buffer, amode) != -1) {
				return 0;
			}
			/* Don't change the errno value. */
			errno = curr_err;
		}
		if (path && *path == ':') {
			path++;
		}
	} while (path && *path);
	*buffer = 0;
	return -1;
}

static int find_file(const char *name, const char *libpath, const char *rpath, const char **fullpath) {
	char				pathname[PATH_MAX + 1];
	int					fd;

	/*
	 * Search standard directories in the right order, 
	 * see ABI section on the dynamic loader. Order is
	 * DT_RPATH, LD_LIBRARY_PATH, /usr/lib(_CS_LIBPATH).
	 */  
	//PR 7654 and Unix98 say we look at the whole path for a slash character 
	for(fd = 0; name[fd] != '/' && name[fd] != '\0'; fd++) { ; }

	if (name[fd] != '/') {
		/* First - search DT_RPATH */
		if (searchpath(name, rpath, R_OK, pathname, sizeof pathname) == -1) {
			/* Next, search LD_LIBRARY_PATH, unless we are a suid executable */
			if (searchpath(name, libpath, R_OK, pathname, sizeof pathname) == -1) {
				/* Finally, search _CS_LIBPATH */
				if (searchpath(name, _system_libpath, R_OK, pathname, sizeof pathname) == -1) {
					return -1;
				}
			}
		}
		name = pathname;
	}
	if ((fd = sopen(name, O_RDONLY, SH_DENYWR)) == -1) {
		return -1;
	}
	if (fullpath) {
		*fullpath = xstrdup(name);
	}
	return fd;
}

/*
 If the name is specified, then the name must match.
 If the stat is specified, then the stat must match.
 If both are specified, then both must match.
*/
static struct object *find_object_stat(const list_head_t *list, const char *name, struct stat *st, struct objlist **listentry) {
	struct objlist		*l;

	list_forward(list, l) {
		if (l->object->name && 
		   (!name || strcmp(name, l->object->name) == 0) &&
		   (!st || (st->st_ino == l->object->sinfo.st_ino && st->st_dev == l->object->sinfo.st_dev))) {
			if(listentry) {
				*listentry = l;
			}
			return l->object;
		}
	}
	return 0;
}

static struct object *find_object(const list_head_t *list, const char *name, struct objlist **listentry) {
	return find_object_stat(list, name, NULL, listentry);
}

static const Elf32_Sym*
lookup_linkmap_scope(const char *name, struct object* target_obj, struct object* omit, struct object** robj) {
	Link_map *cur_lm;
	struct objlist *ol;
	struct objlist *to;
	const Elf32_Sym *ns_sym = NULL;
	list_head_t obj_linkmap;

	list_empty(&obj_linkmap);

	/*	I think linkmap order is correct, which means NOT looking at the
		main executable as that is upstream, not downstream of link map order.
		Scan our link map, assemble a list of objects in that order to search.
		We use the dynamic table pointer to match up objects to link map entries -- total fromage!
	*/
	for(cur_lm = &target_obj->_link_map; cur_lm ; cur_lm = cur_lm->l_next) {
		list_forward(&_dl_all_objects, ol) {
			if(cur_lm->l_ld == ol->object->dynamic) { /* fromage! */
				if(ol->object == omit)  break; /* skip it */

				to = _alloca(sizeof(*to));
				to->object = ol->object;
				to->root = &obj_linkmap;
				list_insert(&obj_linkmap, to);
				break; /* stop traversing the object list, get new linkmap */
			} 
		} 
	} 
	if(!list_isempty(&obj_linkmap)) {
		ns_sym = lookup_global(name, &obj_linkmap, omit, 0, robj);
	}
	return ns_sym;
}

static int load_object(struct object *obj, const char *name, const char *libpath, const char *rpath,
                       struct object **dupobj, list_head_t *objectlist) {
	int					fd, r;
	const char 			*fullpath = NULL;

	if(getenv_ne("DL_DEBUG")) {
		xprintf("load_object: attempt load of %s\n", (name) ? name : ""); 
	}
	if ((fd = find_file(name, libpath, rpath, &fullpath)) == -1) {
		ERROR_NOT_FOUND;
		return -1;
	}
	if(fstat(fd, &obj->sinfo) == -1) {
		if (fullpath != NULL) {
			_dl_free((char *)fullpath);
		}
		close(fd);
		ERROR_NOT_FOUND;
		return -1;
	}

	if(dupobj != NULL && objectlist != NULL &&
	   ((*dupobj = find_object_stat(objectlist, NULL, &obj->sinfo, NULL)) != NULL)) {	
		if (fullpath != NULL) {
			_dl_free((char *)fullpath);
		}
		close(fd);
		return 0;
	}

#ifdef TEMP_HACK
	obj->name = name_only(name);
#endif
	if ((r = load_elf32(obj, fd)) != 0) {
		if (fullpath != NULL) {
			_dl_free((char *)fullpath);
		}
		close(fd);
		// load_elf32 sets _dl_error
		return -1;
	}
#ifdef TEMP_HACK
	obj->name = 0;
#endif
	dynamic_decode(obj);
	if(!obj->name) { 
		obj->name = xstrdup(name_only(name));
		obj->flags |= OBJFLAG_NOSONAME;
		obj->_link_map.l_name = (char *)obj->name;
	}
        stuff_path_and_time(&obj->_link_map, fullpath);
	if ( _trace_loaded_objects )
		xprintf( "\t%s => %s (0x%x)\n", obj->name, fullpath, obj->text_addr);
	__ldd_run_all_handlers(obj, LDD_EH_DLL_LOAD);
	close(fd);
	return 0;
}

static int preinit_array(const list_head_t *objs) {
	struct objlist		*o;
	unsigned long		vec[50];
	void			(*func)(void) = NULL;
	int *p;
	int i, funcs; 
	list_backward(objs, o) {
		/* ignore DT_PREINIT_ARRAY in shared objects. GNU ld prevents
		   .preinit_array sections in shared objects so this check isn't
		   required. 
		*/ 
		if ((o->object->flags & (OBJFLAG_BEGANPREINIT | OBJFLAG_PREINIT))
		    || !(o->object->flags & OBJFLAG_EXECUTABLE)) { 
			continue;
		}
		vector_decode(vec, sizeof vec / sizeof *vec, o->object->dynamic, 0);
		if (vec[DT_PREINIT_ARRAY] != -1) {
			funcs = vec[DT_PREINIT_ARRAYSZ] / ELF32_FSZ_WORD;
			p = RELOCP(o->object, vec[DT_PREINIT_ARRAY]);
			o->object->flags |= OBJFLAG_BEGANPREINIT;
			for (i = 0; i < funcs; i++) {
				func = (void *)*p++;
				_r_debug.r_rdevent = RD_PREINIT;
				_dl_debug_state();
				func();
				_r_debug.r_rdevent = RD_POSTINIT;
				_dl_debug_state();
			}
		}
		o->object->flags &= ~OBJFLAG_BEGANPREINIT;
		o->object->flags |= OBJFLAG_PREINIT;
	}
	return 0;
}


static void init(const list_head_t *objs) {
	struct objlist		*o;
	unsigned long		vec[50];
	void				(*func)(void);

	list_backward(objs, o) {
		/*
		 If we started doing the init on an object and then that caused us 
		 to do another init processing on another object before we finished, 
		 then we should just skip the "in progress object".  Perhaps we should 
		 even jump out of the list at this point and not even init other
		 objects (can't for fpemu however).
		*/
		if (o->object->flags & (OBJFLAG_INIT | OBJFLAG_BEGANINIT)) {
			continue;
		}

		vector_decode(vec, sizeof vec / sizeof *vec, o->object->dynamic, 0);
		if (vec[DT_INIT] != (unsigned long)-1) {
			func = RELOCP(o->object, vec[DT_INIT]);
			_r_debug.r_rdevent = RD_PREINIT;
			_dl_debug_state();
			o->object->flags |= OBJFLAG_BEGANINIT;
			func();
			_r_debug.r_rdevent = RD_POSTINIT;
			_dl_debug_state();
		}
		o->object->flags &= ~OBJFLAG_BEGANINIT;
		o->object->flags |= OBJFLAG_INIT;
	}
}

static int init_array(const list_head_t *objs) {
	struct objlist		*o;
	unsigned long		vec[50];
	void			(*func)(void) = NULL;
	int *p;
	int i, funcs; 
	list_backward(objs, o) {
		vector_decode(vec, sizeof vec / sizeof *vec, o->object->dynamic, 0);
		if (o->object->flags  
	    	    & (OBJFLAG_BEGANINITARRAY | OBJFLAG_INITARRAY)) {
			continue;
		}
		if (vec[DT_INIT_ARRAY] != -1) {
			funcs = vec[DT_INIT_ARRAYSZ] / ELF32_FSZ_WORD;
			p = RELOCP(o->object, vec[DT_INIT_ARRAY]);
			o->object->flags |= OBJFLAG_BEGANINITARRAY;
			for (i = 0; i < funcs; i++) {
				func = (void *) *p++;
				_r_debug.r_rdevent = RD_PREINIT;
				_dl_debug_state();
				func();
				_r_debug.r_rdevent = RD_POSTINIT;
				_dl_debug_state();
			}
		}
		o->object->flags &= ~OBJFLAG_BEGANINITARRAY;
		o->object->flags |= OBJFLAG_INITARRAY;
	}
	return 0;
}

/* 
 Do the .fini processing for an object. Note that this takes an
 object pointer as an argument, not the list.
 */
static void fini(struct object *obj) {
	void			(*func)(void);
	unsigned long		vec[50];

	if (obj->flags & (OBJFLAG_FINI | OBJFLAG_BEGANFINI)) {
		return;
	}
	vector_decode(vec, sizeof vec / sizeof *vec, obj->dynamic, 0);
	if (vec[DT_FINI] != (unsigned long)-1) {
		func = RELOCP(obj, vec[DT_FINI]);
		obj->flags |= OBJFLAG_BEGANFINI;
		func();
	}
	obj->flags &= ~OBJFLAG_BEGANFINI;
	obj->flags |= OBJFLAG_FINI;
}

static int fini_array(struct object *obj) {
	void			(*func)(void) = NULL;
	unsigned long		vec[50];
	int *p;
	int i, funcs; 

	vector_decode(vec, sizeof vec / sizeof *vec, obj->dynamic, 0);
	if (obj->flags & (OBJFLAG_FINIARRAY | OBJFLAG_BEGANFINIARRAY)) {
		return 0;
	}

	if (vec[DT_FINI_ARRAY] != -1) {
		funcs = vec[DT_FINI_ARRAYSZ] / ELF32_FSZ_WORD;
		p = RELOCP(obj, (int *) vec[DT_FINI_ARRAY] + funcs);
		obj->flags |= OBJFLAG_BEGANFINIARRAY;
		for (i = 0; i < funcs; i++) {
			func = (void *) *--p;
			func();
		}
	}
	obj->flags |= OBJFLAG_FINIARRAY;
	obj->flags &= ~OBJFLAG_BEGANFINIARRAY;
	return 0;
}

/* 
 This function is installed by atexit and called at program exit to do the .fini
 sections of all loaded objects.
 */
static void LDD_C_USEAGE_ATTRIBUTE _do_exit_fini(void) {
	struct objlist 			*o;

	list_forward(&_dl_all_objects, o) {
		fini_array (o->object);
	}
	list_forward(&_dl_all_objects, o) {
		fini (o->object);
	}
}



/*
 * Return the address of r_debug to avoid the optimizer that may be getting the 
 * address of r_debug early (before we resolve the lib)
 */
static volatile struct r_debug * volatile _get_rdebug_addr(void) {
	return &_r_debug;
}

static char * volatile *_get_system_libpath_addr(void) {
	return &_system_libpath;
}

static volatile list_head_t *_get_dl_all_objects_addr(void) {
	return &_dl_all_objects;
}

static volatile list_head_t *_get_dl_start_objects_addr(void) {
	return &_dl_start_objects;
}


static volatile list_head_t *_get_dl_handle_list_addr(void) {
	return &_dl_handle_list;
}

static volatile int *_get_tlo_addr(void) {
	return &_trace_loaded_objects;
}
/*
 * Stuff DT_DEBUG in obj's dynamic section to point to the _r_debug
 * structure. Also initializes _r_debug if needed.
 */

static void _set_dt_debug(void *dynamic) {

	struct {
		unsigned long		key;
		void 				*val;
	}					*s = dynamic;

	while (s->key!= DT_DEBUG && s++->key) {
		//nothing to do
	}
	if(s->key == DT_DEBUG) {
		s->val = (void *)&_r_debug;
	}
}

static void resolve_error_funcs(list_head_t *bootstrap)
{
struct object *tobj;
const Elf32_Sym *sym;
	if ((sym = lookup_global("strlen", bootstrap, NULL, 1, &tobj ))) {
		ldd_strlen = RELOCP(tobj,sym->st_value);
	}
	if ((sym = lookup_global("write", bootstrap, NULL, 1, &tobj ))) {
		ldd_write = RELOCP(tobj,sym->st_value);
	}
	if ((sym = lookup_global("exit", bootstrap, NULL, 1, &tobj ))) {
		ldd_exit = RELOCP(tobj,sym->st_value);
	}
	if ( ldd_strlen == NULL || ldd_write == NULL || ldd_exit == NULL ) {
		/* should never happen */
		write( STDERR_FILENO, "symbol lookup failure!\n", 23 );
		_exit(EXIT_FAILURE);
	}
}

/* The startup frame is set-up so that we have:
	auxv
	NULL
	...
	envp2
	envp1 <----- void *frame + (argc + 2) * sizeof(char *)
	NULL
	...
	argv2
	argv1
	argc  <------ void * frame 

	On entry to ldd, frame gives the adress of argc on the stack.
*/

static uintptr_t LDD_C_USEAGE_ATTRIBUTE ldd (void *frame) {
	unsigned long		vec[50];
	list_head_t			objects, bootstrap;
	struct objlist		*o, *l, *bslist;
	struct object		self, other, *obj;
	Elf32_Ehdr			*ehdr;
	const Elf32_Dyn		*dyn;
	const char			*needed;
	char				*libpath, *ld_preload;
	char				**argv, **env, **p;
	auxv_t				*auxv;
	int					len;
	int					argc;
	volatile struct r_debug		* volatile rd;
	char 				*volatile *slp;
	volatile list_head_t		*dao;
	volatile list_head_t		*dso;
	volatile list_head_t		*dhl;
	char				*_interp;
	volatile int		*_tlop;

	argc = *(int *)frame;
	argv = (char **)((int *)frame + 1);
	p = env = (char **)frame + (*(int *)frame + 2);
	while(*p++) {
		//nothing to do
	}
	auxv = (auxv_t *)p;

	/*
	 * At this point the library is completely unresolved, only
	 * references to functions defined within the library (and not
	 * overridable) are permitted.  None of these functions can access
	 * static data.
	 * 
	 * We expect the following AT_ values in the aux vector:
	 * 
	 * AT_BASE		the base address of the interpreter (ehdr)
	 * AT_DATA		the address of the interpreter's data segment (optional)
	 * AT_PHDR		the address of the executable's program header
	 * AT_PHNUM		# of program headers in executable
	 * AT_PHENT		size of program header structure
	 */

	vector_decode(vec, sizeof vec / sizeof *vec, auxv, 0);


	/*
	 * find the interpreter dynamic segment, and data segment
	 */

	ehdr = (Elf32_Ehdr *)vec[AT_BASE];
	xmemset(&self, 0, sizeof self);
	self.text_addr = vec[AT_BASE];
	phdr_walk(ehdr->e_phoff, ehdr->e_phnum, &self, vec[AT_DATA], NULL);
	dynamic_decode(&self);
	self.sinfo.st_dev = vec[AT_INTP_DEVICE];	
	self.sinfo.st_ino = vec[AT_INTP_INODE];	

	/*
	 * @@@ the interpreter's name should be in the dynamic section, but
	 * it isn't
	 */
	if(!self.name) {
		self.name = RELOCP(&self, "libself.so");
	}

	/*
	 * find the executable dynamic segment, this is carried over
	 */

	xmemset(&other, 0, sizeof other);
	phdr_walk(vec[AT_PHDR], vec[AT_PHNUM], &other, -1, &_interp);
	dynamic_decode(&other);
	other.sinfo.st_dev = vec[AT_INTP_DEVICE];	
	other.sinfo.st_ino = vec[AT_INTP_INODE];	

	/*
	 * setup the object list
	 */

	list_empty(&objects);
	o = _alloca(sizeof *o);
	o->object = &other;
	/* Don't do init and fini for executable object itself */
	o->object->flags |= OBJFLAG_INIT | OBJFLAG_FINI | OBJFLAG_EXECUTABLE | OBJFLAG_INITARRAY | OBJFLAG_FINIARRAY;
	o->root = &objects;
	list_append(&objects, o);
	other.refcount++;

	/*
	 * As a first pass, resolve the PLT entries so that 
	 * we can at least use open/read to load any dependent libraries.
	 */
	list_empty(&bootstrap);
	bslist = _alloca(sizeof *bslist);
	bslist->object = &self;
	bslist->root = &bootstrap;
	list_append(&bootstrap, bslist);
	
	(void) resolve(&bootstrap, RTLD_NOW);
	self.flags &= ~(OBJFLAG_RESOLVED); /* Bootstrap is done */

	resolve_error_funcs(&bootstrap);

	/*
	 * Call _init_libc() so the libc internal variables are inited
	 */
	_init_libc(argc, argv, env, auxv, NULL);

	if(((unsigned) other.dynamic >= (other.text_addr + other.data_offset)) && ((unsigned) other.dynamic < (other.text_addr + other.data_offset + other.data_size))) {
		_set_dt_debug((void *)other.dynamic);
	}
	rd =_get_rdebug_addr(); 
	rd->r_state = RT_ADD;
	/* set r_ldbase and r_ldsomap */
	rd->r_ldbase = vec[AT_BASE];
	rd->r_ldsomap = NULL;

	libpath = NULL;
	ld_preload = NULL;
	_tlop = _get_tlo_addr();
	*_tlop = (int)getenv_ne("LD_TLO");

	/* From the sysv x86 ABI:
     *  AT_LIBPATH - The a_val member of this entry is non-zero if the dynamic 
     *  linker should examine LD_LIBRARY_PATH when searching for shared
     *  objects of the process based on the security considerations in the
     *  Shared Object Dependency section in Chapter 5 of the gABI.
     * No AT_LIBPATH (-1 value) will also use LD_LIBRARY_PATH
	 */
	if(vec[AT_LIBPATH] != 0) {
		p = env;
		while((libpath = *(p++))) {
			if(!strncmp(libpath, LIBPATHENV "=", sizeof(LIBPATHENV "=")-1)) {
				libpath += sizeof(LIBPATHENV "=")-1;
				break;
			}
		}
		p = env;

		/*
		 * The preload directive follows the same rules
		 */
		while((ld_preload = *(p++))) {
			if(!strncmp(ld_preload, LDPRELOAD "=", sizeof(LDPRELOAD "=")-1)) {
				ld_preload += sizeof(LDPRELOAD "=")-1;
				break;
			}
		}
	} else {
		_is_setuid = 1;
	}

	/*
	 * Get the system library path
	 */
	slp = _get_system_libpath_addr();
	if((len = confstr(_CS_LIBPATH, NULL, 0)) != 0) {
		*slp = _alloca(len+1);
		if((confstr(_CS_LIBPATH, *slp, len)) != len) {
			*slp = NULL;
		}
	}

	/*
	 * Preload any spcified libs (colon separated list)
	 */
	if(ld_preload) {
		char *colon, c = 0;

		needed = ld_preload;
		while(needed && *needed) {
			colon = strchr( (char *)needed, ':' );
			if ( colon != NULL ) {
				c = *colon;
				*colon = '\0';
			}
			if (!(obj = find_object(&objects, name_only(needed), NULL))) {
				obj = _alloca(sizeof *obj);
				xmemset(obj, 0, sizeof *obj);
				if (load_object(obj, needed,
					libpath, NULL, NULL, NULL)) {
					error("Could not preload library ", needed);
				} 
				obj->refcount++;
				obj->flags |= OBJFLAG_LD_PRELOAD; /* mark this object as LD_PRELOAD-ed */
				l = _alloca(sizeof *l);
				l->object = obj;
				l->root = &objects;
				list_insert(&objects, l);
			}
			needed = colon;
			if (colon) {
				*colon = c;
				needed++;
			}
		}
	}

	stuff_path_and_time(&self._link_map, xstrdup(_interp));
	stuff_path_and_time(&other._link_map, xstrdup(argv[0]));

	/*
	 * load any other needed libraries
	 */

	list_forward(&objects, o) {
		dyn = o->object->dynamic;
		while (dyn->d_tag) {
			if (dyn->d_tag == DT_NEEDED) {
				needed = name_only(o->object->strings + dyn->d_un.d_val);
				if (!(obj = find_object(&objects, needed, NULL))) {
					if (!strcmp(needed, self.name)) {
						obj = &self;
					} else {
						obj = _alloca(sizeof *obj);
						xmemset(obj, 0, sizeof *obj);
						if (load_object(obj, o->object->strings + 
							dyn->d_un.d_val, libpath, o->object->rpath, NULL, NULL)) {
							error("Could not load library ", o->object->strings + dyn->d_un.d_val);
						} 
					}
					obj->refcount++;
					l = _alloca(sizeof *l);
					l->object = obj;
					l->root = &objects;
					list_insert(&objects, l);
				}
			}
			dyn++;
		}
	}

	if (*_tlop) {
		xprintf( "\t%s => %s (0x%x)\n", self.name, _interp, self.text_addr);
		exit(EXIT_SUCCESS);
	}

	/*
	 * Now reverse all global relative relocs (i.e. 386_32, 386_PC32...) from the 
	 * bootstrapping of libc.so.
	 */

	self.flags |= OBJFLAG_REVERSED;
	(void) resolve(&bootstrap, RTLD_NOW);
	self.flags &= ~(OBJFLAG_REVERSED | OBJFLAG_RESOLVED);

	/*
	 * Now resolve all relocations in a normal manner.
	 * @@@ Allow switching to RTLD_LAZY when recovery from _dl_alloc is fixed
	 */

	if(resolve(&objects, RTLD_NOW) == -1) {
		// We're in trouble, not all relocations succeeded
		error("Could not resolve all symbols ", NULL);
	}		

	/*
	 * Incase we didn't load ourselfs, we still need to relocate
	 * with the new objects using ourselfs as a last resort.
	 */
	if((self.flags & OBJFLAG_RESOLVED) == 0) {
		bslist->root = &objects;
		l = _alloca(sizeof *l);
		*l = *bslist;
		list_append(&objects, l);
		if(resolve(&bootstrap, RTLD_NOW) == -1) {
			error("Could not resolve all symbols ", self.name);
		}
		list_delete(l);
	}

	/*
	 * It is now safe to access static data, and call any library
	 * functions.  Copy the object list into permanent space and create
	 * the start and all lists (initially the same list).  It is
	 * perfectly acceptable for _dl_alloc () to be overridden by the
	 * application with a function that just returns 0, this causes the
	 * loader to not allocate storage for the lists.  If this is done no
	 * other dl calls can be made and the .fini section processing will
	 * not be performed.
	 */

	dao = _get_dl_all_objects_addr();
	dso = _get_dl_start_objects_addr();
	dhl = _get_dl_handle_list_addr();
	list_empty(dao);
	list_empty(dso);
	list_empty(dhl);
	rd->r_map = NULL;

	list_forward(&objects, o) {
		if (!(obj = _dl_alloc(sizeof *obj))) {
			break;
		}
		xmemcpy(obj, o->object, sizeof *obj);
		if ( o->object == &self ) {
			/* this is the interpreter */
			rd->r_ldsomap = (Link_map *)obj;
		}

        if (obj->flags & OBJFLAG_TEXTREL) {
        	/* previously mapped in as writeable, now make it read only again */
			mprotect((void *)obj->text_addr, obj->text_size, PROT_READ|PROT_EXEC);
			obj->flags &= ~OBJFLAG_TEXTREL;
        }

		if (!(l = _dl_alloc(sizeof *l))) {
			break;
		}
		l->flags = 0;
		l->object = obj;

		link_map_append(rd->r_map, (Link_map *)l->object);
		if(!rd->r_map) rd->r_map = (Link_map *)l->object;

		l->root = (list_head_t *)dso;
		list_insert(dao, l);

		if (!(l = _dl_alloc(sizeof *l))) {
			break;
		}
		l->flags = 0;
		l->object = obj;
		l->root = (list_head_t *)dso;
		list_insert(dso, l);
	}
	/*
	 * Copy the system library path to permanent storage
	 */
	*slp = xstrdup(*slp);

	/*
	 * perform .init section processing on the newly created heap
	 * objects and not the ones which are on the stack.
	 */

	_dl_debug_state();
	preinit_array((list_head_t *)dso);
	_dl_debug_state();
	init((list_head_t *)dso);
	init_array((list_head_t *)dso);
	rd->r_state = RT_CONSISTENT;
	_dl_debug_state();

	/*
	 * start the object
	 */

	return vec[AT_ENTRY];
}


static void _dl_free_object(struct object *obj) {

	munmap((void *)obj->text_addr, obj->text_size);
	munmap((void *)(obj->text_addr + obj->data_offset), obj->data_size);
	if(obj->flags & OBJFLAG_NOSONAME && obj->name) {
		_dl_free ((void *)obj->name);
	}
	if (obj->_link_map.l_path) {
		_dl_free ((void *)obj->_link_map.l_path);
	}
	_dl_free(obj);
}
#define _DLCLOSE_FLAG_NOFINI 0x1
int _dlclose(void *handle, unsigned flags) {
	struct objlist		*o, *h = NULL;

	if(handle == NULL || handle == RTLD_DEFAULT) {
		ERROR_INV_HANDLE;
		return -1;
	} else if ((list_head_t *)handle == &_dl_start_objects) {
		return 0;
	}

	LOCK_LIST;
	list_forward( &_dl_handle_list, o ) {
		if ( o->object == handle ) {
			h = o;
			break;
		}
	}
	if ( h == NULL ) {
		ERROR_INV_HANDLE;
		UNLOCK_LIST;
		return -1;
	}
	while(!list_atend((list_head_t *)handle, o = list_first((list_head_t *)handle))) {
		struct object		*obj = o->object;

		/*
		 * Delete object from dll list.
		 */
		list_delete(o);
		_dl_free(o);

		if (--obj->refcount == 0) {
			/*
			 * Remove from all objects list
			 */
			__ldd_run_all_handlers(obj, LDD_EH_DLL_UNLOAD);
			list_forward(&_dl_all_objects, o) {
				if(o->object == obj) {
					link_map_delete(&_r_debug.r_map, o->object);
					list_delete(o);
					_dl_free(o);
					break;
				}
			}
			/*
			 * If the object was also on the start (global) list, remove it.
			 */
			list_forward(&_dl_start_objects, o) {
				if(o->object == obj) {
					list_delete(o);
					_dl_free(o);
					break;
				}
			}

			/* Now we can do fini.... */
			if(!(flags & _DLCLOSE_FLAG_NOFINI)) {
				fini(obj);
			}

			_dl_free_object(obj);
			_r_debug.r_rdevent = RD_DLACTIVITY;
			_dl_debug_state();
		}
	}

	_r_debug.r_state = RT_CONSISTENT;
	_dl_debug_state();
	list_delete(h);
	_dl_free(h);
	UNLOCK_LIST;
	_dl_free(handle);
	return 0;
}

int dlclose(void *handle) {
	return _dlclose(handle, 0);
}

char *dlerror(void) {
	char		*str = _dl_error;

	_dl_error = 0;
	return str;
}

void *dlopen(const char *name, int mode) {
	list_head_t			*handle;
	struct objlist		*olist, *ol;
	struct object		stkobj, *obj;
	char				*libpath = NULL;
	int					debug = 0, prepended = 0;

	_trace_loaded_objects = (int)getenv_ne("LD_TLO");
	if(getenv_ne("DL_DEBUG")) {
		debug = 1;
		xprintf("dlopen(\"%s\",%d)\n", name ? name : "NULL", mode);
	}
	if((mode & RTLD_NOSHARE) && (mode & (RTLD_WORLD | RTLD_GROUP)) != RTLD_GROUP) { 
		ERROR_BAD_MODE;
		return 0;
	}

/*
 * dlopen() will only work on dynamic executables, but be graceful about it!
 */
#ifndef VARIANT_so
#if defined(__WATCOMC__)
#pragma disable_message(201);
#endif
	ERROR_NO_DYN;
	return(NULL);
#endif

	/*
	 * if the _dl_alloc routine was unable to allocate memory at startup
	 * then no dl function will work.
	 */
	LOCK_LIST;
	if (!list_iszero(&_dl_start_objects) && list_isempty(&_dl_start_objects)) {
		ERROR_NO_MEMORY;
		UNLOCK_LIST;
		return 0;
	}
	if (list_iszero(&_dl_all_objects)) {
		list_empty(&_dl_all_objects);
	}
	UNLOCK_LIST;

	/*
	 * if passed a 0 for the pathname, returns a handle to the executable
	 * (and all DT_NEEDED) symbols
	 */
	if (!name) {
		if(list_iszero(&_dl_start_objects)) {
			_dl_error = "no symbols in executable";
			return 0;
		}
		return &_dl_start_objects;
	}

	/*
	 * create the handle with an initially empty list of objects
	 */
	if (!(handle = _dl_alloc(sizeof *handle))) {
		ERROR_NO_MEMORY;
		return 0;
	}
	list_empty(handle);

	if ( !_is_setuid ) {
		libpath = getenv_ne(LIBPATHENV);
	}

	/*
	 * If mode does not contain either the RTLD_GROUP or RTLD_WORLD, set
	 * those flags.
	 */
	if(!(mode & (RTLD_GROUP | RTLD_WORLD))) {
		mode |= (RTLD_GROUP | RTLD_WORLD);
	}

	LOCK_LIST;

	/* 
	 * Load the dll and tell the load_object() function that we want to
	 * verify that this dll is unique with respect to the other dll's
	 * which have been loaded and placed in the _dl_all_objects list.
	 */
	obj = NULL;
	xmemset(&stkobj, 0x00, sizeof stkobj);

	_r_debug.r_state = RT_ADD;
	_dl_debug_state();
	if (load_object(&stkobj, name, libpath, 0, 
	                (mode & RTLD_NOSHARE) ? NULL : &obj, &_dl_all_objects) == -1) {
		_r_debug.r_state = RT_CONSISTENT;
		_dl_debug_state();
		UNLOCK_LIST;
		if(debug) {
			xprintf("dlopen: %s\n", _dl_error);
		}
		return 0;
	}
	_r_debug.r_state = RT_CONSISTENT;
	_dl_debug_state();

	if (obj == NULL) {
		struct objlist     *l;

		if (!(obj = _dl_alloc(sizeof *obj))) {
			ERROR_NO_MEMORY;
			UNLOCK_LIST;
			return 0;
		}
		xmemcpy(obj, &stkobj, sizeof stkobj);

		/*
		 * place the object at the end of the all list
		 */
		if (!(l = _dl_alloc(sizeof *l))) {
			//@@@ memory leak of obj
			ERROR_NO_MEMORY;
			UNLOCK_LIST;
			return 0;
		}
		l->flags = 0;
		l->object = obj;
		l->root = &_dl_all_objects;
		link_map_append(_r_debug.r_map, (Link_map *)l->object);
		list_insert(&_dl_all_objects, l);
		_r_debug.r_rdevent = RD_DLACTIVITY;
		_dl_debug_state();
	}

	/*
	 * If mode indicates global binding for the object, respect that.
	 */
	if((mode & RTLD_GLOBAL) && !find_object(&_dl_start_objects, obj->name, NULL)) {
		struct objlist     *l;
		if (!(l = _dl_alloc(sizeof *l))) {
			ERROR_NO_MEMORY;
			UNLOCK_LIST;
			return 0;
		}
		l->flags = 0;
		l->object = obj;
		l->root = &_dl_start_objects;
		list_insert(&_dl_start_objects, l);
	}

	if (!(olist = _dl_alloc(sizeof *olist))) {
		// _dl_free_object(obj);
		ERROR_NO_MEMORY;
		UNLOCK_LIST;
		return 0;
	}
	obj->refcount++;
	olist->flags = 0;
	olist->object = obj;
	olist->root = handle;
	list_insert(handle, olist);

	/*
	 * Prepend the global start list. This is not used if mode has
	 * only RTLD_GROUP, except for objects brought in with LD_PRELOAD
	 */

	if( !list_iszero(&_dl_start_objects) && !list_isempty(&_dl_start_objects) ) {
		int		first = 1, ld_preload_only = !(mode & RTLD_WORLD);

		list_forward(&_dl_start_objects, ol) {
			struct objlist		*l;

				if( ld_preload_only && !(ol->object->flags & OBJFLAG_LD_PRELOAD) ) {
					/* we are looking only for objects with OBJFLAG_LD_PRELOAD */
					first = 0;
					continue;
					}

			if(ol->object != obj) {
				if (!(l = _dl_alloc(sizeof *l))) {
					ERROR_NO_MEMORY;
					UNLOCK_LIST;
					return 0;
				}
				ol->object->refcount++;
				l->flags = (ol->object->flags & OBJFLAG_LD_PRELOAD) ? 0:OBJLISTFLAG_REMOVE;
				l->object = ol->object;
				l->root = handle;
				if(first) {
					// The first global (the executable) goes to the front of the list
					list_append(handle, l);	
				} else {
					list_insert(handle, l);	
				}
				prepended = 1;
			}
			first = 0;
		}
	}

	/* add handle to list of valid handles */
	if (!(ol = _dl_alloc(sizeof *ol))) {
		ERROR_NO_MEMORY;
		UNLOCK_LIST;
		return 0;
	}
	ol->flags = 0;
	ol->object = (void *)handle;
	ol->root = &_dl_handle_list;
	list_insert(&_dl_handle_list, ol);

	/*
	 * For debugging, print out the adress of the text segment
	 */
	if(debug){
		xprintf("Library loaded; type \'add-sym %s %p\' in gdb to load symbols\n",
			obj->name, dlsym(handle,"_btext"));
	}

	/*
	 * now process the DT_NEEDED entries. Only do this if the RTLD_GROUP
	 * flag is set (i.e. if dlopen was called with RTLD_WORLD only, then
	 * do not process DT_NEEDED entries)
	 */
	if(mode & RTLD_GROUP) {
		list_forward(handle, olist) {
			const Elf32_Dyn			*dyn;

			/* 
			 We only need to process the DT_NEEDED entries for those new
			 things which we are being loaded, not all of the previous
			 entries which are inherited for relocation purposes.
			*/
			if(olist->flags & OBJLISTFLAG_REMOVE) {
				continue;
			}

			for (dyn = olist->object->dynamic; dyn->d_tag; dyn++) {
				if (dyn->d_tag == DT_NEEDED) {
					const char			*needed;
					struct objlist		*l;

					needed = name_only(olist->object->strings + dyn->d_un.d_val);
	
					if (!(obj = find_object(&_dl_all_objects, needed, NULL))) {
	
						/*
						 * the object has never been loaded so haul it in
						 */

						if (!(obj = _dl_alloc(sizeof *obj))) {
							ERROR_NO_MEMORY;
							UNLOCK_LIST;
							_dlclose(handle, _DLCLOSE_FLAG_NOFINI);
							return 0;
						}
						xmemset(obj, 0, sizeof *obj);

						if (load_object(obj, olist->object->strings + 
							dyn->d_un.d_val, libpath, olist->object->rpath, NULL, NULL)) {
							// load_object sets _dl_error
							_dl_free(obj);
							UNLOCK_LIST;
							if(debug) {
								xprintf("dlopen: %s\n", _dl_error);
							}
							_dlclose(handle, _DLCLOSE_FLAG_NOFINI);
							return 0;
						} 

						/*
						 * place the object at the end of the all list
						 */

						if (!(l = _dl_alloc(sizeof *l))) {
							ERROR_NO_MEMORY;
							UNLOCK_LIST;
							_dlclose(handle, _DLCLOSE_FLAG_NOFINI);
							return 0;
						}
						l->flags = 0;
						l->object = obj;
						l->root = handle;
						link_map_append(_r_debug.r_map, (Link_map *)l->object);
						list_insert(&_dl_all_objects, l);
						_r_debug.r_rdevent = RD_DLACTIVITY;
						_dl_debug_state();
					} else if (find_object(handle, needed, &l)) {
						/*
						 * the object is already in the list for this handle, ignore
						 */
						l->flags &= ~OBJLISTFLAG_REMOVE;
						continue;
					}
					/*
					 * place the object at the end of the handle list
					 */

					if (!(l = _dl_alloc(sizeof *l))) {
						ERROR_NO_MEMORY;
						UNLOCK_LIST;
						_dlclose(handle, _DLCLOSE_FLAG_NOFINI);
						return 0;
					}
					l->flags = 0;
					l->object = obj;
					l->root = handle;
					list_insert(handle, l);
					obj->refcount++;

					/*
					 * place the object at the end of the dso list, if RTLD_GLOBAL was set
					 */

					if((mode & RTLD_GLOBAL) && !find_object(&_dl_start_objects, obj->name, NULL)) {
						if (!(l = _dl_alloc(sizeof *l))) {
							ERROR_NO_MEMORY;
							UNLOCK_LIST;
							_dlclose(handle, _DLCLOSE_FLAG_NOFINI);
							return 0;
						}
						l->flags = 0;
						l->object = obj;
						l->root = &_dl_start_objects;
						list_insert(&_dl_start_objects, l);
						obj->refcount++;
					}
				}
			}
		}
	}

	if ( _trace_loaded_objects ) {
		UNLOCK_LIST;
		_dlclose(handle, _DLCLOSE_FLAG_NOFINI);
		return 0;
	}

	/*
	 * resolve the objects
	 */

	if(resolve(handle, RTLD_NOW) == -1) {
		ERROR_UNRES_SYM;
		UNLOCK_LIST;
		_dlclose(handle, _DLCLOSE_FLAG_NOFINI);
		return 0;
	}

	list_forward(handle, olist) {
        if (olist->object->flags & OBJFLAG_TEXTREL) {
        	/* previously mapped in as writeable, now make it read only again */
			mprotect((void *)olist->object->text_addr, olist->object->text_size, PROT_READ|PROT_EXEC);
			olist->object->flags &= ~OBJFLAG_TEXTREL;
        }
    }
	/*
	 After the resolution is complete, we want to return the handle list
	 to only containing the dependancies so that future dlsym()'s will
	 work as expected only showing items from the dependant list.

	 Only bother with this if we pre-pended the start list above
	*/
	if( prepended ) {
		olist = list_first(handle);
		while(!list_atend(handle, olist)) {
			ol = olist;
			olist = list_next(ol);
			if(ol->flags & OBJLISTFLAG_REMOVE) {
				ol->object->refcount--; //Never 0 in this scenario
				list_delete(ol);
				_dl_free(ol);
			}
		}
	}


	/*
	 * perform .init processing
	 */
	init(handle);

	/*
	 * perform .init_array processing
	 */
        init_array(handle);

	UNLOCK_LIST;

	return handle;
}

static struct object*
find_calling_dso(void *calling_pc) {
	struct objlist *o;
	struct list_head_t* das;
	uintptr_t cpc = (uintptr_t)calling_pc;

	das = (struct list_head_t*)_get_dl_all_objects_addr();
	if(!list_iszero(das) && !list_isempty(das)) {
		list_forward(das, o) {
			if((cpc >= o->object->text_addr) &&
				(cpc < (o->object->text_addr+o->object->text_size))) {
				return o->object;
			}
		}
	}
	return NULL;
}

void *dlsym(void *handle, const char *name) {
	const Elf32_Sym	*sym;
	struct object *obj,*robj;
	struct objlist *o, *h = NULL;
	int debug = 0;

	if(getenv_ne("DL_DEBUG")) {
		debug = 1;
		xprintf("dlsym(%p,%s)=", handle, name);
	}
	if(handle == RTLD_DEFAULT) { // Do search off the global list
		handle = &_dl_start_objects;
		h = (struct objlist *)handle;
	} else if(handle == RTLD_NEXT) { // Do search from calling object and forward
		void* caller = get_return_address();
		if((obj = find_calling_dso(caller)) ) {
			if((sym = lookup_linkmap_scope(name, obj, obj, &robj))) {
				UNLOCK_LIST;
				if(debug) {
					xprintf("%p\n", (void *) RELOCP(robj, sym->st_value));
				}
				return RELOCP(robj, sym->st_value);
			}
		}

		/* Guess we didn't find anything */
		UNLOCK_LIST;
		ERROR_NO_SYM;
		return NULL;
	} else if ((list_head_t *)handle == &_dl_start_objects) {
		h = (struct objlist *)handle;
	}
	LOCK_LIST;
	/* check for valid handle */
	list_forward( &_dl_handle_list, o ) {
		if ( o->object == handle ) {
			h = o;
			break;
		}
	}
	if ( h == NULL ) {
		ERROR_INV_HANDLE;
		UNLOCK_LIST;
		return NULL;
	}

	if ((sym = lookup_global(name, handle, NULL, 0, &obj ))) {
		UNLOCK_LIST;
		if(debug) {
			xprintf("%p\n", (void *) RELOCP(obj, sym->st_value));
		}
		return RELOCP(obj, sym->st_value);
	}
	UNLOCK_LIST;
	if(debug) {
		xprintf("NULL\n");
	}
	ERROR_NO_SYM;
	return 0;
}

/* this is an internal node that represents a single event
 * handler */
typedef struct ldd_eh_node {
	Ldd_Eh_t eh;
	void *eh_d_handle;
	unsigned flags;
	struct ldd_eh_node *eh_next;
	struct ldd_eh_node *eh_prev;
} Ldd_Eh_Node_t;

static Ldd_Eh_Node_t *ldd_eh_head=NULL;
static Ldd_Eh_Node_t *ldd_eh_tail=NULL;

/* add an event handler to the list */
static Ldd_Eh_Node_t *ldd_add_event_handler(Ldd_Eh_t eh, void *eh_d_handle, 
                      unsigned flags) 
{
	Ldd_Eh_Node_t *node;
	if (eh == NULL) {
		errno = EINVAL;
		return(NULL);
	}
	node = _dl_alloc(sizeof(Ldd_Eh_Node_t));
	if (node == NULL) 
		return(NULL);
	node->eh = eh;
	node->eh_d_handle = eh_d_handle;
	node->flags = flags;
	node->eh_next = node->eh_prev = NULL;
	/* if first */
	if (ldd_eh_head == NULL) {
		ldd_eh_head = ldd_eh_tail = node;
		return(node);
	}
	/* else add at end */
	ldd_eh_tail->eh_next = node;
	node->eh_prev = ldd_eh_tail;
	ldd_eh_tail = node;
	return(node);
}

/* remove an event handler previously added */
static int ldd_remove_event_handler(Ldd_Eh_Node_t *handle)
{
	Ldd_Eh_Node_t *temp;
	if (handle == NULL) {
		errno = EINVAL;
		return(-1);
	}
	/* walk list to find handler */
	temp = ldd_eh_head;
	while (temp) {
		if (temp == handle)
			break;
		temp = temp->eh_next;
	}	
	if (temp == NULL) {
		/* the handle passed in is not valid */
		errno = EINVAL;
		return(-1);
	}
	/* removing head */
	if (handle == ldd_eh_head) {
		ldd_eh_head = handle->eh_next;
		if (ldd_eh_head == NULL) {
			/* this is also the last */
			ldd_eh_tail = NULL;
		}
	}
	else {
		/* removing tail */
		if (handle == ldd_eh_tail) {
			ldd_eh_tail = handle->eh_prev;
		}
		else {
			/* somewhere in the middle */
			handle->eh_prev->eh_next = handle->eh_next;
			handle->eh_next->eh_prev = handle->eh_prev;
		}
	}
	/* free memory */
	_dl_free(handle);
	return(0);
}

/* run one handler for a given map object*/
static void ldd_run_one_handler(struct object *obj, Ldd_Eh_t eh, 
           void *eh_d_handle, unsigned flags)
{
	Ldd_Eh_Data_t ehd;
	ehd.l_map = &obj->_link_map;
	ehd.text_addr = obj->text_addr;
	ehd.text_size = obj->text_size;
	ehd.data_offset = obj->data_offset;
	ehd.data_size = obj->data_size;
	/* call one handler */
	(void) eh(&ehd, eh_d_handle, flags);
	return;
}

/* run all handlers for a given dll, and a given type
 * (load/unload) */
static void __ldd_run_all_handlers(struct object *obj, unsigned type)
{
	unsigned flags=0;
	Ldd_Eh_Node_t *temp;
	/* check if there is anything to do */
	if (ldd_eh_head == NULL)
		return;
	if (type == LDD_EH_DLL_LOAD) {
		flags |= LDD_EH_DLL_LOAD;
	}
	else {
		flags |= LDD_EH_DLL_UNLOAD;
	}
	temp = ldd_eh_head;
	while (temp) {
		/* if the requested type matches the stored type */
		if (temp->flags & type) {
			ldd_run_one_handler(obj, temp->eh, temp->eh_d_handle, flags);
		}
		temp = temp->eh_next;
	}
	return;
}

/* replay all dll loads */
static void ldd_replay_dll_load(Ldd_Eh_t eh, void *eh_d_handle)
{
	/* at this time we are locked */
	unsigned flags=0;
	struct objlist      *o;
	flags = (LDD_EH_DLL_LOAD | LDD_EH_DLL_REPLAY);

	if (!(list_iszero(&_dl_all_objects))) {
		list_forward(&_dl_all_objects, o) {
			ldd_run_one_handler(o->object, eh, eh_d_handle, flags);
		}
	}
	return;
}

/* public api: register event handler */
void *__ldd_register_eh(Ldd_Eh_t eh, void *eh_d_handle, unsigned flags)
{
	Ldd_Eh_Node_t *node;
	if (eh == NULL) {
		errno = EINVAL;
		return(NULL);
	}
	/* if the request is to replay then the type must be load */
	if ((flags & LDD_EH_DLL_REPLAY) && (!(flags & LDD_EH_DLL_LOAD))) {
		errno = EINVAL;
		return(NULL);
	}
	LOCK_LIST;
	node = ldd_add_event_handler(eh, eh_d_handle, flags);
	if (node != NULL) {
		if (flags & LDD_EH_DLL_REPLAY) {
			ldd_replay_dll_load(eh, eh_d_handle);
		}
	}
	UNLOCK_LIST;
	return((void *)node);
}

/* public api: de-register event handler */
int __ldd_deregister_eh(void *ehh)
{
	int status;
	LOCK_LIST;
	status = ldd_remove_event_handler((Ldd_Eh_Node_t *)ehh);
	UNLOCK_LIST;
	return(status);
}

#ifndef __WATCOMC__
/*
 * Now include the CPU-specific file. It is *extremly* important that this
 * be done with an angle bracket include rather than double quotes. If
 * we used the quote version, GCC would include the template relocs.ci
 * in cvs/nto/libc/ldd rather than the processor specific one.
 */
#include <relocs.ci>

#endif

#ifdef TEST

/*
 * Test harness, loads two objects
 */

#define SELF		"test/libself.so"
#define SELF_BASE	0x200000

#define OTHER		"test/other"
#define OTHER_BASE	0x100000

int main(int argc, char *argv[]) {
	int                 selffd, otherfd;
	struct object       obj;
	auxv_t              auxv[] = {
		{AT_DATA, 0},
		{AT_BASE, SELF_BASE},
		{AT_ENTRY, OTHER_BASE + 0x230},
		{AT_PHDR, OTHER_BASE + 0x34},
		{AT_PHNUM, 5},
		{AT_PHENT, sizeof(Elf32_Phdr)},
		{AT_NULL, 0}
	};

	selffd = open(SELF, O_RDONLY);
	load_elf32(&obj, selffd);
	auxv[0].a_un.a_val = obj.dataoff + obj.data_rel;

	otherfd = open(OTHER, O_RDONLY);
	load_elf32(&obj, otherfd);

	ldd(auxv);

	close(otherfd);
	close(selffd);
	return 0;
}

#endif
#undef LDD_C_USEAGE_ATTRIBUTE


__SRCVERSION("ldd.c $Rev: 201235 $");
