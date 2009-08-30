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




/*- was made by gentab.awk, now hand-crafted */

#include <sys/types.h>
//Must use <> include for building libmalloc.so
#include <malloc-lib.h>
#include <limits.h>
static Band a1 = { _MALLOC_ALIGN*2, 32};
static Band a2 = { _MALLOC_ALIGN*3, 32};
static Band a3 = { _MALLOC_ALIGN*4, 32};
static Band a4 = { _MALLOC_ALIGN*6, 24};
static Band a5 = { _MALLOC_ALIGN*8, 24};
static Band a6 = { _MALLOC_ALIGN*10, 24};
static Band a7 = { _MALLOC_ALIGN*12, 16};
static Band a8 = { _MALLOC_ALIGN*16, 8};  
Band *__static_Bands[] = {
&a1,
&a2,
&a3,
&a4,
&a5,
&a6,
&a7,
&a8,
};
unsigned __static_nband=8;

__SRCVERSION("malloc-config.c $Rev: 153052 $");
