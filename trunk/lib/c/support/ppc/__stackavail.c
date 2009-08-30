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
#include <alloca.h>
#include <sys/neutrino.h>
#include "cpucfg.h"

#define _sp()	({unsigned __sp; __asm__ ( "mr %0,%%r1": "=r" (__sp)); __sp; })

size_t
__stackavail(void) {
	return _sp() - (uintptr_t)LIBC_TLS()->__stackaddr;
}

__SRCVERSION("__stackavail.c $Rev: 153052 $");
