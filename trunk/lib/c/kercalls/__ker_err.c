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




#include <errno.h>
#include <sys/neutrino.h>

#ifdef __KER_ERR_ATTR_REGPARM
#error __KER_ERR_ATTR_REGPARM already defined!
#endif

#if defined(__X86__) && defined(__WATCOMC__)
    #pragma aux __ker_err parm [eax];
	#define __KER_ERR_ATTR_REGPARM
#elif defined(__X86__) && (defined(__GNUC__) || defined(__INTEL_COMPILER))
    #define __KER_ERR_ATTR_REGPARM __attribute__((regparm(1)))
#elif defined(__PPC__) \
   || defined(__MIPS__) \
   || defined(__ARM__) \
   || defined(__SH__)
	#define __KER_ERR_ATTR_REGPARM
#else
    #error __ker_err not configured
#endif

int __KER_ERR_ATTR_REGPARM 
__ker_err(int err) {
	errno = err;
	return -1;
}

__SRCVERSION("__ker_err.c $Rev: 153052 $");
