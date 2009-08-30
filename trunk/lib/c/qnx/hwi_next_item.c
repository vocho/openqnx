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
#include <hw/sysinfo.h>

unsigned
hwi_next_item(unsigned off) {
	hwi_tag	*tag = hwi_off2tag(off);

	off += tag->item.itemsize * sizeof(uint32_t);
	tag = hwi_off2tag(off);
	if(tag->prefix.size == 0) return(HWI_NULL_OFF);
	return(off);
}

__SRCVERSION("hwi_next_item.c $Rev: 153052 $");
