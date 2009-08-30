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
#undef memset

void *memset(void *s, int c, size_t n) {
	unsigned char		*p = s;

	/* Stuff unaligned addresses first */
	while(((uintptr_t)p & (sizeof(unsigned) - 1)) && n) {
		*p++ = c;
		n--;
	}

	/* Now stuff in native int size chunks if we can */
	if(n >= sizeof(unsigned)) {
#if __INT_BITS__ == 32
		unsigned		cc = 0x01010101 * (unsigned char)c;
#elif __INT_BITS__ == 64
		unsigned		cc = 0x0101010101010101 * (unsigned char)c;
#else
#error Unknown __INT_BITS__ size
#endif
		unsigned		*pp = (unsigned *)p - 1;

		while(n >= sizeof(unsigned)) {
			n -= sizeof(unsigned);
			*++pp = cc;
		}
		if(n) {
			p = (char *)(pp + 1);
		}
	}

	/* Get the remaining bytes */
	if(n) {
		p--;
		while(n) {
			n--;
			*++p = c;
		}
	}

	return s;
}

__SRCVERSION("memset.c $Rev: 153052 $");
