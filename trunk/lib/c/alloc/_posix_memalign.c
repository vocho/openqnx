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



#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc-lib.h>

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#ifndef MALLOC_WRAPPER
#ifndef weak_alias
#define weak_alias(alias,sym) \
__asm__(".weak " #alias " ; " #alias " = " #sym);
#endif
weak_alias(posix_memalign,_posix_memalign)
#endif
#endif

int __posix_memalign(void **memptr, size_t alignment, size_t size);

int _posix_memalign(void **memptr, size_t alignment, size_t size)
{
	return(__posix_memalign(memptr, alignment, size));
}

__SRCVERSION("_posix_memalign.c $Rev: 153052 $");
