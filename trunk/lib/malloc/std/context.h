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





#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <ucontext.h>

#if defined(__X86__)
#include "x86/inline.h"
#elif defined(__MIPS__)
#include "mips/inline.h"
#elif defined(__PPC__)
#include "ppc/inline.h"
#endif

#ifdef __X86__
#define GET_REGFP(regs)	((regs)->ebp)
#define GET_FRAME_PREVIOUS(fp)	(*((void **)fp))
#define GET_FRAME_RETURN_ADDRESS(fp)	(*((ulong_t *)fp + 1))

#define _esp() ({register _uint32 __reg; __asm__ ( "movl %%esp,%0": "=qm" (__reg):); __reg; })
#define _ebp() ({register _uint32 __reg; __asm__ ( "movl %%ebp,%0": "=qm" (__reg):); __reg; })
#endif

#endif
