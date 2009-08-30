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
#include "mm_internal.h"


int	munmap_flags_default = UNMAP_INIT_REQUIRED;

/*
 * Default mm_flags options to set
 * FUTURE: Remove BACKWARDS_COMPAT with 6.4
 */
static int mm_flags_default = MM_FLAG_BACKWARDS_COMPAT|MM_FLAG_VPS;

void
vmm_configure(char *cfg) {
	int			enable;
	char		ch;
	unsigned	flag;

	mm_flags = mm_flags_default;
	for( ;; ) {
		ch = *cfg++;
		enable = (ch != '~');
		if(!enable) ch = *cfg++;
		if(ch == '\0') break;
		flag = 0;
		switch(ch) {
		case 'i':	
			munmap_flags_default = enable ? UNMAP_INIT_REQUIRED : UNMAP_INIT_OPTIONAL;
			break;
		case 'x':
			procmgr_stack_executable(enable);
			break;
		case 'b':	
			flag = MM_FLAG_BACKWARDS_COMPAT;
			break;
		case 'l':
			flag = MM_FLAG_LOCKALL;
			break;
		case 'L':
			flag = MM_FLAG_SUPERLOCKALL;
			break;
		case 'P':
			flag = MM_FLAG_PADDR64_SAFE_SYS;
			break;
		case 'v':
			flag = MM_FLAG_VPS;
			break;
		//RUSH3: What other config's do we need?
		default: break;
		}	
		if(flag) {
			if(enable) {
				mm_flags |= flag;
			} else {
				mm_flags &= ~flag;
			}
		}
	}
	mm_flags_default = mm_flags;
}

__SRCVERSION("vmm_configure.c $Rev: 199574 $");
