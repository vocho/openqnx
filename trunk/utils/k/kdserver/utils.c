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




#include "kdserver.h"
#define SYSPAGE_TARGET_ALL
#include _NTO_HDR_(sys/syspage.h)
#include <stddef.h>

static struct kdebug_private	kdbg_private;
static struct kdump_private		kdump_private;

#define SWAP16(__x)		((((__x) >> 8) & 0xff) | \
							(((__x) & 0xff) << 8))
#define SWAP32(__x)		((((__x) >> 24) & 0xff) | \
								(((__x) >> 8) & 0xff00) | \
								(((__x) & 0xff00) << 8) | \
								(((__x) & 0xff) << 24))
#define SWAP64(__x)		((((__x) >> 56) & 0xff) | \
								 (((__x) >> 40) & 0xff00) | \
								 (((__x) >> 24) & 0xff0000) | \
								 (((__x) >>  8) & 0xff000000) | \
								 (((__x) & 0xff000000) <<  8) | \
								 (((__x) & 0xff0000) << 24) | \
								 (((__x) & 0xff00) << 40) | \
								 (((__x) & 0xff) << 56))


uint16_t
endian_native16(uint16_t data) {
	return cross_endian ? SWAP16(data) : data;
}


uint32_t
endian_native32(uint32_t data) {
	return cross_endian ? SWAP32(data) : data;
}


uint64_t
endian_native64(uint64_t data) {
	return cross_endian ? SWAP64(data) : data;
}

target_ptr
endian_native_ptr(target_ptr data) {
	if(!cross_endian) return data;
//	if(cpu->ptr_size == sizeof(uint64_t)) return SWAP64(data);
	return SWAP32(data);
}


target_ptr
get_ptr_paddr(paddr64_t paddr) {
	union {
		uint32_t	_32;
		uint64_t	_64;
	}			data;

	if(core_read_paddr(paddr, &data, cpu->ptr_size) != cpu->ptr_size) {
		return TARGET_NULL;
	}
	switch(cpu->ptr_size) {
	case 4:	
		return endian_native32(data._32);
	case 8:	
		return endian_native64(data._64);
	}
	return TARGET_NULL;
}


target_ptr
get_ptr_vaddr(target_ptr vaddr) {
	union {
		uint32_t	_32;
		uint64_t	_64;
	}			data;

	if(core_read_vaddr(vaddr, &data, cpu->ptr_size) != cpu->ptr_size) {
		return TARGET_NULL;
	}
	switch(cpu->ptr_size) {
	case 4:	
		return endian_native32(data._32);
	case 8:	
		return endian_native64(data._64);
	}
	return TARGET_NULL;
}


static int
get_kdinfo(void) {
	paddr_t				paddr;
	syspage_entry_info	section;
	target_ptr			vaddr;
	struct kdebug_info	kdinfo;

	if(kdbg_private.vector_ver != 0) {
		return 1;
	}
	paddr = syspage_paddr + offsetof(struct syspage_entry, system_private);
	if(core_read_paddr(paddr, &section, sizeof(section)) != sizeof(section)) {
		return 0;
	}
	paddr = syspage_paddr + endian_native16(section.entry_off)
				+ offsetof(struct system_private_entry, kdebug_info);
	vaddr = get_ptr_paddr(paddr);
	if(vaddr == TARGET_NULL) {
		return 0;
	}
	if(core_read_vaddr(vaddr, &kdinfo, sizeof(kdinfo)) != sizeof(kdinfo)) {
		return 0;
	}
	vaddr = endian_native_ptr((target_ptr)kdinfo.kdbg_private);
	if(core_read_vaddr(vaddr, &kdbg_private, sizeof(kdbg_private)) != sizeof(kdbg_private)) {
		return 0;
	}
	vaddr = endian_native_ptr((target_ptr)kdinfo.kdump_private);
	if(core_read_vaddr(vaddr, &kdump_private, sizeof(kdump_private)) != sizeof(kdump_private)) {
		return 0;
	}
	kdbg_private.process_vector = (void *)endian_native_ptr((target_ptr)kdbg_private.process_vector);
	kdbg_private.th_reg_off = endian_native16(kdbg_private.th_reg_off);
	kdbg_private.pr_pid_off = endian_native16(kdbg_private.pr_pid_off);
	kdbg_private.pr_debug_name_off = endian_native16(kdbg_private.pr_debug_name_off);
	kdbg_private.pr_threads_off = endian_native16(kdbg_private.pr_threads_off);
	kdbg_private.pr_memory_off = endian_native16(kdbg_private.pr_memory_off);

	kdump_private.as_pgdir_off = endian_native16(kdump_private.as_pgdir_off);
	return 1;
}


paddr64_t
find_pid(unsigned pid) {
	struct kdebug_vector_entry	vector;
	unsigned					i;
	target_ptr					prp;
	paddr64_t					prp_paddr;
	uint32_t					chk_pid;
	target_ptr					base;
	unsigned					len;

	if(!get_kdinfo()) {
		return 0;
	}
	if(core_read_vaddr((target_ptr)kdbg_private.process_vector,
			&vector, sizeof(vector)) != sizeof(vector)) {
		return 0;
	}
	vector.nentries = endian_native16(vector.nentries);
	base = endian_native_ptr((target_ptr)vector.vector);
	for(i = 0; i < vector.nentries; ++i, base += cpu->ptr_size) {
		prp = get_ptr_vaddr(base);
		if((prp != TARGET_NULL) && ((prp & 1) == 0)) {
			if(cpu->v2p(prp, &prp_paddr, &len)) {
				if(core_read_paddr(prp_paddr + kdbg_private.pr_pid_off, &chk_pid, sizeof(chk_pid)) == sizeof(chk_pid)) {
					if(endian_native32(chk_pid) == pid) {
						return prp_paddr;
					}
				}
			}
		}
	}
	return 0;
}


int
set_pgtbl(paddr64_t prp_paddr) {
	target_ptr	vaddr;

	vaddr = get_ptr_paddr(prp_paddr + kdbg_private.pr_memory_off);
	if(vaddr == TARGET_NULL) {
		return 0;
	}
	vaddr = get_ptr_vaddr(vaddr + kdump_private.as_pgdir_off);
	if(vaddr == TARGET_NULL) {
		return 0;
	}
	core_read_vaddr(vaddr, pgtbl, cpu->pgtbl_size);
	return 1;
}


paddr64_t
find_tid(paddr64_t prp_paddr, unsigned *tidp) {
	struct kdebug_vector_entry	vector;
	unsigned					i;
	target_ptr					base;
	target_ptr					thp;
	paddr64_t					thp_paddr;
	unsigned					len;

	if(!get_kdinfo()) {
		return 0;
	}
	if(core_read_paddr(prp_paddr + kdbg_private.pr_threads_off, 
			&vector, sizeof(vector)) != sizeof(vector)) {
		return 0;
	}
	vector.nentries = endian_native16(vector.nentries);
	base = endian_native_ptr((target_ptr)vector.vector);
	i = *tidp;
	if(i > 0) --i;
	base += i * cpu->ptr_size;
	while(i < vector.nentries) {
		thp = get_ptr_vaddr(base);
		if((thp != TARGET_NULL) && ((thp & 1) == 0)) {
			if(cpu->v2p(thp, &thp_paddr, &len)) {
				*tidp = i + 1;
				return thp_paddr;
			}
		}
		++i;
		base += cpu->ptr_size;
	}
	return 0;
}


int
set_regset(paddr64_t thp_paddr) {
	core_read_paddr(thp_paddr + kdbg_private.th_reg_off, regset, note->regset_size);
	return 1;
}
