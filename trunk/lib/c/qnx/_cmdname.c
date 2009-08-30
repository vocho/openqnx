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




#include <string.h>
#include <process.h>
#include <sys/auxv.h>

extern auxv_t		*_auxv;

char *_cmdname(char *name) {
	auxv_t			*auxp;

	for(auxp = _auxv; auxp->a_type != AT_NULL; auxp++) {
		if(auxp->a_type == AT_EXEFILE) {
			if(name) {
				return strcpy(name, auxp->a_un.a_ptr);
			}
			return auxp->a_un.a_ptr;
		}
	}
	return 0;
}

__SRCVERSION("_cmdname.c $Rev: 153052 $");
