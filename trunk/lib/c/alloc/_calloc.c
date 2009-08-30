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

#ifdef MALLOC_PC
void *calloc_pc (size_t n, size_t size, unsigned int *caller_pc);
#else
void *__calloc(size_t n, size_t size);
#endif

void *calloc(size_t n, size_t size)
{
#ifdef MALLOC_PC
	return(calloc_pc(n, size, __builtin_return_address(0)));
#else
	return(__calloc(n, size));
#endif
}

__SRCVERSION("_calloc.c $Rev: 212948 $");
