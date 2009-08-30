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




#include <inttypes.h>
#include <string.h>
#include <stdarg.h>
#include <alloca.h>
#include <hw/sysinfo.h>

#define NEXT_ITEM(t)	((hwi_tag *)((uint32_t *)(t) + (t)->item.itemsize))

struct name_list {
	struct name_list	*prev;
	const char			*name;
};

static int
match_item(hwi_tag *tag, struct name_list *name) {
	for( ;; ) {
		if(strcmp(name->name, __hwi_find_string(tag->item.itemname)) != 0) return(0);
		name = name->prev;
		if(name == NULL) return(1);
		if(tag->item.owner == HWI_NULL_OFF) return(0);
		tag = (hwi_tag *)((uint8_t *)__hwi_base() + tag->item.owner);
	}
}

unsigned
hwi_find_item(unsigned start, ...) {
	hwi_tag				*tag;
	hwi_tag				*base;
	va_list				args;
	struct name_list	*list;
	struct name_list	*curr;
	const char			*name;

	list = NULL;
	va_start(args, start);
	for(;;) {
		name = va_arg(args, char *);
		if(name == NULL) break;
		curr = alloca(sizeof(*curr)); /* Never returns NULL in startup code */
		curr->name = name;
		curr->prev = list;
		list = curr;
	}
	va_end(args);
	tag = base = __hwi_base();
	if(start != HWI_NULL_OFF) {
		tag = (hwi_tag *)((uint8_t *)tag + start);
		tag = NEXT_ITEM(tag);
	}
	for( ;; ) {
		if(tag->prefix.size == 0) return(HWI_NULL_OFF);
		if(match_item(tag, list)) {
			return((uintptr_t)tag - (uintptr_t)base);
		}
		tag = NEXT_ITEM(tag);
	}
}

__SRCVERSION("hwi_find_item.c $Rev: 200568 $");
