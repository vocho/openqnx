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

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
void *memcpy(void *dst, const void *src, size_t nbytes) {
	register char	*tmp;

	asm volatile (
		"cld\n\t"
		"rep; movsl\n\t"
		"movb %b4,%b0\n\t"
		"rep; movsb"
		:"=c" (tmp), "=D" (tmp), "=S" (tmp)
		:"0" (nbytes / 4), "q" (nbytes & 3), "1" (tmp = dst), "2" (src)
		:"memory");
	return dst;
}

#else

void *memcpy(void *dst, const void *src, size_t nbytes) {
#if defined(__WATCOMC__) && defined(__386__)
	if( ((unsigned)src % sizeof(unsigned)) == 0
	 && ((unsigned)dst % sizeof(unsigned)) == 0) {
		void *movs(void *dst, const void *src, unsigned nbytes);
		#pragma aux movs = ".386" "mov eax,ecx" "shr ecx,2" "repne movsd" "mov cl,al" "and cl,3" "repne movsb" \
			parm [edi] [esi] [ecx] modify exact [eax ecx edi esi] value [edi];
		movs(dst, src, nbytes);
	} else
#endif
	{
		char			*d = dst;
		const char		*s = src;

		while(nbytes--) {
			*d++ = *s++;
		}
	}
	return dst;
}

#endif

__SRCVERSION("memcpy.c $Rev: 153052 $");
