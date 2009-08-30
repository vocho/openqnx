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
#undef memcpy

void *memcpy(void *dst, const void *src, size_t nbytes) {
	void			*ret = dst;

	/* Both addresses must be aligned to stuff in int size chunks */
	if(		nbytes >= sizeof(unsigned) &&
			((uintptr_t)src & (sizeof(unsigned) - 1)) == 0 &&
			((uintptr_t)dst & (sizeof(unsigned) - 1)) == 0) {
		unsigned			*d = (unsigned *)dst - 1;
		const unsigned		*s = (const unsigned *)src - 1;

		while(nbytes >= sizeof(unsigned)) {
			nbytes -= sizeof(unsigned);
			*++d = *++s;
		}
		if(nbytes) {
			dst = (unsigned char *)(d + 1);
			src = (const unsigned char *)(s + 1);
		}
	}

	/* Get the unaligned bytes, or the remaining bytes */
	while(nbytes) {
		*(unsigned char *)dst = *(const unsigned char *)src;
		dst = (char *)dst + 1;
		src = (const char *)src + 1;
		--nbytes;
	}

	return ret;
}

__SRCVERSION("memcpy.c $Rev: 153052 $");
