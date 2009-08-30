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

#include "pmm.h"

#undef MEMPMM_DEBUG
//#define MEMPMM_DEBUG

static unsigned
_dummy_alloc(unsigned size, void **addr) {
	// Can't get anything more from the system, since it's all been
	// put in the heap.
	return 0;
}

int
pmm_mmap(PROCESS *prp, uintptr_t addr, size_t len, int prot, int flags, 
		OBJECT *obp, uint64_t boff, unsigned align, unsigned preload, int fd, 
		void **vaddr, size_t *size, part_id_t mpart_id) {
	unsigned		off = boff;

#ifdef MEMPMM_DEBUG
kprintf("\npmm_mmap: prp=%x, addr=%x, len=%x, prot=%x, flags=%x\n", prp, addr, len, prot, flags);
kprintf("pmm_mmap: obp=%x, off=%x\n", obp, off);
#endif
	// Physical can't do guardpages, so remove them from the len
	if(flags & MAP_STACK) {
		if(off >= len) {
			return EINVAL;
		}
		len -= off;
	}
	if((prot & (PROT_READ | PROT_WRITE | PROT_EXEC)) == PROT_NONE) {
		return EACCES;
	}
	if(obp) {
		if(off >= obp->mem.mm.size) {
			len = 0;
		} else if(off + len >= obp->mem.mm.size) {
			len = obp->mem.mm.size - off;
		}
		if(len == 0) {
			*size = 0;
			*vaddr = (void *)addr;
			return EOK;
		}
		off += CPU_V2P(obp->mem.mm.pmem); //???
		if((flags & MAP_TYPE) == MAP_SHARED) {
			flags |= MAP_PHYS;
		}
	}
	if((flags & MAP_TYPE) == MAP_PRIVATE) {
		struct mem_phys_entry			*mem;
		unsigned						len2;

		if(flags & MAP_FIXED) {

#ifdef MEMPMM_DEBUG
kprintf("\naddr=%x,user_addr=%x,user_addr_end=%x\n",addr,user_addr,user_addr_end);
#endif
			
			if(addr >= user_addr && addr + len - 1 <= user_addr_end) {
				*vaddr = (void *)addr;
				*size = ((addr + len + 3) & ~3) - addr;
				if(!(flags&MAP_ELF)) {
					memset((void *)addr, 0x00, *size);
				}
				return EOK;
			}
		} else if(!(flags & (MAP_PHYS | MAP_STACK | MAP_ELF))) {
			len = ((len + 32 + sizeof *mem) & ~32) - sizeof *mem;
		}
		len2 = sizeof *mem + len;
		if(flags & MAP_NOX64K) {
			if(len & 7) {
				len = (len | 7) + 1;
				len2 = sizeof *mem + len;
			}
			if(len > (32 << 10)) {
				len2 += 64 << 10;
			} else {
				len2 += len;
			}
		}
		for( ;; ) {
			mem = _sreallocfunc(0, 0, len2, _dummy_alloc);
			if(mem != NULL) break;
			if(!purger_invoke(len2)) {
				return ENOMEM;
			}
		}
		if(flags & MAP_NOX64K) {
			if(len > (32 << 10)) {
				unsigned						len3;

				if((len3 = (64 << 10) - ((uintptr_t)(mem + 1) & ((64 << 10) - 1)))) {
					_sfree(mem, len3);
					mem = (struct mem_phys_entry *)((char *)mem + len3);
				}
				_sfree((char *)(mem + 1) + len, (64 << 10) - len3);
				len2 -= 64 << 10;
			} else {
				if(((uintptr_t)(mem + 1) & ((64 << 10) - 1)) <= (((uintptr_t)(mem + 1) + len) & ((64 << 10) - 1))) {
					_sfree((char *)(mem + 1) + len, len);
				} else {
					_sfree(mem, len);
					mem = (struct mem_phys_entry *)((char *)mem + len);
				}
				len2 -= len;
			}
		}
		if((flags & MAP_FIXED) && (uintptr_t)(mem + 1) != addr) {
			_sfree(mem, len2);
			return ENXIO;
		}
		if((flags & MAP_BELOW16M) && (((uintptr_t)(mem + 1) + len - 1) & ~((16 << 20) - 1))) {
			_sfree(mem, len2);
			return ENOMEM;
		}
		
		if (prot & PROT_NOCACHE) {
			*vaddr = (void *)CPU_P2V_NOCACHE(CPU_V2P(mem + 1));
		} else {
			*vaddr = (void *)(mem + 1);
		}

		mem->size = len;
		mem->next = (struct mem_phys_entry *)prp->memory;
		mem->flags = flags | MAP_SYSRAM;
		mem->reloc = 0;
		prp->memory = (ADDRESS *)mem;
		mem_free_size -= (sizeof *mem + mem->size + 3) & ~3;
		memset(*vaddr, 0x00, len);
		if(obp) {
			memcpy(vaddr, (void *)off, len);
		}
	} else if(flags & MAP_PHYS) {
		switch(flags & MAP_TYPE) {
		case MAP_SHARED:
			if((flags & MAP_FIXED) && addr != off) {
				if((flags & (MAP_ELF | MAP_SYSRAM)) != (MAP_ELF | MAP_SYSRAM) || prp->pid != SYSMGR_PID) {
					return ENXIO;
				}
			}
			*vaddr = (void *)off;
			break;
		case MAP_PRIVATE: {
			if((flags & MAP_FIXED) && addr >= user_addr && addr + len - 1 <= user_addr_end && addr != off) {
				memcpy((void *)addr, (void *)CPU_P2V(off), len);
				*vaddr = (void *)addr;
				*size = ((addr + len + 3) & ~3) - addr;
				memset((char *)addr + len, 0x00, *size - len);
				return EOK;
			}
			return EINVAL;
		}
		default:
			return EINVAL;
		}
	} else {
		return ENODEV;
	}
	*size = len;
	return EOK;
}

__SRCVERSION("pmm_mmap.c $Rev: 168445 $");
