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
#include <hw/sysinfo.h>

#define NEXT_TAG(t)		((hwi_tag *)((uint32_t *)(t) + (t)->prefix.size))

unsigned
hwi_find_tag(unsigned start, int curr_item, const char *name) {
	hwi_tag		*tag;
	hwi_tag		*base;
	char		*tagname;

	tag = base = __hwi_base();
	if(start != HWI_NULL_OFF) {
		tag = (hwi_tag *)((uint8_t *)tag + start);
		tag = NEXT_TAG(tag);
	}
	for( ;; ) {
		if(tag->prefix.size == 0) return(HWI_NULL_OFF);
		tagname = __hwi_find_string(tag->prefix.name);
		if(strcmp(tagname, name) == 0) {
			return((uintptr_t)tag - (uintptr_t)base);
		}
		if(curr_item && (*tagname >= 'A' && *tagname <= 'Z')) {
			return(HWI_NULL_OFF);
		}
		tag = NEXT_TAG(tag);
	}
}

__SRCVERSION("hwi_find_tag.c $Rev: 153052 $");
