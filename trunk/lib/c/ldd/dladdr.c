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

//  Declare it here so that Proc does not include ldd.c...
pthread_mutex_t _dl_list_mutex = PTHREAD_MUTEX_INITIALIZER;
list_head_t		_dl_all_objects;

/* Find the symbol name from the address */

int _dladdr(void *addr, Dl_info *info){
	struct object		*matchobj;
	struct objlist		*o;
	list_head_t		*list;
	unsigned long 		offset, newoffset;
	const Elf32_Sym		*matchsym;
	int			i;

	if (!addr || !info) {
		return 0;
	}

	matchobj = NULL;
	if(!(list = _dll_list())) {
		return 0;
	}
	list_forward(list, o) {
		if (!matchobj && 
			(uintptr_t)addr >= o->object->text_addr && 
			(uintptr_t)addr <= o->object->text_addr + 
				o->object->data_offset + o->object->data_size){
			matchobj = o->object;
		}
	}
	if(!matchobj) {
		return 0;
	}
	/*
	 * Now we have the object where the address resides. Next, we need to 
	 * walk the hash/symbol table and find the closest match.
	 */

	offset = ~0;
	matchsym = NULL;
	for (i = 0; i < matchobj->hash[1]; i++) {

		if (((uintptr_t)addr >= RELOC(matchobj, matchobj->symbols[i].st_value)) &&
		   ((newoffset = ((uintptr_t)addr - RELOC(matchobj, matchobj->symbols[i].st_value)))  
		     <= offset)) {
			// If we had an identical match to a section, replace it
			if(!(newoffset == offset && matchsym && ELF32_ST_TYPE(matchsym->st_info) != STT_SECTION)){
				offset = (uintptr_t) addr - RELOC(matchobj, matchobj->symbols[i].st_value);
				matchsym = &matchobj->symbols[i];
			}
		}
	}		
	info->dli_fname = (const char *) matchobj->name;
	info->dli_fbase = (void *) matchobj->text_addr;
	if (matchsym) {
		info->dli_sname = matchobj->strings + matchsym->st_name;
		info->dli_saddr = (void *) RELOC(matchobj, matchsym->st_value);
	} else {
		info->dli_sname = "_btext";
		info->dli_saddr = (void *) matchobj->text_addr;
	}

	return 1;
}

int dladdr(void *addr, Dl_info *info){
	int 		ret;

	LOCK_LIST;
	ret = _dladdr(addr, info);
	UNLOCK_LIST;

	return ret;
}

void *_dll_list(void) {
	struct _process_local_storage		*pls;
	extern void *__SysCpupageGet(int index);

	if((pls = __SysCpupageGet(CPUPAGE_PLS))) {
		if(!pls->__dll_list) {
			pls->__dll_list = &_dl_all_objects;
		}
		return ( list_iszero(pls->__dll_list) ? NULL : pls->__dll_list );
	}
	return ( list_iszero(&_dl_all_objects) ? NULL : &_dl_all_objects );
}

__SRCVERSION("dladdr.c $Rev: 153052 $");
