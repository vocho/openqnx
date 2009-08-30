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




#include <stdlib.h>

#if RAND_MAX != 32767
#error RAND_MAX not conforming to common ANSI definition
#endif

int rand_r(unsigned *seed) {
	*seed = *seed * 1103515245 + 12345;
	return (unsigned)(*seed / 65536) % 32768;
}

static unsigned				next = 1;

int rand(void) {
	return rand_r(&next);
}

void srand(unsigned seed) {
	next = seed;
}

__SRCVERSION("rand.c $Rev: 153052 $");
