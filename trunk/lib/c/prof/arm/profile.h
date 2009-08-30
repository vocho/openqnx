/*
 * $QNXtpLicenseC:
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




/*
 * Copyright (c) 1992, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)profile.h   8.1 (Berkeley) 6/11/93
 * $Id: profile.h 166179 2008-04-11 14:54:30Z cburgess $
 */

/*
 * Removed kernel profiling code for Neutrino 1.1, 31/08/98
 */

#ifndef _MACHINE_PROFILE_H_
#define _MACHINE_PROFILE_H_


#define FUNCTION_ALIGNMENT      4

#define _MCOUNT_DECL static void _mcount_

/*
 * The compiler inserts the following code on function entry:
 *	mov		ip, sp
 *	stmdb	sp!, {fp,ip,lr,pc}
 *	sub		fp, ip, #4
 *	bl		mcount
 *
 * On entry to mcount:
 *	lr is the address of the function
 *	fp is the address of the function's stack frame
 *     fp[-4]  is the caller's lr (address of the caller)
 *     fp[-8]  is the function's sp
 *     fp[-12] is the caller's fp
 *
 * We perform the following:
 * 1) save the function's arguments (r0-r3) and the lr
 * 2) set r0 to the function's caller's address
 * 3) set r1 to the function's address
 * 4) call _mcount_ to do the work
 * 5) restore r0-r3 and return to the function
 */

#define MCOUNT 						\
__asm__(".globl\tmcount\n"          \
	".hidden\tmcount\n"             \
	"mcount:\n"                     \
	"stmdb	sp!, {r0-r3,lr};\n"		\
	"ldr	r0, [fp, #-4];\n"		\
	"mov	r1, lr;\n"				\
	"bl		_mcount_;\n"			\
	"ldmia	sp!, {r0-r3,pc};\n"		\
	".type\tmcount,function\n"		\
	".size\tmcount,.-mcount\n"	    \
	);
typedef unsigned int    uintfptr_t;


/*
 * An unsigned integral type that can hold non-negative difference between
 * function pointers.
 */
typedef u_int   fptrdiff_t;

#include <sys/cdefs.h>

__BEGIN_DECLS
#ifdef __GNUC__
void    mcount __P((void)) __asm("mcount");
#endif
#ifdef DEF_MCOUNT
static void MCOUNT_ATTR _mcount_  __P((uintfptr_t frompc, uintfptr_t selfpc));
#endif
__END_DECLS

#endif /* !_MACHINE_PROFILE_H_ */


/* __SRCVERSION("profile.h $Rev: 166179 $"); */
